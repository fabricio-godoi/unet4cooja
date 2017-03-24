/*
 * debugdco.c
 *
 *  Created on: Nov 9, 2016
 *      Author: user
 */



#include "debugdco.h"

#if (DEBUGDCO_ENABLE == 1)
#include "leds.h"
#include "drivers.h"

volatile unsigned long debugdco_counter;
volatile unsigned short debug_tbcounter;
#define CINTERVAL	1000

#if MCU == msp430f5437
#define TBIV_TBCCR1 TB0IV_TBCCR1
#elif MCU == msp430f2617
#else
#error "debugdco: MCU not supported!\n"
#endif // END MCU Check

#define SHORT_DIGITS 5
const char* itoa (unsigned short i){
	// max value of 16bits = 0 to 65535

	/* Room for INT_DIGITS digits, - and '\0' */
	  static char buf[SHORT_DIGITS + 2];
	  char *p = buf + SHORT_DIGITS + 1;	/* points to terminating '\0' */
	  if (i >= 0) {
	    do {
	      *--p = '0' + (i % 10);
	      i /= 10;
	    } while (i != 0);
	    return p;
	  }
	  else {			/* i < 0 */
	    do {
	      *--p = '0' - (i % 10);
	      i /= 10;
	    } while (i != 0);
	    *--p = '-';
	  }
	  return p;

}

void debugdco_init(){
	debugdco_counter = 0;

	__enable_interrupt();

	// Use TimerB0 with SMCLK compare mode with 1000 counts
	TBCTL = TBSSEL1 | MC1;

	/* Initialize ccr1 to create the X ms interval. */
	/* CCR1 interrupt enabled, interrupt occurs when timer equals CCR. */
	TBCCTL1 = CCIE;

	// Start script
	PRINTF("S\n");

	/* Interrupt after X ms. */
	TBCCR1 = CINTERVAL;  // Start count

	/* Trap process */
	while(1);
}

/*
 * TODO:
 * Wismote and Z1 mote:
 *	Timer Interruption doesn't occour when the clock source isn't ACLK!
 *	TBR is incrementing
 *	Cannot put breakpoint in interruption function
 */


#define interrupt(x) void __attribute__((interrupt (x)))
interrupt(TIMERB1_VECTOR) TIMERB_DEBUGDCO(void)
{
//	TBCTL &= ~TBIFG;
	if(TBIV & TBIV_TBCCR1) { //// Capture/Compare 1
		TBIV &= ~TBIV_TBCCR1; // clear interrupt flag
		 debugdco_counter++;
		 TBCCR1 += CINTERVAL;
		 if(TBCCR1 == 0) TBCCR1 = 1; // prevent timer shutdown
	}
}

#endif //(DEBUGDCO_ENABLE == 1)
