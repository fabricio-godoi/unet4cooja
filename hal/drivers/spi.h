/*
 * spi.h
 *
 *  Created on: Mar 7, 2017
 *      Author: Fabricio Negrisolo de Godoi
 */

#ifndef SPI_H_
#define SPI_H_


#include "hardware.h"

/* SPI input/output registers. */
#define SPI_TXBUF UCB0TXBUF
#define SPI_RXBUF UCB0RXBUF

                                /* USART0 Tx ready? */
#define SPI_WAITFOREOTx() while ((UCB0STAT & UCBUSY) != 0)
                                /* USART0 Rx ready? */
//#define SPI_WAITFOREORx() BUSYWAIT_UNTIL(!((UCB0IFG & UCRXIFG) == 0), 4096)
// MSP430@16MHz ~ 20 =~ 6us, enough to read a byte from SPI
#define SPI_WAITFOREORx() 										\
	do{															\
		uint16_t max_time = 20;                              \
		while (((UCB0IFG & UCRXIFG) == 0) && (max_time-- > 0)); \
	}while(0)
                                /* USART0 Tx buffer ready? */
#define SPI_WAITFORTxREADY()  while ((UCB0IFG & UCTXIFG) == 0)
	//BUSYWAIT_UNTIL(!((UCB0IFG & UCTXIFG) == 0), 2)

/*                                 /\* USART0 Tx ready? *\/ */
/* #define SPI_WAITFOREOTx() while (!(UCB0IFG & UCRXIFG)) */
/*                                 /\* USART0 Rx ready? *\/ */
/* #define SPI_WAITFOREORx() while (!(UCB0IFG & UCRXIFG)) */
/*                                 /\* USART0 Tx buffer ready? *\/ */
/* #define SPI_WAITFORTxREADY() while (!(UCB0IFG & UCRXIFG)) */
/* #define SPI_BUSY_WAIT() 		while ((UCB0STAT & UCBUSY) == 1) */

#define MOSI           1  /* P3.1 - Output: SPI Master out - slave in (MOSI) */
#define MISO           2  /* P3.2 - Input:  SPI Master in - slave out (MISO) */
#define SCK            3  /* P3.3 - Output: SPI Serial Clock (SCLK) */
/* #define SCK            1  /\* P3.1 - Output: SPI Serial Clock (SCLK) *\/ */
/* #define MOSI           2  /\* P3.2 - Output: SPI Master out - slave in (MOSI) *\/ */
/* #define MISO           3  /\* P3.3 - Input:  SPI Master in - slave out (MISO) *\/ */

/*
 * SPI bus - M25P80 external flash configuration.
 */

#define FLASH_PWR       //3       /* P4.3 Output */
#define FLASH_CS        //4       /* P4.4 Output */
#define FLASH_HOLD      //7       /* P4.7 Output */

/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  //( P4OUT &= ~BV(FLASH_CS) )
#define SPI_FLASH_DISABLE() //( P4OUT |=  BV(FLASH_CS) )

#define SPI_FLASH_HOLD()               // ( P4OUT &= ~BV(FLASH_HOLD) )
#define SPI_FLASH_UNHOLD()              //( P4OUT |=  BV(FLASH_HOLD) )

/* Define macros to use for checking SPI transmission status depending
   on if it is possible to wait for TX buffer ready. This is possible
   on for example MSP430 but not on AVR. */
#ifdef SPI_WAITFORTxREADY
#define SPI_WAITFORTx_BEFORE() SPI_WAITFORTxREADY()
#define SPI_WAITFORTx_AFTER()
#define SPI_WAITFORTx_ENDED() SPI_WAITFOREOTx()
#else /* SPI_WAITFORTxREADY */
#define SPI_WAITFORTx_BEFORE()
#define SPI_WAITFORTx_AFTER() SPI_WAITFOREOTx()
#define SPI_WAITFORTx_ENDED()
#endif /* SPI_WAITFORTxREADY */

/* Write one character to SPI */
#define SPI_WRITE(data) \
  do { \
    SPI_WAITFORTx_BEFORE(); \
    SPI_TXBUF = data; \
    SPI_WAITFOREOTx(); \
  } while(0)

/* Write one character to SPI - will not wait for end
   useful for multiple writes with wait after final */
#define SPI_WRITE_FAST(data) \
  do { \
    SPI_WAITFORTx_BEFORE(); \
    SPI_TXBUF = data; \
    SPI_WAITFORTx_AFTER(); \
  } while(0)

/* Read one character from SPI */
#define SPI_READ(data) \
  do { \
    SPI_TXBUF = 0; \
    SPI_WAITFOREORx(); \
    data = SPI_RXBUF; \
  } while(0)

/* Flush the SPI read register */
#ifndef SPI_FLUSH
#define SPI_FLUSH() \
  do { \
    SPI_RXBUF; \
  } while(0)
#endif



//***********************************
// Functions prototypes here
extern unsigned char spi_busy;

void spi_init(void);
void spi_write(char *a, short l);
//void spi_read(char *a, short l);
//uint8_t spi_read(uint8_t *a, uint8_t l);
//***********************************

#endif /* SPI_H_ */
