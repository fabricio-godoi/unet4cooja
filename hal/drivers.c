
/**
* \file drivers.c
* \brief Microcontroller drivers
*
* This file contain the main microcontroller drivers: serial.
*
**/

#include "drivers.h"
#include "BRTOS.h"

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
 * \fn unsigned char getchar()
 * \brief Read data from UART buffer
 * \return The character read by UART
 */
extern BRTOS_Queue *Serial;
unsigned char getchar(){
	unsigned char read = '\0';

	//return READ_BUFFER_OK Data successfully read
	//return NO_ENTRY_AVAILABLE There is no more available entry in queue
	OSQueuePend(Serial, &read, 0);
	return read;
}

/**
 * \fn const unsigned char* gets()
 * \brief Read data from UART buffer
 * \return String with data storage, maximum length 50 bytes
 */
int gets(char *string){
	unsigned char i=0;
	do{
		string[i]=getchar();
	}while(string[i++]!='\0');
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
//	while (!(UART_UCxIFG&UART_UCTXRXIFG));	// USCI_xx TX buffer ready?
//	UART_TXBUF = *s;      		// Store one character to be sent

//	while (!(UART_UCxIFG&UART_UCTXRXIFG));		// USCI_xx TX buffer ready?
//	UART_TXBUF = LF;			   		// Add line feed (end of line)

	return size;
}
#endif





