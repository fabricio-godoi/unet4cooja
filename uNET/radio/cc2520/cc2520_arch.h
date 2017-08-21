/**
 * \addtogroup cc2520 CC2520 Driver
 *
 * @{
 */

/** 
 * \file   cc2520_arch.h
 * \brief  CC2520 Specific Arch Conf
 * \author Fabricio Negrisolo de Godoi <giovanni.pellerano@evilaliv3.org>
 * \date   2017-03-21
 */

#ifndef __CC2520_ARCH_H__
#define __CC2520_ARCH_H__


#include <stdint.h>
//#include "system.h"
#include "BRTOS.h"

#include "spi.h"

/* Pin Mapping */
//#define CC2520_RESETn                     PORTGbits.RG15
//#define CC2520_INT                        PORTAbits.RA15
//#define CC2520_CSn                        PORTFbits.RF12
//#define CC2520_WAKE                       PORTGbits.RG12
// TODO get from contiki config file
#define CC2520_CS_AS_IO			// not usable?
#define CC2520_CS_DS			// not usable?
#define CC2520_CS_LOW			P3OUT &= ~BIT0
#define CC2520_CS_HIGH			P3OUT |=  BIT0
#define CC2520_CS_DIR_IN		P3DIR &= ~BIT0
#define CC2520_CS_DIR_OUT		P3DIR |=  BIT0
#define CC2520_RESETn_AS_IO     // not usable?
#define CC2520_RESETn_DS        // not usable?
#define CC2520_RESETn_LOW       P4OUT &= ~BIT4
#define CC2520_RESETn_HIGH      P4OUT |=  BIT4
#define CC2520_RESETn_DIR_IN    P4DIR &= ~BIT4
#define CC2520_RESETn_DIR_OUT   P4DIR |=  BIT4
#define CC2520_WAKE_AS_IO       // not usable?
#define CC2520_WAKE_DS          // not usable?
#define CC2520_WAKE_LOW         P4OUT &= ~BIT3
#define CC2520_WAKE_HIGH        P4OUT |=  BIT3
#define CC2520_WAKE_DIR_IN      P4DIR &= ~BIT3
#define CC2520_WAKE_DIR_OUT     P4DIR |=  BIT3
 //#endif
#define CC2520_PIN_CLOCK_INIT

#define CC2520_INT_ENABLE()
#define CC2520_INTERRUPT_FLAG_CLR()		CC2520_CLEAR_FIFOP_INT()/*; CC2520_CLEAR_SFD_INT()*/   //from contiki
#define CC2520_INTERRUPT_ENABLE_CLR()   CC2520_DISABLE_FIFOP_INT()/*; CC2520_DISABLE_SFD_INT()*/
#define CC2520_INTERRUPT_ENABLE_SET()   CC2520_ENABLE_FIFOP_INT()/*; CC2520_ENABLE_SFD_INT()*/

/* Spi port Mapping */
#define CC2520_SPI_PORT_INIT()			spi_init()
#define CC2520_SPI_PORT_WRITE			SPI_WRITE
#define CC2520_SPI_PORT_READ			spi_read

/* RESET low/high */
#define CC2520_HARDRESET_LOW()            CC2520_RESETn_LOW     					///< RESET pin = 0
#define CC2520_HARDRESET_HIGH()           CC2520_RESETn_HIGH     					///< RESET pin = 1
#define CC2520_CSn_LOW()                  CC2520_CS_LOW     						///< CS pin = 0
#define CC2520_CSn_HIGH()                 CC2520_CS_HIGH     						///< CS pin = 1

#define CC2520_PIN_INIT()        			\
do {                                       	\
   CC2520_PIN_CLOCK_INIT;					\
   CC2520_CS_AS_IO;							\
   CC2520_RESETn_AS_IO;						\
   CC2520_WAKE_AS_IO;						\
   CC2520_CS_DS;							\
   CC2520_RESETn_DS;						\
   CC2520_WAKE_DS;							\
   CC2520_RESETn_LOW;						\
   CC2520_RESETn_DIR_OUT;					\
   CC2520_WAKE_LOW;							\
   CC2520_WAKE_DIR_OUT;						\
   CC2520_CS_HIGH;							\
   CC2520_CS_DIR_OUT;						\
   CC2520_INT_ENABLE();						\
   CC2520_WAKE_HIGH;						\
} while(0)


#endif /* __CC2520_ARCH_H__ */

/** @} */
