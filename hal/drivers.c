
/**
* \file drivers.c
* \brief Microcontroller drivers
*
* This file contain the main microcontroller drivers: serial.
*
**/

#include "drivers.h"
#include "mcu.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include <string.h>

/**
 * \brief Initialize all drivers from specific MCU
 */
void drivers_init(void *params){
	(void) params;

	//*********************************
	// Must be the first initialization
	mcu_init();
	//*********************************
	// Enable LEDs modules
	leds_init();
	//*********************************
	// Enable stdio uart
	uart_init(UART_PRIORITY);
	//*********************************
	// Enable spi for radio
	spi_init();
}


// If its Cooja msp430 GCC
#ifndef putchar
int putchar(int c)
{
	while (!(UART_UCxIFG&UART_UCTXRXIFG));             // TX buffer ready?
	UART_TXBUF = c;  	/* Armazena o caracter a ser transmitido no registrador de transmiss�o */
	return 1;
}
#endif

/**
 * \brief Read data from UART queue;
 * 		  A task exits a pending state with a queue post or by timeout.
 * \param *c Character read
 * \param timeout Timeout to the queue pend exits
 * \return The read status from the queue
 */
extern BRTOS_Queue *Serial;
uint8_t getchar(uint8_t *c, ostick_t time_wait){
	*c = (uint8_t) '\0';

	//return READ_BUFFER_OK Data successfully read
	//return NO_ENTRY_AVAILABLE There is no more available entry in queue
	return OSQueuePend(Serial, c, time_wait);
}

/**
 * \fn const unsigned char* gets()
 * \brief Read data from UART buffer
 * \return String with data storage, maximum length 50 bytes
 */
int gets(uint8_t *string){
	unsigned char i=0;
	while(getchar(&string[i++],NO_TIMEOUT) == READ_BUFFER_OK);
	string[i++]='\0';
	return i;
}

// TI GCC
#ifndef puts
int puts(const char *s)
{
	int size = 0;

	while(*s){
		while (!(UART_UCxIFG&UART_UCTXRXIFG));	// TX buffer ready?
		UART_TXBUF = *s++;      		// Store one character to be sent
		size++;
	}

	return size;
}
#endif

