/*
 * Copyright (c) 2011, Swedish Institute of Computer Science
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
 */


/*
 * spi.c
 *
 *  Created on: Mar 7, 2017
 *      Author: Fabricio Negrisolo de Godoi
 *     Details: Source from Contiki OS adapted to BRTOS
 */

#include "spi.h"

/** This enables some interrupt routines, not fully tested yet */
#define SPI_ENABLE_INTERRUPT_FEATURE 0

/*
 * This is SPI initialization code for the MSP430X architecture.
 *
 */


#ifndef BV
#define BV(x) (1 << x)
#endif

/** Serial queue */
#if SPI_ENABLE_INTERRUPT_FEATURE
BRTOS_Queue *SpiRecvQueue;
#endif

/*
 * Initialize SPI bus.
 */
void spi_init(void) {


	/** Create queue to read data */
//	assert(OSQueueCreate(128, &SpiRecvQueue) == ALLOC_EVENT_OK);


  // Initialize ports for communication with SPI units.
  UCB0CTL1 |=  UCSWRST;                //reset usci
  UCB0CTL1 |=  UCSSEL_2;               //smclk while usci is reset
  UCB0CTL0 = ( UCMSB | UCMST | UCSYNC | UCCKPL); // MSB-first 8-bit, Master, Synchronous, 3 pin SPI master, no ste, watch-out for clock-phase UCCKPH
  UCB0BR1 = 0x00;
//  UCB0BR0 = 0x00;
  UCB0BR0 = 0x02;

//  UCB0MCTL = 0;                       // Dont need modulation control.

  P3SEL |= BV(SCK) | BV(MOSI) | BV(MISO); // Select Peripheral functionality
  P3DIR |= BV(SCK) | BV(MISO);  // Configure as outputs(SIMO,CLK).

  //ME1   |= USPIE0;            // Module enable ME1 --> U0ME? xxx/bg

  // Clear pending interrupts before enable!!! /// TODO this is wrong on contiki!
//  UCB0IE &= ~UCRXIFG;
//  UCB0IE &= ~UCTXIFG;
  UCB0IFG &= ~UCRXIFG;
  UCB0IFG &= ~UCTXIFG;
  UCB0CTL1 &= ~UCSWRST;         // Remove RESET before enabling interrupts

  //Enable UCB0 Interrupts
#if SPI_ENABLE_INTERRUPT_FEATURE
  UCB0IE |= UCTXIE;              // Enable USCI_B0 TX Interrupts
  UCB0IE |= UCRXIE;              // Enable USCI_B0 RX Interrupts
#endif
}


//void spi_write(char *a, short l){
//	short i;
//	for(i=0;i<l;i++) SPI_WRITE(a[i]);
//}
//

//uint8_t spi_read(uint8_t *a, uint8_t l){
	//	for(i=0;i<l;i++) SPI_READ(a[i]);
//	uint8_t i=0;
//	while(i<l){
//		if(OSQueuePend(SpiRecvQueue, &a[i++], 1) == TIMEOUT){ // wait 1ms for each byte
//			// Return SPI error
//			return -1;
//		}
//	}
//	return i;
//}

#if SPI_ENABLE_INTERRUPT_FEATURE

volatile uint8_t receive_byte;
extern INT8U iNesting;
#define interrupt(x) void __attribute__((interrupt (x)))
interrupt(USCI_B0_VECTOR) Spi_Interrupt(void) {
	// ************************
	// Interruption entry
	// ************************
	OS_INT_ENTER();

	// Check if it's RX interruption
	if (UCB0IFG & UCRXIFG){
		UCB0IFG &= ~UCRXIFG;

		// Data received put it in the queue
		receive_byte = UCB0RXBUF; /* Read input data */
		if (OSQueuePost(SpiRecvQueue, receive_byte) == BUFFER_UNDERRUN) {
			OSQueueClean(SpiRecvQueue);
		}

	} else UCB0IFG = 0;

	// ************************
	// Interruption exit
	// ************************
	OS_INT_EXIT();
	// ************************
}
#endif /// SPI_ENABLE_INTERRUPT_FEATURE
