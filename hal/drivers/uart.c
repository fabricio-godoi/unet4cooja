/*
 * uart.c
 *
 *  Created on: Mar 7, 2017
 *      Author: user
 */

#include "uart.h"

/** Pointer to control serial door */
BRTOS_Mutex *SerialResource;

/** Serial queue */
BRTOS_Queue *Serial;

/** Serial semaphore */
BRTOS_Sem *StringEvent;

/** Serial event detection */
BRTOS_Sem *SerialCallback;

/** Get value from uart */
volatile INT8U receive_byte;


// Function to acquire exclusive right to serial door
// TODO: as soon as possible, add timeout resource
void acquireUART(void)
{
  // Alloc the resource to serial door
  OSMutexAcquire(SerialResource,0);
}

// Function to release UART
void releaseUART(void)
{
  // Release the serial door resource
  OSMutexRelease(SerialResource);
}


void uart_init(INT8U priority){
#if (MCU == msp430f2617 || MCU == msp430f5437)
	UART_PxSEL |= UART_PIN;            // P3.4,5 = USCI_A0 TXD/RXD
	UART_CTLx = UCSSEL_2;     	// SMCLK

	// Information from MSP430 User Guide:
	// Table 15-4. Commonly Used Baud Rates, Settings, and Errors, UCOS16 = 0
#if (BAUD_RATE == 115200)
	// BOUD RATE 115200
	UART_UCxBR0  = 0x45;           // 115200 for 8Mhz
	UART_UCxBR1  = 0;
	UART_UCxMCTL = UCBRS2;
#elif (BAUD_RATE == 9600)
	// BOUD RATE 9600 (more stable)
	UART_UCxBR0  = 0x65;           // 9600 for 8Mhz
	UART_UCxBR1  = 0x3;
	UART_UCxMCTL = UCBRS1;
#else
#error "Baud rate not supported! Include it, or change it."
#endif

	UART_CTLx &= ~UCSWRST;		// **Initialize USCI state machine**
	UART_UCxIE |= UART_UCTXRXIE;          // Enable USCI_A0 RX interrupt
#endif


	// Create a mutex with counter = 1, notifying that the
	// resource is available after initialization
	// Maximum priority at resource access = priority
	assert(OSMutexCreate(&SerialResource,priority) == ALLOC_EVENT_OK);

	/** Create queue to read data */
	assert(OSQueueCreate(32, &Serial) == ALLOC_EVENT_OK);
	assert(OSSemCreate(0,&StringEvent) == ALLOC_EVENT_OK);
	SerialCallback = NULL;
}

/*------------------------------------------------------------------------------
* USCIA interrupt service routine
------------------------------------------------------------------------------*/
#define interrupt(x) void __attribute__((interrupt (x)))
interrupt(UART_USCI) USCIxRX_ISR(void)
{
	// ************************
	// Interrupt Enter
	// ************************
	OS_INT_ENTER();

	// Tratamento da interrupcao
	// Interruption handler
	if (UART_UCAxIV == 2) { //UCRXIFG
		UART_UCAxIV &= ~2;
		receive_byte = UART_RXBUF; /* Read input data */

		if (OSQueuePost(Serial, receive_byte) == BUFFER_UNDERRUN) {
			OSQueueClean(Serial);
		}

		// Notify application that command has arrived
		if(receive_byte == '\n'){
			OSSemPost(StringEvent);
			if(SerialCallback != NULL) OSSemPost(SerialCallback);
		}
	}
	else UART_UCAxIV = 0; // clear all interruptions call

	// ************************
	// Interrupt Exit
	// ************************
	OS_INT_EXIT();
	// ************************
}


void uart_callback(BRTOS_Sem *ev){
	SerialCallback = ev;
}
