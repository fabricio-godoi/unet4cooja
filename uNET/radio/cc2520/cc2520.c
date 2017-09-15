/*
 * \author Fabricio Negrisolo de Godoi
 * \date 17-03-2017
 * \brief Driver for CC2520 radio from Texas Instruments
 *
 * \details
 *         Cooja PINOUTs
 *         MCU         CC2520
 *         P1.5  <---   FIFO
 *         P1.6  <---   FIFOP  (Interruption)
 *         P1.7  <---   CCA
 *         P2.0  <---   SFD
 *         P3.0  --->   CSn
 *         P4.3  --->   WAKE   (VREG)
 *         P4.4  --->   RESET
 */

#include "cc2520.h"

#include <stdlib.h>
#include "cc2520_arch.h"
#include "cc2520_config.h"
#include "cc2520_const.h"
#include "ieee802154.h"

// Stats
#include "unet_api.h"

static uint32_t (*_isr_handler)(void);
static radio_params_t cc2520_radio_params;
volatile unsigned char cc2520_waiting_ack;//todo change to static

static uint8_t receive_on;
static int channel;
volatile unsigned char pkt_len;


//#define CC2520_DEBUG // NOTE: this could cause Cooja simulator to crash, probably problem with interruptions
#ifdef CC2520_DEBUG
#define CC2520_PRINTF  				PRINTF
#define CC2520_PRINTF_PACKET(buf,len)	packet_print(buf,len)
#else
#define CC2520_PRINTF(...)
#define CC2520_PRINTF_PACKET(buf,len)
#endif


/* Configuration */
#define WITH_SEND_CCA 1
#define FOOTER_LEN 2
#define CC2520_ACK_LENGTH 6	// [ LEN FRC.L FRC.M SQN FCH.L FCH.M ] ieee802.15.4
#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f
#define CC2520_MAX_PACKET_LENGTH 128

#ifndef CC2520_CONF_AUTOACK
#define CC2520_CONF_AUTOACK              1
#endif /* CC2520_CONF_AUTOACK */

#define CC2520_AUTOCRC (1 << 6)
#define CC2520_AUTOACK (1 << 5)
#define FRAME_MAX_VERSION ((1 << 3) | (1 << 2))
#define FRAME_FILTER_ENABLE (1 << 0)
#define CORR_THR(n) (((n) & 0x1f) << 6)
#define FIFOP_THR(n) ((n) & 0x7f)

#define WITH_SEND_CCA 1
#define FOOTER_LEN 2
#define RTIMER_ARCH_SECOND 1000 //(4096U*8) // TODO got it from contiki, tune it
///
#define RTIMER_SECOND RTIMER_ARCH_SECOND
#define RTIMER_NOW() OSGetTickCount() //rtimer_arch_now()
#define RTIMER_CLOCK_LT(a,b)     ((signed short)((a)-(b)) < 0)
#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
	  unsigned long t0;                                                 \
    t0 = (unsigned long) RTIMER_NOW();                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)

void clock_delay_usec(uint16_t dt) {
	int i = 0;
	for (i = 0; i < dt * 6; i++) {
	};
}

static void strobe(uint8_t regname) {
	CC2520_STROBE(regname);
}

static uint8_t getreg(uint16_t regname) {
	uint8_t reg;
	CC2520_READ_REG(regname, reg);
	return reg;
}

static void setreg(uint16_t regname, uint8_t value) {
	CC2520_WRITE_REG(regname, value);
}

static unsigned int status(void) {
	uint8_t status;
	CC2520_GET_STATUS(status);

	return status;
}

static void flushrx(void) {
	uint8_t dummy;
	(void) dummy;
	CC2520_READ_FIFO_BYTE(dummy);
	CC2520_STROBE(CC2520_INS_SFLUSHRX);
	CC2520_STROBE(CC2520_INS_SFLUSHRX);
}

static void flushtx(void) {
	CC2520_STROBE(CC2520_INS_SFLUSHTX);
	CC2520_STROBE(CC2520_INS_SFLUSHTX);
	// They are duplicated to ensure that the buffer is clear
	// in Cooja simulator some times the buffer aren't clear
	// inducing to TX buffer to be wrapped
}
static void cc2520_on(void)
{
  CC2520_ENABLE_FIFOP_INT();
  strobe(CC2520_INS_SRXON);
  BUSYWAIT_UNTIL(status() & (BV(CC2520_XOSC16M_STABLE)), RTIMER_SECOND / 100);
}
static void cc2520_off(void)
{
  /* Wait for transmission to end before turning radio off. */
  BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);

  strobe(CC2520_INS_SRFOFF);
  CC2520_DISABLE_FIFOP_INT();

  if(!CC2520_FIFOP_IS_1) {
    flushrx();
  }
}

void clock_delay(unsigned int i) {
	while (i--) {
		_NOP();
	}
}

void cc2520_set_channel(uint8_t ch) {
	uint16_t f;

	/*
	 * Subtract the base channel (11), multiply by 5, which is the
	 * channel spacing. 357 is 2405-2048 and 0x4000 is LOCK_THR = 1.
	 */
	channel = ch;

	f = MIN_CHANNEL + ((channel - MIN_CHANNEL) * CHANNEL_SPACING);
	/*
	 * Writing RAM requires crystal oscillator to be stable.
	 */
	BUSYWAIT_UNTIL((status() & (BV(CC2520_XOSC16M_STABLE))),
			RTIMER_SECOND / 10); /// TODO status and BUSYWAIT

	/* Wait for any transmission to end. */
	BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);

	/* Define radio channel (between 11 and 25) */
	setreg(CC2520_FREQCTRL, f);

	/* If we are in receive mode, we issue an SRXON command to ensure
	 that the VCO is calibrated. */
	if (receive_on) {
		strobe(CC2520_INS_SRXON);
	}

	return;
}

void cc2520_set_pan_addr(unsigned pan, unsigned addr, const uint8_t *ieee_addr) {
	uint8_t tmp[2];

	/*
	 * Writing RAM requires crystal oscillator to be stable.
	 */
	BUSYWAIT_UNTIL(status() & (BV(CC2520_XOSC16M_STABLE)), RTIMER_SECOND / 10);

	tmp[0] = pan & 0xff;
	tmp[1] = pan >> 8;
	CC2520_WRITE_RAM(&tmp, CC2520RAM_PANID, 2);

	tmp[0] = addr & 0xff; /// L byte
	tmp[1] = addr >> 8;   /// H byte
	CC2520_WRITE_RAM(&tmp, CC2520RAM_SHORTADDR, 2);
	if (ieee_addr != NULL) {
		int f;
		uint8_t tmp_addr[8];
		// LSB first, MSB last for 802.15.4 addresses in CC2520
		for (f = 0; f < 8; f++) {
			tmp_addr[7 - f] = ieee_addr[f];
		}
		CC2520_WRITE_RAM(tmp_addr, CC2520RAM_IEEEADDR, 8);
	}
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set TX power
 *
 *        This routine sets the transmission power of the CC2520.
 */
void cc2520_set_tx_power(uint8_t pwr) {
//	set_long_add_mem(CC2520_RFCON3, pwr); // brtos
	setreg(CC2520_TXPOWER, pwr);
}
/*---------------------------------------------------------------------------*/
static void cc2520_rx_disable(void) {
//	strobe(CC2520_INS_SRFOFF); // this strobe turn off the radio
	// need to set the RXENABLE0 register
	/* Disable packet reception */
	setreg(CC2520_RXENABLE0, 0x00); //  Does not abort ongoing TX/RX,
									// goes to idle when TX/RX is done
}
/*---------------------------------------------------------------------------*/
static void cc2520_rx_enable(void) {
	/* Enable packet reception */
//	strobe(CC2520_INS_SRXON); // Enable RX, set RXENABLE[15] register
							  // This strobe abort ongoing transmissions
//	The SRXON strobe:
//	o Sets RXENABLE[15]
//	o Aborts ongoing transmission/reception by forcing a transition to RX calibration.
//	• The STXON strobe when FRMCTRL1.SET_RXENMASK_ON_TX is enabled:
//	o Sets RXENABLE[14]
//	o The receiver is enabled after transmission completes.
//	• Setting RXENABLE != 0x0000:
//	o Does not abort ongoing transmission/reception.
	setreg(CC2520_RXENABLE0, 0x01); //  Does not abort ongoing TX/RX
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set radio configurations
 * \param opt radio options
 * \param value for setting the specific option value
 * \return -1 on error, 0 without error
 */
int cc2520_set(radio_opt_t opt, uint8_t value) {
	cc2520_radio_params.param[opt] = value; // refresh local mem information
	switch (opt) {
	case RADIO_STATE:
		break;
	case CHANNEL:
		//value = ((value - 11)%16)+11;
		cc2520_radio_params.param[opt] = value;
		cc2520_set_channel(value);
		break;
	case MACADDR16H:
		CC2520_WRITE_RAM(&value, CC2520RAM_SHORTADDR + 1, 1);
		break;
	case MACADDR16L:
		CC2520_WRITE_RAM(&value, CC2520RAM_SHORTADDR, 1);
		break;
	case PANID16H:
		CC2520_WRITE_RAM(&value, CC2520RAM_PANID + 1, 1);
		break;
	case PANID16L:
		CC2520_WRITE_RAM(&value, CC2520RAM_PANID, 1);
		break;
	case MACADDR64_7:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 7, 1);
		break;
	case MACADDR64_6:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 6, 1);
		break;
	case MACADDR64_5:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 5, 1);
		break;
	case MACADDR64_4:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 4, 1);
		break;
	case MACADDR64_3:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 3, 1);
		break;
	case MACADDR64_2:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 2, 1);
		break;
	case MACADDR64_1:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR + 1, 1);
		break;
	case MACADDR64_0:
		CC2520_WRITE_RAM(&value, CC2520RAM_IEEEADDR, 1);
		break;
#if 0
		case MACADDR16:
			setreg(value);
		break;
		case MACADDR64:
			setreg(value);
		break;
		case PANID:
			setreg(value);
		cc2520_radio_params.param[opt] = value;
		break;
#endif
	case TXPOWER:
		if (value > 31)
			value = 31;
		cc2520_radio_params.param[opt] = value;
		cc2520_set_tx_power(CC2520_TX_PWR_SET(((value >> 3) & 0x3), (value & 0x7))); /// TODO this is right?
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

/**
 * \brief Get radio configurations
 * \param opt radio options from radio_opt_t enum
 * \param value of return of the radio configuration
 * \return -1 on error, 0 without error
 */
int cc2520_get(radio_opt_t opt, uint8_t *value) {
	*value = cc2520_radio_params.param[opt];

	switch(opt){
//		case MACADDR16:
//			CC2520_READ_RAM(value, CC2520RAM_SHORTADDR, 2);
//			break;
		case MACADDR16H:
			CC2520_READ_RAM(value, CC2520RAM_SHORTADDR + 1, 1);
			break;
		case MACADDR16L:
			CC2520_READ_RAM(value, CC2520RAM_SHORTADDR, 1);
			break;
//		case PANID:
//			CC2520_READ_RAM(value, CC2520RAM_PANID, 2);
//			break;
		case PANID16H:
			CC2520_READ_RAM(value, CC2520RAM_PANID + 1, 1);
			break;
		case PANID16L:
			CC2520_READ_RAM(value, CC2520RAM_PANID, 1);
			break;
//		case MACADDR64:
//			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR, 8);
//			break;
		case MACADDR64_7:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 7, 1);
			break;
		case MACADDR64_6:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 6, 1);
			break;
		case MACADDR64_5:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 5, 1);
			break;
		case MACADDR64_4:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 4, 1);
			break;
		case MACADDR64_3:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 3, 1);
			break;
		case MACADDR64_2:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 2, 1);
			break;
		case MACADDR64_1:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR + 1, 1);
			break;
		case MACADDR64_0:
			CC2520_READ_RAM(value, CC2520RAM_IEEEADDR, 1);
			break;
		default:
			return -1;
			break;
	}

	return 0;
}



/*---------------------------------------------------------------------------*/
///* TODO move to specific file
#define asmv(arg) __asm__ __volatile__(arg)
int splhigh_(void) {
	int sr;
	/* Clear the GIE (General Interrupt Enable) flag. */
#ifdef __IAR_SYSTEMS_ICC__
	sr = __get_SR_register();
	__bic_SR_register(GIE);
#else
	asmv("mov r2, %0" : "=r" (sr));
	asmv("bic %0, r2" : : "i" (GIE));
#endif
	__no_operation(); // must be set after updating GIE /// Fabricio
	return sr & GIE; /* Ignore other sr bits. */
}
#define splhigh() splhigh_()
/// Set Processor Level
#define splx(sr) __asm__ __volatile__("bis %0, r2" : : "r" (sr))
////*******
/**
 * \brief Init transceiver
 *
 *        This routine initializes the radio transceiver
 */
int cc2520_init(const void *isr_handler) {

//	const addr64_t mac_addr64 = { .u8 = { PANID_INIT_VALUE, 0x00, 0x00, 0x00,
//			0x00, MAC16_INIT_VALUE } };
	//uint16_t shortaddr;
	//uint16_t panid;

	_isr_handler = isr_handler;

	// This is performed in the drivers
//	CC2520_SPI_PORT_INIT();

//	int s = splhigh();

	/* all input by default, set these as output */
	CC2520_CSN_PORT(DIR) |= BV(CC2520_CSN_PIN);
	CC2520_VREG_PORT(DIR) |= BV(CC2520_VREG_PIN);
	CC2520_RESET_PORT(DIR) |= BV(CC2520_RESET_PIN);

	/* force input pins */
	CC2520_FIFOP_PORT(DIR) &= ~(BV(CC2520_FIFOP_PIN));
	CC2520_FIFO_PORT(DIR) &= ~(BV(CC2520_FIFO_PIN));
	CC2520_CCA_PORT(DIR) &= ~(BV(CC2520_CCA_PIN));
	CC2520_SFD_PORT(DIR) &= ~(BV(CC2520_SFD_PIN));

	CC2520_SPI_DISABLE(); /* Unselect radio. */

	// Configure interruption event (rising edge)
	CC2520_DISABLE_FIFOP_INT();
	CC2520_FIFOP_INT_INIT();
//	splx(s);

	/* Set the IO pins direction */
//	CC2520_PIN_INIT();  above (BRTOS portion)

	SET_VREG_INACTIVE();
	clock_delay(250);
	/* Turn on voltage regulator and reset. */
	SET_VREG_ACTIVE();
	clock_delay(250);
	SET_RESET_ACTIVE();
	clock_delay(127);
	SET_RESET_INACTIVE();
	clock_delay(125);
	/* Turn on the crystal oscillator. */
	strobe(CC2520_INS_SXOSCON);
	clock_delay(125);

	BUSYWAIT_UNTIL(status() & (BV(CC2520_XOSC16M_STABLE)), RTIMER_SECOND / 100);

	/* Change default values as recommended in the data sheet, */
	/* correlation threshold = 20, RX bandpass filter = 1.3uA.*/

	setreg(CC2520_TXCTRL, 0x94);
	setreg(CC2520_TXPOWER, 0x13);    // Output power 1 dBm

	/*

	 valeurs de TXPOWER
	 0x03 -> -18 dBm
	 0x2C -> -7 dBm
	 0x88 -> -4 dBm
	 0x81 -> -2 dBm
	 0x32 -> 0 dBm
	 0x13 -> 1 dBm
	 0x32 -> 0 dBm
	 0x13 -> 1 dBm
	 0xAB -> 2 dBm
	 0xF2 -> 3 dBm
	 0xF7 -> 5 dBm
	 */
	setreg(CC2520_CCACTRL0, 0xF8);  // CCA treshold -80dBm

	// Recommended RX settings
	setreg(CC2520_MDMCTRL0, 0x84);  // Controls modem
	setreg(CC2520_MDMCTRL1, 0x14);  // Controls modem
	setreg(CC2520_RXCTRL, 0x3F); // Adjust currents in RX related analog modules
	setreg(CC2520_FSCTRL, 0x5A);  // Adjust currents in synthesizer.
	setreg(CC2520_FSCAL1, 0x2B);  // Adjust currents in VCO
	setreg(CC2520_AGCCTRL1, 0x11);  // Adjust target value for AGC control loop
	setreg(CC2520_AGCCTRL2, 0xEB);

	//  Disable external clock
	setreg(CC2520_EXTCLOCK, 0x00);

	//  Tune ADC performance
	setreg(CC2520_ADCTEST0, 0x10);
	setreg(CC2520_ADCTEST1, 0x0E);
	setreg(CC2520_ADCTEST2, 0x03);


	/// Contiki Original
	/* Set auto CRC on frame. */
#if CC2520_CONF_AUTOACK
	setreg(CC2520_FRMCTRL0, CC2520_AUTOCRC | CC2520_AUTOACK);
	setreg(CC2520_FRMFILT0, FRAME_MAX_VERSION|FRAME_FILTER_ENABLE); // accept all frames and filter it (ACK, etc)
	// minimum frame length is 3
#else
	/* setreg(CC2520_FRMCTRL0,    0x60); */
	setreg(CC2520_FRMCTRL0, CC2520_AUTOCRC);
	/* Disable filter on @ (remove if you want to address specific wismote) */
	setreg(CC2520_FRMFILT0, 0x00);
#endif /* CC2520_CONF_AUTOACK */



	/* SET_RXENMASK_ON_TX */
	setreg(CC2520_FRMCTRL1, 1);
	/* Set FIFOP threshold to maximum .*/
	setreg(CC2520_FIFOPCTRL, FIFOP_THR(0x7F));


#if BRTOS_ENDIAN == BIG_ENDIAN
	// Big endian
	uint8_t mac_16[2] = {MAC16_INIT_VALUE};
	uint8_t pan_16[2] = {PANID_INIT_VALUE};
	uint16_t mac_id = mac_16[0]<<8+mac_16[1];
	uint16_t pan_id = pan_16[0]<<8+pan_16[1];
#else
	// Little endian
	uint8_t mac_16[2] = {MAC16_INIT_VALUE};
	uint8_t pan_16[2] = {PANID_INIT_VALUE};
	uint16_t mac_id = (mac_16[1]<<8)+mac_16[0];
	uint16_t pan_id = (pan_16[1]<<8)+pan_16[0];
#endif
	cc2520_set_pan_addr(pan_id,mac_id,NULL);
	cc2520_set_channel(CHANNEL_INIT_VALUE);


	// Clear all buffers
	flushtx();
	flushrx();

	// Enable radio in reception mode
    strobe(CC2520_INS_SRXON);
    flushrx(); // See errata bug 1
	BUSYWAIT_UNTIL(status() & (BV(CC2520_XOSC16M_STABLE)), RTIMER_SECOND / 100);
	CC2520_ENABLE_FIFOP_INT();

	CC2520_PRINTF("cc2520: Radio initialized!\n");

	return 0;
}
/*---------------------------------------------------------------------------*/
int cc2520_prepare(const void *data, unsigned short len) {
	uint8_t total_len;

	total_len = len + FOOTER_LEN;

	CC2520_PRINTF("cc2520: sending %d bytes\n", len);

	/* Write packet to TX FIFO. */
	CC2520_WRITE_FIFO_BUF(&total_len, 1);
	CC2520_WRITE_FIFO_BUF(data, len);

	return 0;
}
/*---------------------------------------------------------------------------*/
int cc2520_transmit(void) {
//	uint8_t state;
	int i;

	/* The TX FIFO can only hold one packet. Make sure to not overrun
	 * FIFO by waiting for transmission to start here and synchronizing
	 * with the CC2520_TX_ACTIVE check in cc2520_send.
	 *
	 * Note that we may have to wait up to 320 us (20 symbols) before
	 * transmission starts.
	 */
#ifndef CC2520_CONF_SYMBOL_LOOP_COUNT
#error CC2520_CONF_SYMBOL_LOOP_COUNT needs to be set!!!
#else
#define LOOP_20_SYMBOLS CC2520_CONF_SYMBOL_LOOP_COUNT
#endif

	// CC2520 12 symbols is 192us
	//	320 / 20 = 16us per symbol - 12 symbols = 192us

	// Start transmitting
#if WITH_SEND_CCA
	/// TODO get warning "Turning off radio while transmitting, ending packet prematurely" from Cooja
	strobe(CC2520_INS_SRXON); // TX is aborted by SRXON, STXON, SROFF
	flushrx(); // See errata bug number one
	BUSYWAIT_UNTIL(status() & BV(CC2520_RSSI_VALID), RTIMER_SECOND / 10);
//	while(!(status() & BV(CC2520_RSSI_VALID)));
	strobe(CC2520_INS_STXONCCA); // Start transmission
#else /* WITH_SEND_CCA */
	strobe(CC2520_INS_STXON);
#endif /* WITH_SEND_CCA */

	/// Check if transmission occur without errors
	for (i = LOOP_20_SYMBOLS; i > 0; i--) {
		if (CC2520_SFD_IS_1) {

			if (!(status() & BV(CC2520_TX_ACTIVE))) {
				/* SFD went high but we are not transmitting. This means that
				 we just started receiving a packet, so we drop the transmission. */
				// Return the radio to RX
//				BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);
//				strobe(CC2520_INS_SRXON);
				NODESTAT_UPDATE(radiocol);
				UNET_RADIO.set(TX_STATUS, RADIO_TX_ERR_COLLISION); // RADIO_TX_WAIT
				UNET_RADIO.set(TX_RETRIES, 0);
				CC2520_PRINTF("cc2520: TX Collision\n");
				return RADIO_TX_ERR_COLLISION;
			}

			/* We wait until transmission has ended so that we get an
			 accurate measurement of the transmission time.*/
			//BUSYWAIT_UNTIL(getreg(CC2520_EXCFLAG0) & TX_FRM_DONE , RTIMER_SECOND / 100);
			BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);
//			while((status() & BV(CC2520_TX_ACTIVE))); // wait the end of transmission

			/**
			 * 	When AUTOACK is enabled, all frames that are
			 *	accepted by address filtering, have the acknowledge request flag
			 *	set and have a valid CRC, are automatically acknowledged 12
			 *	symbol periods after being received.
			 */

			/// TODO ACK Disabled until project specification is completed
			// Check if ACK message return is needed
			NODESTAT_UPDATE(radiotx);
			if (is_radio_tx_ack(cc2520_radio_params.param[RADIO_STATE])){ // will need to wait ack
				// TX sent, but no acked
				UNET_RADIO.set(TX_STATUS, RADIO_TX_WAIT);
//				radio_tx_acked(FALSE);
				CC2520_PRINTF("cc2520: TX WAIT\n"); // waiting ack
				cc2520_waiting_ack = true;
				return RADIO_TX_WAIT;
			} else {
				// Broadcast message, no ack needed
//				radio_tx_acked(TRUE);
				UNET_RADIO.set(TX_STATUS, RADIO_TX_ERR_NONE);
				UNET_RADIO.set(TX_RETRIES, 0);
				CC2520_PRINTF("cc2520: TX OK\n");
				return RADIO_TX_ERR_NONE;
			}
			return 0; //RADIO_TX_OK;
		}
	}

	/* If we are using WITH_SEND_CCA, we get here if the packet wasn't
	 transmitted because of other channel activity. */
	// Return the radio to RX
//	BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);
//	strobe(CC2520_INS_SRXON);
	NODESTAT_UPDATE(radiocol);
	UNET_RADIO.set(TX_STATUS, RADIO_TX_ERR_COLLISION); // RADIO_TX_WAIT
	UNET_RADIO.set(TX_RETRIES, 0);
	CC2520_PRINTF("cc2520: do_send() transmission never start\n");
	CC2520_PRINTF("cc2520: TX Collision\n");
	return RADIO_TX_ERR_COLLISION;
}

/*---------------------------------------------------------------------------*/
int cc2520_write(const void *buf, uint16_t len) {
	int ret = -1;

	// Error in packet size
	if (((len + FOOTER_LEN) <= 0) || ((len + FOOTER_LEN) > 127)) {
		return -1;
	}
	// Flush here to give the radio some more time to finish flushing
	flushtx();

	uint8_t *frame_control = (uint8_t *) buf;
	CC2520_PRINTF("cc2520: write: ");
	CC2520_PRINTF_PACKET(frame_control, len);
	/// Check with the IEEE frame control if ACK is needed
	/// NOTE maybe should be in unet_core
	if (frame_control[0] == 0x41) {
		CC2520_PRINTF("cc2520: ACK NOT needed!\n");
		radio_tx_ack(FALSE);
		radio_tx_acked(TRUE);
	}
	if (frame_control[0] == 0x61) {
		CC2520_PRINTF("cc2520: ACK needed!\n");
		radio_tx_ack(TRUE);
		radio_tx_acked(FALSE);
	}
//	radio_txing(TRUE);

	if (cc2520_prepare(buf, len)) return ret;

	CC2520_INTERRUPT_ENABLE_CLR();
//	if((ret = cc2520_transmit()) != RADIO_TX_WAIT) _isr_handler();
	ret = cc2520_transmit();
	CC2520_INTERRUPT_ENABLE_SET();

	return ret;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get message
 *
 *        This routine is used to retrieve a message stored in the RX_FIFO
 */
int32_t cc2520_get_rxfifo(uint8_t *buf, uint16_t buf_len) {
	volatile uint8_t len;
	uint8_t footer[2];

	// If reach here without FIFOP, some bug occured
	if (!CC2520_FIFOP_IS_1) {
		return 0;
	}
	CC2520_READ_FIFO_BYTE(len);
//	len = getreg(CC2520_RXFIFOCNT);
	if (len > FOOTER_LEN && len < buf_len) { // not shorter, neither longer
		CC2520_READ_FIFO_BUF(buf, len - FOOTER_LEN); /// TODO stuck HERE! why?
		CC2520_READ_FIFO_BUF(footer, FOOTER_LEN);
		len = len - FOOTER_LEN;
		
		/// Print PKT
		CC2520_PRINTF("cc2520: pkt: ");
		CC2520_PRINTF_PACKET(buf,len);
		CC2520_PRINTF("cc2520: rssi&lqi: ");
		CC2520_PRINTF_PACKET(footer,FOOTER_LEN);

		/**
		 * APPEND_DATA_MODE:
		 * When AUTOCRC = 0:Don’t care
		 * When AUTOCRC = 1:
		 * 0: RSSI + The crc_ok bit and the 7 bit correlation value is
		 * appended at the end of each received frame
		 * 1: RSSI + The crc_ok bit and the 7 bit SRCRESINDEX is
		 * appended at the end of each received frame. See Table 15 for
		 * details.
		 */
		cc2520_radio_params.st_radio_param.crc = (footer[1] & FOOTER1_CRC_OK) >> 7; // get the CRC bit
		cc2520_radio_params.st_radio_param.last_rssi = buf[0];
		cc2520_radio_params.st_radio_param.last_lqi = buf[1] & FOOTER1_CORRELATION;
	} else {
		/* Oops, out of synch */
		flushrx();
		len = 0;
	}

	if (CC2520_FIFOP_IS_1) {
		if (!CC2520_FIFO_IS_1) {
			/* Clean up in case of FIFO overflow!  This happens for every
			 * full length frame and is signaled by FIFOP = 1 and FIFO =
			 * 0. */
			flushrx();
		}
	}

	return len;
}
/*---------------------------------------------------------------------------*/
int cc2520_read(const void *buf, uint16_t *len) {
	*len = cc2520_get_rxfifo((uint8_t *) buf, CC2520_MAX_PACKET_LENGTH);
	/* Enable radio to RX, since it was disabled in interruption*/
	cc2520_rx_enable();
	CC2520_ENABLE_FIFOP_INT();
	return 0;
}


/*---------------------------------------------------------------------------*/
// \biref FIFOP interrupt, indicates RX activity
static unsigned char rxfifo_length;
extern INT8U iNesting;
#define interrupt(x) void __attribute__((interrupt (x)))
interrupt(PORT1_VECTOR) Radio_RX_Interrupt(void) {
	// ************************
	// Interruption entry
	// ************************
	OS_INT_ENTER();

	//// Page 53, Status Byte:
	//// "All instructions sent over the SPI to CC2520 result in a status byte being output on SO
	//// when the first byte of the instruction is clocked in on SI."

	// check if is CC2520 interruption
	if (P1IFG & BV(CC2520_FIFOP_PIN)) {
		// Clear interrupt flag
		CC2520_CLEAR_FIFOP_INT();

		// Disable radio (BRTOS)
		CC2520_DISABLE_FIFOP_INT();
		cc2520_rx_disable(); // avoid overflowing rx buffer
		// TODO this should not be disabled here, because the OS may not read the packet in time
		//      so if this packet is forget is OS fault, not driver problem

		// Check if it's a ACK message
		rxfifo_length = getreg(CC2520_RXFIFOCNT);
		// Probably it's an ACK message, if isn't it's just garbage
		if ( rxfifo_length == CC2520_ACK_LENGTH ){
			NODESTAT_UPDATE(radiorx);
//				CC2520_READ_FIFO_BUF(ack_packet, CC2520_ACK_LENGTH);
			// if(ack_packet[1] & 2) // it's ack
			// TODO check to be sure that is ACK packet FRM.CTRL = 2
			// If radio is TXing, then it's a ack message
			if(cc2520_waiting_ack == true){ // if doesn't reach here, ack is lost, don't wait
				RADIO_STATE_SET(TX_ACKED); // ACK packet received, transmission ACKED
				UNET_RADIO.set(TX_STATUS, RADIO_TX_ERR_NONE);
				UNET_RADIO.set(TX_RETRIES, 0);
				RADIO_STATE_SET(TX_OK); // unet_core handle
			}
			flushrx();
			cc2520_rx_enable();
			CC2520_ENABLE_FIFOP_INT();
			RADIO_STATE_RESET(RX_OK);
		}
		// Just received some information
		else if( rxfifo_length > CC2520_ACK_LENGTH && rxfifo_length < CC2520_MAX_PACKET_LENGTH){
			RADIO_STATE_SET(RX_OK);
			NODESTAT_UPDATE(radiorx);
			// Don't enable RX, since it'll be read by OS
		}
		// Not a valid information
		else{
			flushrx();
			cc2520_rx_enable();
			CC2520_ENABLE_FIFOP_INT();
			RADIO_STATE_RESET(RX_OK);
		}

		/*
		 * Even if the ack message was expected, since some other
		 * packet arrived, the ack message will not
		 */
		cc2520_waiting_ack = false;

		// Processa o unet_core_isr_handler
		_isr_handler();
	} else P1IFG = 0;

	// ************************
	// Interruption exit
	// ************************
	OS_INT_EXIT();
	// ************************
}

int _isr(const void *isr_handler) {
	_isr_handler = isr_handler;
	return 0;
}

/*---------------------------------------------------------------------------*/
const struct radio_driver cc2520_driver = { cc2520_init, cc2520_read,
		cc2520_write, cc2520_set, cc2520_get, _isr };
/*---------------------------------------------------------------------------*/

/** @} */
