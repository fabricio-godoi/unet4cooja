#ifndef UNET_RADIO_CC2520_H_
#define UNET_RADIO_CC2520_H_

#include "BRTOS.h"
#include "radio.h"
#include <stdint.h>

extern const struct radio_driver cc2520_driver;

#define ADD_RSSI_AND_LQI_TO_PACKET 1

#define CC2520_DEFAULT_CHANNEL        CHANNEL_INIT_VALUE

#define CC2520_TX_ERR_NONE            0
#define CC2520_TX_ERR_NOTSPECIFIED    1
#define CC2520_TX_ERR_COLLISION       2
#define CC2520_TX_ERR_MAXRETRY        3
#define CC2520_TX_WAIT                4

enum RFSTATE
{ //CC2520               MRF24J40
	CC2520_XOSC  = 7,  //RTSEL2 = 7,
	CC2520_RSSI  = 6,  //RTSEL1 = 6,
	CC2520_EXCCA = 5,  //RX = 5,
	CC2520_EXCCB = 4,  //TX = 4,
	CC2520_DPUH  = 3,  //CALVCO = 3,
	CC2520_DPUL  = 2,  //SLEEP = 2,
	CC2520_TX    = 1,  //CALFIL = 1,
	CC2520_RX    = 0   //RESET = 0
};

/* Functions prototypes */
void cc2520_set_channel(uint8_t ch);
void cc2520_set_panid(uint8_t *id);
void cc2520_set_short_mac_addr(uint8_t *addr);
void cc2520_set_extended_mac_addr(uint8_t *addr);
void cc2520_get_short_mac_addr(uint16_t * addr);
void cc2520_get_short_pan_id(uint16_t *addr);
void cc2520_get_extended_mac_addr(uint64_t * addr);
void cc2520_set_tx_power(uint8_t pwr);
void cc2520_set_csma_par(uint8_t be, uint8_t nb);
uint8_t cc2520_get_status(void);
int32_t cc2520_set_txfifo(const uint8_t * buf, uint8_t buf_len);
int32_t cc2520_get_rxfifo(uint8_t * buf, uint16_t buf_len);


#define CC2520_TX_PWR_SET(large_val, small_val) ((large_val << 6) | (small_val << 3))


#if PROCESSOR==COLDFIRE_V1
#define MSBFIRST 1
#endif

typedef union _TX_status {
  uint8_t val;
#if MSBFIRST
  struct TX_bits {
	uint8_t TXNRETRY:2;
	uint8_t CCAFAIL:1;
	uint8_t TXG2FNT:1;
	uint8_t TXG1FNT:1;
	uint8_t TXG2STAT:1;
	uint8_t TXG1STAT:1;
    uint8_t TXNSTAT:1;
  } bits;
#else
  struct TX_bits {
    uint8_t TXNSTAT:1;
    uint8_t TXG1STAT:1;
    uint8_t TXG2STAT:1;
    uint8_t TXG1FNT:1;
    uint8_t TXG2FNT:1;
    uint8_t CCAFAIL:1;
    uint8_t TXNRETRY:2;
  } bits;
#endif
} TX_status;

typedef union _INT_status {
  uint8_t val;
#if MSBFIRST
  struct INT_bits {
	uint8_t SLPIF:1;
	uint8_t WAKEIF:1;
	uint8_t HSYMTMRIF:1;
	uint8_t SECIF:1;
	uint8_t RXIF:1;
	uint8_t TXG2IF:1;
	uint8_t TXG1IF:1;
	uint8_t TXNIF:1;
  } bits;
#else
  struct INT_bits {
    uint8_t TXNIF:1;
    uint8_t TXG1IF:1;
    uint8_t TXG2IF:1;
    uint8_t RXIF:1;
    uint8_t SECIF:1;
    uint8_t HSYMTMRIF:1;
    uint8_t WAKEIF:1;
    uint8_t SLPIF:1;
  } bits;
#endif
} INT_status;



/************************************************************************/
/* Additional SPI Macros for the CC2520 */
/************************************************************************/
/* Send a strobe to the CC2520 */
#define CC2520_STROBE(s)                                \
  do {                                                  \
    CC2520_SPI_ENABLE();                                \
    SPI_WRITE(s);                                       \
    CC2520_SPI_DISABLE();                               \
  } while (0)

/* Write to a register in the CC2520                         */
/* Note: the SPI_WRITE(0) seems to be needed for getting the */
/* write reg working on the Z1 / MSP430X platform            */
#define CC2520_WRITE_REG(adr,data)                                      \
  do {                                                                  \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE_FAST(CC2520_INS_MEMWR | ((adr>>8)&0xFF));                 \
    SPI_WRITE_FAST(adr & 0xff);                                         \
    SPI_WRITE_FAST((uint8_t) data);                                     \
    SPI_WAITFORTx_ENDED();                                              \
    CC2520_SPI_DISABLE();                                               \
  } while(0)


/* Read a register in the CC2520 */
#define CC2520_READ_REG(adr,data)                                       \
  do {                                                                  \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE((CC2520_INS_MEMRD | ((adr>>8)&0xFF)));                    \
    SPI_WRITE((adr & 0xFF));                                            \
    (void) SPI_RXBUF;													\
    SPI_READ(data);                                                     \
    CC2520_SPI_DISABLE();                                               \
  } while(0)

#define CC2520_READ_FIFO_BYTE(data)                                     \
  do {                                                                  \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE(CC2520_INS_RXBUF);                                        \
    (void)SPI_RXBUF;                                                    \
    SPI_READ(data);                                                     \
    clock_delay(1);                                                     \
    CC2520_SPI_DISABLE();                                               \
  } while(0)

#define CC2520_READ_FIFO_BUF(buffer,count)                              \
  do {                                                                  \
    uint8_t i;                                                          \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE(CC2520_INS_RXBUF);                                        \
    (void)SPI_RXBUF;                                                    \
    for(i = 0; i < (count); i++) {                                      \
      SPI_READ(((uint8_t *)(buffer))[i]);                               \
    }                                                                   \
    clock_delay(1);                                                     \
    CC2520_SPI_DISABLE();                                               \
  } while(0)

#define CC2520_WRITE_FIFO_BUF(buffer,count)                             \
  do {                                                                  \
    uint8_t i;                                                          \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE_FAST(CC2520_INS_TXBUF);                                   \
    for(i = 0; i < (count); i++) {                                      \
      SPI_WRITE_FAST(((uint8_t *)(buffer))[i]);                         \
      SPI_WAITFORTxREADY();                                             \
    }                                                                   \
    SPI_WAITFORTx_ENDED();                                              \
    CC2520_SPI_DISABLE();                                               \
  } while(0)

/* Write to RAM in the CC2520 */
#define CC2520_WRITE_RAM(buffer,adr,count)                              \
  do {                                                                  \
    uint8_t i;                                                          \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE_FAST(CC2520_INS_MEMWR | (((adr)>>8) & 0xFF));             \
    SPI_WRITE_FAST(((adr) & 0xFF));                                     \
    for(i = 0; i < (count); i++) {                                      \
      SPI_WRITE_FAST(((uint8_t*)(buffer))[i]);                          \
    }                                                                   \
    SPI_WAITFORTx_ENDED();                                              \
    CC2520_SPI_DISABLE();                                               \
  } while(0)

/* Read from RAM in the CC2520 */
#define CC2520_READ_RAM(buffer,adr,count)                               \
  do {                                                                  \
    uint8_t i;                                                          \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE(CC2520_INS_MEMRD | (((adr)>>8) & 0xFF));                  \
    SPI_WRITE(((adr) & 0xFF));                                          \
    SPI_RXBUF;                                                          \
    for(i = 0; i < (count); i++) {                                      \
      SPI_READ(((uint8_t*)(buffer))[i]);                                \
    }                                                                   \
    CC2520_SPI_DISABLE();                                               \
  } while(0)

/* Read status of the CC2520 */
#define CC2520_GET_STATUS(s)                                            \
  do {                                                                  \
    CC2520_SPI_ENABLE();                                                \
    SPI_WRITE(CC2520_INS_SNOP);                                         \
    s = SPI_RXBUF;                                                      \
    CC2520_SPI_DISABLE();                                               \
  } while (0)


#endif /* UNET_RADIO_CC2520_H_ */

