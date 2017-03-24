/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \addtogroup mrf24j40 MRF24J40 Driver
 *
 * @{
 */

/** 
 * \file   mrf24j40_arch.h
 * \brief  MRF24J40 Specific Arch Conf
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#ifndef __MRF24J40_ARCH_H__
#define __MRF24J40_ARCH_H__


#include <stdint.h>
//#include "system.h"
#include "BRTOS.h"

#if BRTOS_PLATFORM == BOARD_FRDM_KL25Z                                                                 ///  <-
#include "xhw_types.h"                                                                                 ///    |
#include "xsysctl.h"                                                                                   ///    |
#include "xgpio.h"                                                                                     ///    |
#include "xhw_gpio.h"                                                                                  ///    |
#include "xhw_ints.h"                                                                                  ///    |
#include "xcore.h"                                                                                     ///    |
#include "spi_cox.h"                                                                                   ///    |
#elif BRTOS_PLATFORM == BOARD_COLDUINO || BRTOS_PLATFORM == BOARD_ROTEADORCFV1                         ///    |
#include "utils.h"                                                                                     ///    |
#if BRTOS_PLATFORM == BOARD_COLDUINO			/// <-                                                 ///    |
#include "spi.h"								///   |                                                ///    |
#elif BRTOS_PLATFORM == BOARD_ROTEADORCFV1		///   |                                                ///    |
#include "drivers.h"							///   |                                                ///    |
#endif											/// <-                                                 ///    |
#elif (BRTOS_PLATFORM == __WISMOTE__ || BRTOS_PLATFORM == __Z1__ || BRTOS_PLATFORM == __SKY__)         ///    |
#include "spi.h"                                                                                       ///    |
#else                                                                                                  ///    |
#error "Please define your platform"                                                                   ///    |
#endif                                                                                                 ///  <-

/* Pin Mapping */
//#define MRF24J40_RESETn                     PORTGbits.RG15
//#define MRF24J40_INT                        PORTAbits.RA15
//#define MRF24J40_CSn                        PORTFbits.RF12
//#define MRF24J40_WAKE                       PORTGbits.RG12

#if BRTOS_PLATFORM == BOARD_FRDM_KL25Z

#define MRF24J40_CS_AS_IO		GPIOPinConfigure(GPIO_PC4_PC4)								///< Defines CS pin as IO
#define MRF24J40_CS_DS			GPIOPadConfigSet(GPIOC_BASE, GPIO_PIN_4, PORT_TYPE_DSE_HIGH)///< Defines CS pin drive strength to high
#define MRF24J40_CS_LOW       	GPIOPinReset(GPIOC_BASE, GPIO_PIN_4)     					///< CS pin = 0
#define MRF24J40_CS_HIGH     	GPIOPinSet(GPIOC_BASE, GPIO_PIN_4)     						///< CS pin = 1
#define MRF24J40_CS_DIR_IN    	xGPIODirModeSet(GPIOC_BASE, GPIO_PIN_4, xGPIO_DIR_MODE_IN)	///< CS direction pin = in
#define MRF24J40_CS_DIR_OUT   	xGPIODirModeSet(GPIOC_BASE, GPIO_PIN_4, xGPIO_DIR_MODE_OUT)	///< CS direction pin = out
#define MRF24J40_RESETn_AS_IO	GPIOPinConfigure(GPIO_PC10_PC10)							///< Defines RESETn pin as IO
#define MRF24J40_RESETn_DS		GPIOPadConfigSet(GPIOC_BASE, GPIO_PIN_10, PORT_TYPE_DSE_HIGH)///< Defines RESETn pin drive strength to high
#define MRF24J40_RESETn_LOW   	GPIOPinReset(GPIOC_BASE, GPIO_PIN_10)     					///< RESET pin = 0
#define MRF24J40_RESETn_HIGH  	GPIOPinSet(GPIOC_BASE, GPIO_PIN_10)     					///< RESET pin = 1
#define MRF24J40_RESETn_DIR_IN  	xGPIODirModeSet(GPIOC_BASE, GPIO_PIN_10, xGPIO_DIR_MODE_IN) ///< RESET direction pin = in
#define MRF24J40_RESETn_DIR_OUT  xGPIODirModeSet(GPIOC_BASE, GPIO_PIN_10, xGPIO_DIR_MODE_OUT)///< RESET direction pin = out
#define MRF24J40_WAKE_AS_IO		GPIOPinConfigure(GPIO_PC11_PC11)							///< Defines WAKE pin as IO
#define MRF24J40_WAKE_DS			GPIOPadConfigSet(GPIOC_BASE, GPIO_PIN_11, PORT_TYPE_DSE_HIGH)///< Defines WAKE pin drive strength to high
#define MRF24J40_WAKE_LOW     	GPIOPinReset(GPIOC_BASE, GPIO_PIN_11)     					///< WAKE pin = 0
#define MRF24J40_WAKE_HIGH    	GPIOPinSet(GPIOC_BASE, GPIO_PIN_11)     					///< WAKE pin = 1
#define MRF24J40_WAKE_DIR_IN    	xGPIODirModeSet(GPIOC_BASE, GPIO_PIN_11, xGPIO_DIR_MODE_IN) ///< WAKE direction pin = in
#define MRF24J40_WAKE_DIR_OUT    xGPIODirModeSet(GPIOC_BASE, GPIO_PIN_11, xGPIO_DIR_MODE_OUT)///< WAKE direction pin = out
#define MRF24J40_PIN_CLOCK_INIT	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC)					///< Init PHY pins clock  						///< CS pin = 1

#define MRF24J40_INT_ENABLE()				Enable_INT_Pin()
#define MRF24J40_INTERRUPT_FLAG_CLR()       xHWREG(PORTD_BASE+PORT_ISFR) |= xHWREG(PORTD_BASE+PORT_ISFR)  // Limpa flag da interrup��o externa
#define MRF24J40_INTERRUPT_ENABLE_CLR()		do{extern int radio_int_status; radio_int_status= 0;}while(0); GPIOPinIntDisable(GPIOD_BASE, GPIO_PIN_4)  					// desabilita interrup��o externa
#define MRF24J40_INTERRUPT_ENABLE_SET()		do{extern int radio_int_status; radio_int_status= 1;}while(0); GPIOPinIntEnable(GPIOD_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE)   // habilita interrup��o externa

/* Spi port Mapping */
#define MRF24J40_SPI_PORT_INIT()  			init_SPI(0)
#define MRF24J40_SPI_PORT_WRITE 			SPI0_SendChar
#define MRF24J40_SPI_PORT_READ  			SPI0_GetChar

void Enable_INT_Pin(void)
{
	// Enables pin clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	// Init pin as GPIO, with pull-up enabled
	PORTD_PCR4 = (uint32_t)((PORTD_PCR4 & (uint32_t)~(uint32_t)(
				  PORT_PCR_ISF_MASK |
				  PORT_PCR_MUX(0x06)
				 )) | (uint32_t)(
				  PORT_PCR_MUX(0x01) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK
				 ));

	// Enable interrupt as falling edge
	GPIOPinIntEnable(GPIOD_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);

	// Enable INT
	xIntEnable(INT_PORTD);
}

#elif BRTOS_PLATFORM == BOARD_COLDUINO || BRTOS_PLATFORM == BOARD_ROTEADORCFV1

#if BRTOS_PLATFORM == BOARD_COLDUINO
// Defines the radio pins
#define MRF24J40_CS           	PTAD       ///< CS pin  - TODO: pino compartilhado com SD (trocar)
#define MRF24J40_CS_DIR       	PTADD     	///< CS direction pin
#define MRF24J40_CS_PIN       	0
#define MRF24J40_RESETn        	PTFD       ///< RESET pin
#define MRF24J40_RESETn_DIR  	PTFDD     ///< RESET direction pin
#define MRF24J40_RESETn_PIN  	4
#define MRF24J40_WAKE         	PTAD       ///< WAKE pin
#define MRF24J40_WAKE_DIR   	PTADD     ///< WAKE direction pin
#define MRF24J40_WAKE_PIN   	2

#define MRF24J40_ISR_REG   		KBI1SC
#define MRF24J40_ISR_FLAG   	2
#define MRF24J40_INTERRUPT 		KBI1PE
#define MRF24J40_ISR_PIN   		0

#define MRF24J40_INT_ENABLE()				BITSET(MRF24J40_INTERRUPT,MRF24J40_ISR_PIN);
#define MRF24J40_INTERRUPT_FLAG_CLR()		BITSET(MRF24J40_ISR_REG,MRF24J40_ISR_FLAG);
#define MRF24J40_INTERRUPT_ENABLE_CLR()		BITCLEAR(MRF24J40_INTERRUPT,MRF24J40_ISR_PIN);
#define MRF24J40_INTERRUPT_ENABLE_SET()		BITSET(MRF24J40_INTERRUPT,MRF24J40_ISR_PIN);

// Defines activity LED pin
//#define ACTIVITY_LED     PTFD_PTFD3       ///< RF activity LED
#define ACTIVITY_LED_DD  PTFDD_PTFDD3     ///< activity LED direction
#define ACTIVITY_LED_DS  PTFDS_PTFDS3     ///< low power activity LED

/* Spi port Mapping */
#define MRF24J40_SPI_PORT_INIT()  			init_SPI(1)
#define MRF24J40_SPI_PORT_WRITE 			SPI1_Write
#define MRF24J40_SPI_PORT_READ  			SPI1_Read

#elif BRTOS_PLATFORM == BOARD_ROTEADORCFV1

// Defines the radio pins
#define MRF24J40_CS           	PTDD       ///< CS pin
#define MRF24J40_CS_DIR       	PTDDD     	///< CS direction pin
#define MRF24J40_CS_PIN       	3
#define MRF24J40_RESETn        	PTED       ///< RESET pin
#define MRF24J40_RESETn_DIR  	PTEDD     ///< RESET direction pin
#define MRF24J40_RESETn_PIN  	6
#define MRF24J40_WAKE         	PTHD       ///< WAKE pin
#define MRF24J40_WAKE_DIR   	PTHDD     ///< WAKE direction pin
#define MRF24J40_WAKE_PIN   	1

#define MRF24J40_ISR_REG   		TPM1C1SC
#define MRF24J40_ISR_FLAG   	7
#define MRF24J40_ISR_PIN   		6

#define MRF24J40_INT_ENABLE()				MRF24J40_ISR_REG=0x48;		\
											BITSET(MRF24J40_ISR_REG,MRF24J40_ISR_PIN);
#define MRF24J40_INTERRUPT_FLAG_CLR()		BITCLEAR(MRF24J40_ISR_REG,MRF24J40_ISR_FLAG);
#define MRF24J40_INTERRUPT_ENABLE_CLR()		BITCLEAR(MRF24J40_ISR_REG,MRF24J40_ISR_PIN);
#define MRF24J40_INTERRUPT_ENABLE_SET()		BITSET(MRF24J40_ISR_REG,MRF24J40_ISR_PIN);

// Defines activity LED pin
#define ACTIVITY_LED     //PTFD_PTFD3       ///< RF activity LED
#define ACTIVITY_LED_DD  //PTFDD_PTFDD3     ///< activity LED direction
#define ACTIVITY_LED_DS  //PTFDS_PTFDS3     ///< low power activity LED

/* Spi port Mapping */
#define MRF24J40_SPI_PORT_INIT()  			SPI_Init(2)
#define MRF24J40_SPI_PORT_WRITE 			SPI2_Write
#define MRF24J40_SPI_PORT_READ  			SPI2_Read

#else
#error "Error: unknown platform"
#endif

#define MRF24J40_CS_AS_IO 	 	BITSET(MRF24J40_CS_DIR, MRF24J40_CS_PIN);
#define MRF24J40_CS_DS
#define MRF24J40_CS_LOW		 	BITCLEAR(MRF24J40_CS,MRF24J40_CS_PIN);
#define MRF24J40_CS_HIGH	 	BITSET(MRF24J40_CS,MRF24J40_CS_PIN);
#define MRF24J40_CS_DIR_IN	 	BITCLEAR(MRF24J40_CS_DIR,MRF24J40_CS_PIN);
#define MRF24J40_CS_DIR_OUT  	BITSET(MRF24J40_CS_DIR,MRF24J40_CS_PIN);
#define MRF24J40_RESETn_AS_IO  	BITSET(MRF24J40_RESETn_DIR,MRF24J40_RESETn_PIN);
#define MRF24J40_RESETn_DS
#define MRF24J40_RESETn_LOW		BITCLEAR(MRF24J40_RESETn,MRF24J40_RESETn_PIN);
#define MRF24J40_RESETn_HIGH	BITSET(MRF24J40_RESETn,MRF24J40_RESETn_PIN);
#define MRF24J40_RESETn_DIR_IN	BITCLEAR(MRF24J40_RESETn_DIR,MRF24J40_RESETn_PIN);
#define MRF24J40_RESETn_DIR_OUT	BITSET(MRF24J40_RESETn_DIR,MRF24J40_RESETn_PIN);
#define MRF24J40_WAKE_AS_IO		BITSET(MRF24J40_WAKE_DIR,MRF24J40_WAKE_PIN);
#define MRF24J40_WAKE_DS
#define MRF24J40_WAKE_LOW		BITCLEAR(MRF24J40_WAKE,MRF24J40_WAKE_PIN);
#define MRF24J40_WAKE_HIGH		BITSET(MRF24J40_WAKE,MRF24J40_WAKE_PIN);
#define MRF24J40_WAKE_DIR_IN	BITCLEAR(MRF24J40_WAKE_DIR,MRF24J40_WAKE_PIN);
#define MRF24J40_WAKE_DIR_OUT	BITSET(MRF24J40_WAKE_DIR,MRF24J40_WAKE_PIN);
#define MRF24J40_PIN_CLOCK_INIT

#elif ( BRTOS_PLATFORM == __WISMOTE__ )

#define MRF24J40_CS_AS_IO			P3DIR // not usable?
#define MRF24J40_CS_DS					  // not usable?
#define MRF24J40_CS_LOW				P3OUT &= ~BIT0
#define MRF24J40_CS_HIGH			P3OUT |=  BIT0
#define MRF24J40_CS_DIR_IN			P3DIR &= ~BIT0
#define MRF24J40_CS_DIR_OUT			P3DIR |=  BIT0
#define MRF24J40_RESETn_AS_IO
#define MRF24J40_RESETn_DS
#define MRF24J40_RESETn_LOW         P4OUT &= ~BIT4
#define MRF24J40_RESETn_HIGH        P4OUT |=  BIT4
#define MRF24J40_RESETn_DIR_IN      P4DIR &= ~BIT4
#define MRF24J40_RESETn_DIR_OUT     P4DIR |=  BIT4
#define MRF24J40_WAKE_AS_IO
#define MRF24J40_WAKE_DS
#define MRF24J40_WAKE_LOW           P4OUT &= ~BIT3
#define MRF24J40_WAKE_HIGH          P4OUT |=  BIT3
#define MRF24J40_WAKE_DIR_IN        P4DIR &= ~BIT3
#define MRF24J40_WAKE_DIR_OUT       P4DIR |=  BIT3
 //#endif
#define MRF24J40_PIN_CLOCK_INIT

#define MRF24J40_INT_ENABLE()
#define MRF24J40_INTERRUPT_FLAG_CLR()
#define MRF24J40_INTERRUPT_ENABLE_CLR()
#define MRF24J40_INTERRUPT_ENABLE_SET()

/* Spi port Mapping */
#define MRF24J40_SPI_PORT_INIT()			spi_init()
#define MRF24J40_SPI_PORT_WRITE				spi_write
#define MRF24J40_SPI_PORT_READ				spi_read


#else
#error "Please define radio port for the platform"
#define MRF24J40_CS_AS_IO
#define MRF24J40_CS_DS
#define MRF24J40_CS_LOW
#define MRF24J40_CS_HIGH
#define MRF24J40_CS_DIR_IN
#define MRF24J40_CS_DIR_OUT
#define MRF24J40_RESETn_AS_IO
#define MRF24J40_RESETn_DS
#define MRF24J40_RESETn_LOW
#define MRF24J40_RESETn_HIGH
#define MRF24J40_RESETn_DIR_IN
#define MRF24J40_RESETn_DIR_OUT
#define MRF24J40_WAKE_AS_IO
#define MRF24J40_WAKE_DS
#define MRF24J40_WAKE_LOW
#define MRF24J40_WAKE_HIGH
#define MRF24J40_WAKE_DIR_IN
#define MRF24J40_WAKE_DIR_OUT
 //#endif
#define MRF24J40_PIN_CLOCK_INIT

#define MRF24J40_INT_ENABLE()
#define MRF24J40_INTERRUPT_FLAG_CLR()
#define MRF24J40_INTERRUPT_ENABLE_CLR()
#define MRF24J40_INTERRUPT_ENABLE_SET()

/* Spi port Mapping */
#define MRF24J40_SPI_PORT_INIT()
#define MRF24J40_SPI_PORT_WRITE
#define MRF24J40_SPI_PORT_READ

#endif

/* RESET low/high */
#define MRF24J40_HARDRESET_LOW()            MRF24J40_RESETn_LOW     					///< RESET pin = 0
#define MRF24J40_HARDRESET_HIGH()           MRF24J40_RESETn_HIGH     					///< RESET pin = 1
#define MRF24J40_CSn_LOW()                  MRF24J40_CS_LOW     						///< CS pin = 0
#define MRF24J40_CSn_HIGH()                 MRF24J40_CS_HIGH     						///< CS pin = 1


#define MRF24J40_PIN_INIT()        				\
do {                                        	\
   MRF24J40_PIN_CLOCK_INIT;						\
   MRF24J40_CS_AS_IO;							\
   MRF24J40_RESETn_AS_IO;						\
   MRF24J40_WAKE_AS_IO;							\
   MRF24J40_CS_DS;								\
   MRF24J40_RESETn_DS;							\
   MRF24J40_WAKE_DS;							\
   MRF24J40_RESETn_LOW;							\
   MRF24J40_RESETn_DIR_OUT;						\
   MRF24J40_WAKE_LOW;							\
   MRF24J40_WAKE_DIR_OUT;						\
   MRF24J40_CS_HIGH;							\
   MRF24J40_CS_DIR_OUT;							\
   MRF24J40_INT_ENABLE();						\
   MRF24J40_WAKE_HIGH;							\
} while(0)


#endif /* __MRF24J40_ARCH_H__ */

/** @} */
