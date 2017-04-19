/*
 * uart.h
 *
 *  Created on: Mar 7, 2017
 *      Author: user
 */

#ifndef UART_H_
#define UART_H_

#include "BRTOS.h"

// Drivers configurations
#define UART_PRIORITY 			13
#define BAUD_RATE 				115200
#define ENABLE_STDIO_PRINTF 	TRUE

/**
 * UART configuration by MCU selection
 */
#if MCU == msp430f5437
#define UART_PxSEL		P5SEL
#define UART_PIN		BIT6|BIT7
#define UART_CTLx    	UCA1CTL1
#define UART_TXBUF		UCA1TXBUF
#define UART_RXBUF		UCA1RXBUF
#define UART_UCxBR0		UCA1BR0
#define UART_UCxBR1     UCA1BR1
#define UART_UCxMCTL    UCA1MCTL
#define UART_UCAxIV		UCA1IV
#define UART_UCxIE		UCA1IE
#define UART_UCTXRXIE	UCRXIE
#define UART_UCxIFG		UCA1IFG
#define UART_UCTXRXIFG	UCTXIFG
#define UART_USCI		USCI_A1_VECTOR

#elif MCU == msp430f2617
#define UART_PxSEL		P3SEL
#define UART_PIN		0x30
#define UART_CTLx		UCA0CTL1
#define UART_TXBUF		UCA0TXBUF
#define UART_RXBUF		UCA0RXBUF
#define UART_UCxBR0		UCA0BR0
#define UART_UCxBR1		UCA0BR1
#define UART_UCxMCTL	UCA0MCTL
#define UART_UCAxIV		UCA0IV
#define UART_UCxIE		IE2
#define UART_UCTXRXIE	UCA0RXIE
#define UART_UCxIFG		IFG2
#define UART_UCTXRXIFG	UCA0TXIFG
#define UART_USCI		USCIAB0RX_VECTOR

#elif (MCU == msp430fr5969)
#define UART_PxSEL		P2SEL0
#define UART_PIN		BIT5|BIT6
#define UART_CTLx    	UCA1CTL1
#define UART_TXBUF		UCA1TXBUF
#define UART_RXBUF		UCA1RXBUF
#define UART_UCxBR0		UCA1BR0
#define UART_UCxBR1     UCA1BR1
#define UART_UCxMCTL    UCA1MCTLW
#define UART_UCxIE		UCA1IE
#define UART_UCTXRXIE	UCRXIE
#define UART_UCxIFG		UCA1IFG
#define UART_UCTXRXIFG	UCTXIFG
#define UART_USCI		USCI_A1_VECTOR

#else
#error "UART not supported in this MCU!"
#endif



/* UART functions prototypes */
void acquireUART(void);
void releaseUART(void);
void uart_init(INT8U);
void uart_callback(BRTOS_Sem *ev);

#endif /* HAL_DRIVERS_UART_H_ */
