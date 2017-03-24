#include <stdlib.h>
//#include "printf_lib.h"
#include "mrf24j40.h"
#include "ieee802154.h"
#include "mrf24j40_arch.h"

static uint32_t (*_isr_handler)(void);
static radio_params_t mrf24j40_radio_params;


void clock_delay_usec(uint16_t dt){
    int i=0;
	for(i=0;i<dt*6;i++)
    {
    };
}

/*---------------------------------------------------------------------------*/
static void set_short_add_mem(uint8_t addr, uint8_t val) {
	OS_SR_SAVE_VAR;
	uint8_t msg[2];

	msg[0] = (addr << 1) | 0x01;
	msg[1] = val;

	OSEnterCritical();

	MRF24J40_CSn_LOW();
	MRF24J40_SPI_PORT_WRITE(msg, 2);
	MRF24J40_CSn_HIGH();

	OSExitCritical();
}
/*---------------------------------------------------------------------------*/
static void set_long_add_mem(uint16_t addr, uint8_t val) {
	OS_SR_SAVE_VAR;
	uint8_t msg[3];
	volatile uint16_t addr2 = addr;

	msg[0] = (((uint8_t)(addr2 >> 3)) & 0x7F) | 0x80;
	msg[1] = (((uint8_t)(addr2 << 5)) & 0xE0) | 0x10;
	msg[2] = val;

	OSEnterCritical();

	MRF24J40_CSn_LOW();
	MRF24J40_SPI_PORT_WRITE(msg, 3);
	MRF24J40_CSn_HIGH();

	OSExitCritical();
}
/*---------------------------------------------------------------------------*/
static uint8_t get_short_add_mem(uint8_t addr) {
	OS_SR_SAVE_VAR;
	uint8_t ret_val;
	uint8_t addr2;
	addr2 = (addr << 1);

	OSEnterCritical();

	MRF24J40_CSn_LOW();
	MRF24J40_SPI_PORT_WRITE(&addr2, 1);
	MRF24J40_SPI_PORT_READ(&ret_val, 1);
	MRF24J40_CSn_HIGH();

	OSExitCritical();

	return ret_val;
}
/*---------------------------------------------------------------------------*/
static uint8_t get_long_add_mem(uint16_t addr) {
	OS_SR_SAVE_VAR;
	uint8_t ret_val;
	uint8_t msg[2];

	msg[0] = (((uint8_t)(addr >> 3)) & 0x7F) | 0x80;
	msg[1] = ((uint8_t)(addr << 5)) & 0xE0;

	OSEnterCritical();

	MRF24J40_CSn_LOW();
	MRF24J40_SPI_PORT_WRITE(msg, 2);
	MRF24J40_SPI_PORT_READ(&ret_val, 1);
	MRF24J40_CSn_HIGH();

	OSExitCritical();

	return ret_val;
}
/*---------------------------------------------------------------------------*/
static void reset_rf_state_machine(void) {
	/*
	 * Reset RF state machine
	 */

	const uint8_t rfctl = get_short_add_mem(MRF24J40_RFCTL);

	set_short_add_mem(MRF24J40_RFCTL, rfctl | 0b00000100);
	set_short_add_mem(MRF24J40_RFCTL, rfctl & 0b11111011);

	clock_delay_usec(200);
}
/*---------------------------------------------------------------------------*/
void flush_rx_fifo(void) {
	set_short_add_mem(MRF24J40_RXFLUSH,
	get_short_add_mem(MRF24J40_RXFLUSH) | 0b00000001);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the channel
 *
 *        This routine sets the rx/tx channel
 */
void mrf24j40_set_channel(uint8_t ch) {
	set_long_add_mem(MRF24J40_RFCON0, ((ch - 11) << 4) | 0b00000011);

	reset_rf_state_machine();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store MAC PAN ID
 *
 *        This routine sets the MAC PAN ID in the MRF24J40.
 */
void mrf24j40_set_panid(uint8_t *id) {
#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
	set_short_add_mem(MRF24J40_PANIDL, *id++);
	set_short_add_mem(MRF24J40_PANIDH, *id);
#else
	set_short_add_mem(MRF24J40_PANIDH, *id++);
	set_short_add_mem(MRF24J40_PANIDL, *id);
#endif	
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store short MAC address
 *
 *        This routine sets the short MAC address in the MRF24J40.
 */
void mrf24j40_set_short_mac_addr(uint8_t *addr) {
#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
	set_short_add_mem(MRF24J40_SADRL, *addr++);
	set_short_add_mem(MRF24J40_SADRH, *addr);
#else
	set_short_add_mem(MRF24J40_SADRH, *addr++);
	set_short_add_mem(MRF24J40_SADRL, *addr);
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store extended MAC address
 *
 *        This routine sets the extended MAC address in the MRF24J40.
 */
void mrf24j40_set_extended_mac_addr(uint8_t *addr) {
	
#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
	set_short_add_mem(MRF24J40_EADR0, *addr++);
	set_short_add_mem(MRF24J40_EADR1, *addr++);
	set_short_add_mem(MRF24J40_EADR2, *addr++);
	set_short_add_mem(MRF24J40_EADR3, *addr++);
	set_short_add_mem(MRF24J40_EADR4, *addr++);
	set_short_add_mem(MRF24J40_EADR5, *addr++);
	set_short_add_mem(MRF24J40_EADR6, *addr++);
	set_short_add_mem(MRF24J40_EADR7, *addr);
#else	
	set_short_add_mem(MRF24J40_EADR7, *addr++);
	set_short_add_mem(MRF24J40_EADR6, *addr++);
	set_short_add_mem(MRF24J40_EADR5, *addr++);
	set_short_add_mem(MRF24J40_EADR4, *addr++);
	set_short_add_mem(MRF24J40_EADR3, *addr++);
	set_short_add_mem(MRF24J40_EADR2, *addr++);
	set_short_add_mem(MRF24J40_EADR1, *addr++);
	set_short_add_mem(MRF24J40_EADR0, *addr);
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get short MAC address
 *
 *        This routine gets the short MAC address stored in the MRF24J40.
 */
void mrf24j40_get_short_mac_addr(uint16_t *addr) {
	unsigned char *data = (unsigned char *) addr;

#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
	*data++ = get_short_add_mem(MRF24J40_SADRL);
	*data = get_short_add_mem(MRF24J40_SADRH);
#else
	*data++ = get_short_add_mem(MRF24J40_SADRH);
	*data = get_short_add_mem(MRF24J40_SADRL);
#endif
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Get short PAN id
 *
 *        This routine gets the short MAC address stored in the MRF24J40.
 */
void mrf24j40_get_short_pan_id(uint16_t *addr) {
	unsigned char *data = (unsigned char *) addr;

#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
	*data++ = get_short_add_mem(MRF24J40_PANIDL);
	*data = get_short_add_mem(MRF24J40_PANIDH);
#else
	*data++ = get_short_add_mem(MRF24J40_PANIDH);
	*data = get_short_add_mem(MRF24J40_PANIDL);
#endif
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Gets extended MAC address
 *
 *        This routine gets the extended MAC address stored in the MRF24J40.
 */
void mrf24j40_get_extended_mac_addr(uint64_t *addr) {
	unsigned char *addr2 = (unsigned char *) addr;

#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
	*addr2++ = get_short_add_mem(MRF24J40_EADR0);
	*addr2++ = get_short_add_mem(MRF24J40_EADR1);
	*addr2++ = get_short_add_mem(MRF24J40_EADR2);
	*addr2++ = get_short_add_mem(MRF24J40_EADR3);
	*addr2++ = get_short_add_mem(MRF24J40_EADR4);
	*addr2++ = get_short_add_mem(MRF24J40_EADR5);
	*addr2++ = get_short_add_mem(MRF24J40_EADR6);
	*addr2 = get_short_add_mem(MRF24J40_EADR7);
#else
	*addr2++ = get_short_add_mem(MRF24J40_EADR7);
	*addr2++ = get_short_add_mem(MRF24J40_EADR6);
	*addr2++ = get_short_add_mem(MRF24J40_EADR5);
	*addr2++ = get_short_add_mem(MRF24J40_EADR4);
	*addr2++ = get_short_add_mem(MRF24J40_EADR3);
	*addr2++ = get_short_add_mem(MRF24J40_EADR2);
	*addr2++ = get_short_add_mem(MRF24J40_EADR1);
	*addr2 = get_short_add_mem(MRF24J40_EADR0);
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set TX power
 *
 *        This routine sets the transmission power of the MRF24J40.
 */
void mrf24j40_set_tx_power(uint8_t pwr) {
	set_long_add_mem(MRF24J40_RFCON3, pwr);
}
/*---------------------------------------------------------------------------*/
static void mrf24j40_rx_disable(void)
{
	/* Disable packet reception */
	set_short_add_mem(MRF24J40_BBREG1, 0b00000100);
}
/*---------------------------------------------------------------------------*/
static void mrf24j40_rx_enable(void)
{
	/* Enable packet reception */
	set_short_add_mem(MRF24J40_BBREG1, 0b00000000);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get radio status
 *
 *        This routine returns the MRF24J40 status.
 */
uint8_t mrf24j40_get_status(void)
{
	return ((get_long_add_mem(MRF24J40_RFSTATE) & 0xE0) >> 5);
}

int mrf24j40_set(radio_opt_t opt, uint8_t value){
	mrf24j40_radio_params.param[opt] = value;
	switch(opt){
		case RADIO_STATE:
#if 0
			if(is_radio_rx_disabled(value))
			{
				set_short_add_mem(MRF24J40_BBREG1,0x04); /* disable radio rx */
			}else
			{
				set_short_add_mem(MRF24J40_RXFLUSH,0x01); /* flush rx buffer */
				set_short_add_mem(MRF24J40_BBREG1,0x00); /* enable radio rx */
			}
#endif
			break;
		case CHANNEL:
			//value = ((value - 11)%16)+11;
			mrf24j40_radio_params.param[opt] = value;
			mrf24j40_set_channel(value);
			break;
		case MACADDR16H:
			set_short_add_mem(MRF24J40_SADRH, value);
			break;
		case MACADDR16L:
			set_short_add_mem(MRF24J40_SADRL, value);
			break;
		case PANID16H:
			set_short_add_mem(MRF24J40_PANIDH, value);
			break;
		case PANID16L:
			set_short_add_mem(MRF24J40_PANIDL, value);
			break;
		case MACADDR64_7:
			set_short_add_mem(MRF24J40_EADR7, value);
			break;
		case MACADDR64_6:
			set_short_add_mem(MRF24J40_EADR6, value);
			break;
		case MACADDR64_5:
			set_short_add_mem(MRF24J40_EADR5, value);
			break;
		case MACADDR64_4:
			set_short_add_mem(MRF24J40_EADR4, value);
			break;
		case MACADDR64_3:
			set_short_add_mem(MRF24J40_EADR3, value);
			break;
		case MACADDR64_2:
			set_short_add_mem(MRF24J40_EADR2, value);
			break;
		case MACADDR64_1:
			set_short_add_mem(MRF24J40_EADR1, value);
			break;
		case MACADDR64_0:
			set_short_add_mem(MRF24J40_EADR0, value);
			break;
		#if 0
		case MACADDR16:
			mrf24j40_set_short_mac_addr(value);
			break;
		case MACADDR64:
			mrf24j40_set_extended_mac_addr(value);
			break;
		case PANID:
			mrf24j40_set_panid(value);
			mrf24j40_radio_params.param[opt] = value;
			break;
		#endif
		case TXPOWER:
			if(value > 31) value = 31;
			mrf24j40_radio_params.param[opt] = value;
			mrf24j40_set_tx_power(MRF24J40_TX_PWR_SET(((value >> 3) & 0x3),(value & 0x7)));
			break;
		default:
			return -1;
			break;
	}
	return 0;
}

int mrf24j40_get(radio_opt_t opt, uint8_t *value)
{
	*value =  mrf24j40_radio_params.param[opt];
	switch(opt)
	{
		case RADIO_STATUS:
			*value = mrf24j40_get_status();
			switch((enum RFSTATE)*value)
			{
				case RESET:
					*value = eIDLE;
					break;
				case RX:
					*value = eRX;
					break;
				case TX:
					*value = eTX;
					break;
				case SLEEP:
					*value = eSLEEP;
					break;
				default:
					*value = eUNKNOWN;
					break;
			}
			mrf24j40_radio_params.param[opt] = *value;
			break;
		default:
			break;
	}

	return 0;
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Store message
 *
 *        This routine stores a buffer of buf_len bytes in the TX_FIFO
 *        buffer of the MRF24J40.
 */
int32_t mrf24j40_set_txfifo(const uint8_t *buf, uint8_t buf_len) {
	uint8_t i;

	if ((buf_len == 0) || (buf_len > 128)) {
		return -1;
	}

	for (i = 0; i < buf_len; ++i) {
		set_long_add_mem(2 + i, buf[i]);
	}

	set_long_add_mem(0, 9);

	set_long_add_mem(1, buf_len);

	return 0;
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Get message
 *
 *        This routine is used to retrieve a message stored in the RX_FIFO
 */
int32_t mrf24j40_get_rxfifo(uint8_t *buf, uint16_t buf_len) {
	volatile uint8_t i, len;

	MRF24J40_INTERRUPT_ENABLE_CLR();

	/* Disable packet reception */
	mrf24j40_rx_disable();

	/* Get packet length discarding 2 bytes (LQI, RSSI) */
	len = get_long_add_mem(MRF24J40_RX_FIFO);

	if (len <= buf_len) {
		/* Get the packet */
		for (i = 0; i < len; ++i) {
			buf[i] = get_long_add_mem(MRF24J40_RX_FIFO + i + 1);
		}

		/*
		 * packet len includes = header + paylod + LQI + RSSI
		 */
#ifdef ADD_RSSI_AND_LQI_TO_PACKET
		mrf24j40_radio_params.st_radio_param.last_lqi = get_long_add_mem(MRF24J40_RX_FIFO + len + 1);
		mrf24j40_radio_params.st_radio_param.last_rssi = get_long_add_mem(MRF24J40_RX_FIFO + len + 2);
		//mrf24j40_driver.set(LQI, get_long_add_mem(MRF24J40_RX_FIFO + len + 3));
		//mrf24j40_driver.set(RSSI, get_long_add_mem(MRF24J40_RX_FIFO + len + 4));
#endif
	} else {
		len = 0;
	}

//#ifdef MRF24J40_PROMISCUOUS_MODE
	/*
	 * Flush RX FIFO as suggested by the work around 1 in
	 * MRF24J40 Silicon Errata.
	 */
	//flush_rx_fifo();
//#endif

	/* Enable packet reception */
	mrf24j40_rx_enable();

	MRF24J40_INTERRUPT_ENABLE_SET();


	return len == 0 ? -1 : len;
}


/*---------------------------------------------------------------------------*/
/**
 * \brief Init transceiver
 *
 *        This routine initializes the radio transceiver
 */
int mrf24j40_init(const void *isr_handler) {

	uint8_t i;
	const addr64_t mac_addr64 = {.u8 = {PANID_INIT_VALUE,0x00,0x00,0x00,0x00,MAC16_INIT_VALUE}};
	//uint16_t shortaddr;
	//uint16_t panid;

	_isr_handler = isr_handler;

	MRF24J40_SPI_PORT_INIT();

	/* Set the IO pins direction */
	MRF24J40_PIN_INIT();

	/* Enable Activity LED */
#ifdef ACTIVITY_LED
   ACTIVITY_LED_CLOCK_INIT;
   ACTIVITY_LED_AS_IO;
   ACTIVITY_LED_DS;
   ACTIVITY_LED_LOW;
   ACTIVITY_LED_DIR_OUT;
#endif

	MRF24J40_HARDRESET_LOW();

	clock_delay_usec(2500);

	MRF24J40_HARDRESET_HIGH();

	clock_delay_usec(2500);

	/*
	 * bit 7:3 reserved: Maintain as ‘0’
	 * bit 2   RSTPWR: Power Management Reset bit
	 *         1 = Reset power management circuitry (bit is automatically cleared to ‘0’ by hardware)
	 * bit 1   RSTBB: Baseband Reset bit
	 *         1 = Reset baseband circuitry (bit is automatically cleared to ‘0’ by hardware)
	 * bit 0   RSTMAC: MAC Reset bit
	 *         1 = Reset MAC circuitry (bit is automatically cleared to ‘0’ by hardware)
	 */
	set_short_add_mem(MRF24J40_SOFTRST, 0b00000111);

	/*
	 * wait until the radio reset is completed
	 */
	do {
		i = get_short_add_mem(MRF24J40_SOFTRST);
	} while ((i & 0b0000111) != 0);

	clock_delay_usec(2500);

	/*
	 * bit 7   FIFOEN: FIFO Enable bit 1 = Enabled (default). Always maintain this bit as a ‘1’.
	 * bit 6   reserved: Maintain as ‘0’
	 * bit 5:2 TXONTS<3:0>: Transmitter Enable On Time Symbol bits(1)
	 *         Transmitter on time before beginning of packet. Units: symbol period (16 μs).
	 *         Minimum value: 0x1. Default value: 0x2 (2 * 16 μs = 32 μs). Recommended value: 0x6 (6 * 16 μs = 96 μs).
	 * bit 1:0 TXONT<8:7>: Transmitter Enable On Time Tick bits(1)
	 *         Transmitter on time before beginning of packet. TXONT is a 9-bit value. TXONT<6:0> bits are located
	 *         in SYMTICKH<7:1>. Units: tick (50 ns). Default value = 0x028 (40 * 50 ns = 2 μs).
	 */

	//Mudando
	set_short_add_mem(MRF24J40_RFCTL, 0x04);//Adicionado!
	set_short_add_mem(MRF24J40_RFCTL, 0x00);//Adicionado!
	clock_delay_usec(2500); //Adicionado!

	set_short_add_mem(MRF24J40_PACON2, 0b10011000); // WRITE_FFOEN, 0x98

	//set_short_add_mem(MRF24J40_TXSTBL, 0x95); // Adicionado!

	mrf24j40_set_channel(CHANNEL_INIT_VALUE);

	set_long_add_mem(MRF24J40_RFCON1, 0b00000010); /* program the RF and Baseband Register */
	/* as suggested by the datasheet */

	set_long_add_mem(MRF24J40_RFCON2, 0b10000000); /* enable PLL */

	mrf24j40_set_tx_power(0b00000000); /* set power 0dBm (plus 20db power amplifier 20dBm)*/

	/*
	 * Set up
	 *
	 * bit 7   '1' as suggested by the datasheet
	 * bit 6:5 '00' reserved
	 * bit 4   '1' recovery from sleep 1 usec
	 * bit 3   '0' battery monitor disabled
	 * bit 2:0 '000' reserved
	 */
	set_long_add_mem(MRF24J40_RFCON6, 0b10010000);

	set_long_add_mem(MRF24J40_RFCON7, 0b10000000); /* Sleep clock = 100kHz */
	set_long_add_mem(MRF24J40_RFCON8, 0b00000010); /* as suggested by the datasheet */

	set_long_add_mem(MRF24J40_SLPCON1, 0b00100001); /* as suggested by the datasheet */

	/* Program CCA, RSSI threshold values */
	set_short_add_mem(MRF24J40_BBREG2, 0b01111000); /* Recommended value by the datashet */ //N�o tem
	set_short_add_mem(MRF24J40_CCAEDTH, 0b01100000); /* Recommended value by the datashet */ //N�o tem

#ifdef MRF24J40MB
	/* Activate the external amplifier needed by the MRF24J40MB */
	set_long_add_mem(MRF24J40_TESTMODE, 0b0001111);
#endif

#ifdef ADD_RSSI_AND_LQI_TO_PACKET
	/* Enable the packet RSSI */
	set_short_add_mem(MRF24J40_BBREG6, 0b01000000);
#endif

	/*
	 * Wait until the radio state machine is not on rx mode
	 */
	do {
		i = get_long_add_mem(MRF24J40_RFSTATE);
	} while ((i & 0xA0) != 0xA0);

	i = 0;

#ifdef MRF24J40_DISABLE_AUTOMATIC_ACK
	i = i | 0b00100000;
#endif

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
	i = i | 0b00001100;
	set_short_add_mem(MRF24J40_ORDER, 0b11111111);
#endif

#if (UNET_DEVICE_TYPE == ROUTER)
	i = i | 0b00000100;
#endif

#ifdef MRF24J40_ACCEPT_WRONG_CRC_PKT
	i = i | 0b00000010;
#endif

#ifdef MRF24J40_PROMISCUOUS_MODE
	i = i | 0b00000001;
#endif

	/*
	 * Set the RXMCR register.
	 * Default setting i = 0x00, which means:
	 * - Automatic ACK;
	 * - Device is or is not a PAN coordinator;
	 * - Device is or is not a coordinator;
	 * - Accept only packets with good CRC
	 * - Discard packet when there is a MAC address mismatch,
	 *   illegal frame type, dPAN/sPAN or MAC short address mismatch.
	 */
	set_short_add_mem(MRF24J40_RXMCR, i);

	/*
	 * Set the TXMCR register.
	 * bit 7   '0' Enable No Carrier Sense Multiple Access (CSMA) Algorithm.
	 * bit 6   '0' Disable Battery Life Extension Mode bit.
	 * bit 5   '0' Disable Slotted CSMA-CA Mode bit.
	 * bit 4:3 '11' MAC Minimum Backoff Exponent bits (macMinBE).
	 * bit 2:0 '100' CSMA Backoff bits (macMaxCSMABackoff)
	 */
	set_short_add_mem(MRF24J40_TXMCR, 0b00011100);

	i = get_short_add_mem(MRF24J40_TXMCR);

	/*
	 * Set TX turn around time as defined by IEEE802.15.4 standard
	 */
	set_short_add_mem(MRF24J40_TXSTBL, 0b10010101);
	set_short_add_mem(MRF24J40_TXTIME, 0b00110000);

#ifdef INT_POLARITY_HIGH
	/* Set interrupt edge polarity high */
	set_long_add_mem(MRF24J40_SLPCON0, 0b00000011);
#else
	set_long_add_mem(MRF24J40_SLPCON0, 0b00000001);
#endif


	reset_rf_state_machine();

	/* Flush RX FIFO */
	flush_rx_fifo();

	/* Define os endere�os macAddr curto e longo */
	#if 0
	panid = PANID_INIT_VALUE;
	*((uint8_t *) &shortaddr) = ROUTER_AUTO_ASSOCIATION_MAC_ADDR >> 8;
	*((uint8_t *) &shortaddr + 1) = ROUTER_AUTO_ASSOCIATION_MAC_ADDR;
	*((uint8_t *) &longaddr) = ROUTER_AUTO_ASSOCIATION_MAC_ADDR >> 8;
	*((uint8_t *) &longaddr + 1) = ROUTER_AUTO_ASSOCIATION_MAC_ADDR;
	for (i = 2; i < sizeof(longaddr); ++i) {
		((uint8_t *) &longaddr)[i] = rand();
	}

	mrf24j40_driver.set(MACADDR16,	(uint8_t *)&panid);
	mrf24j40_driver.set(MACADDR16,	(uint8_t *)&shortaddr);
	mrf24j40_driver.set(MACADDR64,	(uint8_t *)&longaddr);
	#endif

	// Acho que n�o precisa do big e little endian aqui
	mrf24j40_set_extended_mac_addr((uint8_t *)&mac_addr64);

	/*
	 *
	 * Setup interrupts.
	 *
	 * set INTCON
	 * bit 7 '1' Disables the sleep alert interrupt
	 * bit 6 '1' Disables the wake-up alert interrupt
	 * bit 5 '1' Disables the half symbol timer interrupt
	 * bit 4 '1' Disables the security key request interrupt
	 * bit 3 '0' Enables the RX FIFO reception interrupt
	 * bit 2 '1' Disables the TX GTS2 FIFO transmission interrupt
	 * bit 1 '1' Disables the TX GTS1 FIFO transmission interrupt
	 * bit 0 '0' Enables the TX Normal FIFO transmission interrupt
	 */
	set_short_add_mem(MRF24J40_INTCON, 0b11110110);

	PRINTF("Radio inicializado com sucesso!\n\r");

	return 0;
}
/*---------------------------------------------------------------------------*/
int mrf24j40_prepare(const void *data, unsigned short len)
{

	MRF24J40_INTERRUPT_ENABLE_CLR();
		mrf24j40_set_txfifo(data, len);
	MRF24J40_INTERRUPT_ENABLE_SET();
	return 0;
}
/*---------------------------------------------------------------------------*/
int mrf24j40_transmit(void)
{
	uint8_t state;

	UNET_RADIO.get(RADIO_STATE,&state);

	if (is_radio_tx_ack(state))
	{
		// transmit packet with ACK requested
		// Para solicitar ACK, bit2 = 1
		set_short_add_mem(MRF24J40_TXNCON, 0b00000101);
	}else
	{
		// Para broadcast, sem auto ACK
		set_short_add_mem(MRF24J40_TXNCON, 0b00000001);
	}

	// O motivo de uma falha ou sucesso deve ser avaliado no unet_core
	return 0;
}
/*---------------------------------------------------------------------------*/
int mrf24j40_write(const void *buf, uint16_t len) {
	int ret = -1;
	uint8_t *frame_control = (uint8_t *)buf;

	if (*frame_control == 0x41)
	{
		radio_tx_ack(FALSE);
	}

	if (*frame_control == 0x61)
	{
		radio_tx_ack(TRUE);
	}

	if (mrf24j40_prepare(buf, len))
		return ret;

	ret = mrf24j40_transmit();

	return ret;
}

/*---------------------------------------------------------------------------*/
int mrf24j40_read(const void *buf, uint16_t *len) {
	*len = mrf24j40_get_rxfifo((uint8_t *)buf, 128);
	return 0;
}

/*---------------------------------------------------------------------------*/
void MRF24J40_ISR(void) {
	static INT_status int_status;
	static TX_status tx_status;

	int_status.val = get_short_add_mem(MRF24J40_INTSTAT);

	if (!int_status.val)
	{
		return;
	}

	if (int_status.bits.RXIF)
	{
		mrf24j40_rx_disable();
		radio_rxing(TRUE);
	}

	if (int_status.bits.TXNIF)
	{
		radio_txing(TRUE);
		tx_status.val = get_short_add_mem(MRF24J40_TXSTAT);

		if (tx_status.bits.TXNSTAT)
		{
			radio_tx_acked(FALSE);
			if (tx_status.bits.CCAFAIL)
			{
				UNET_RADIO.set(TX_STATUS,RADIO_TX_ERR_COLLISION);
				UNET_RADIO.set(TX_RETRIES,0);
			} else
			{
				UNET_RADIO.set(TX_STATUS,RADIO_TX_ERR_MAXRETRY);
				UNET_RADIO.set(TX_RETRIES,tx_status.bits.TXNRETRY);
			}
		} else
		{
			UNET_RADIO.set(TX_STATUS,RADIO_TX_ERR_NONE);
			UNET_RADIO.set(TX_RETRIES,0);
			radio_tx_acked(TRUE);
		}
	}

	// Handler do core n�o limpa flag de interrup��es
	MRF24J40_INTERRUPT_FLAG_CLR();

}

#if PROCESSOR == COLDFIRE_V1
#if !__GNUC__
#if (NESTING_INT == 1)
#pragma TRAP_PROC
#else
interrupt
#endif
#else
__attribute__ ((__optimize__("omit-frame-pointer")))
#endif
#endif
void Radio_Interrupt(void)
{

#if PROCESSOR == COLDFIRE_V1
#if __GNUC__
	OS_SAVE_ISR();
#endif
	OS_INT_ENTER();
#endif

	// Processa as flags de interrup��o
	MRF24J40_ISR();

	// Processa o unet_core_isr_handler
	_isr_handler();

	// ************************
	// Sa�da de interrup��o
	// ************************
#if PROCESSOR == COLDFIRE_V1
	OS_INT_EXIT();
	// ************************
#if __GNUC__
	OS_RESTORE_ISR();
#endif
#endif

#if PROCESSOR == ARM_Cortex_M0
	OS_INT_EXIT_EXT();
	// ************************
#endif
}


int _isr(const void *isr_handler)
{
	_isr_handler = isr_handler;
	return 0;
}

/*---------------------------------------------------------------------------*/
const struct radio_driver mrf24j40_driver = { mrf24j40_init, mrf24j40_read, mrf24j40_write, mrf24j40_set, mrf24j40_get, _isr};
/*---------------------------------------------------------------------------*/

/** @} */
