#include "BRTOS.h"
#include "HAL.h"
#include <signal.h>
#include <stdint.h>
#include "leds.h"


typedef struct{
	unsigned short error;
	unsigned short dcoctl;
	unsigned short bcsctl1;
}ERROR_TYPE;

// TODO: change it to local variable
volatile ERROR_TYPE e;

#if MCU == msp430f2617
void msp430_sync_dco(void) {
	e.error = 0xFFFF;
	uint16_t last;
	uint16_t diff;

	/* DELTA_2 assumes an ACLK of 32768 Hz */
#define DELTA_2  ((SYSTEM_CLOCK) / 32768) // 16MHz

	/* Select SMCLK clock, and capture on ACLK for TBCCR6 */
	TBCTL = TBSSEL1 | TBCLR;
	TBCCTL6 = CCIS0 + CM0 + CAP;
	/* start the timer */
	TBCTL |= MC1;

	while(1) {
		/* wait for next Capture */
		TBCCTL6 &= ~CCIFG;
		while(!(TBCCTL6 & CCIFG));
		last = TBCCR6;

		TBCCTL6 &= ~CCIFG;
		/* wait for next Capture - and calculate difference */
		while(!(TBCCTL6 & CCIFG));
		diff = TBCCR6 - last;

		// If the code reach any boundaries (bottom or upper) it end the calibration with the most accurate value
		// bottom boundary - DCOCTL - 0x0  BCSCTL1 - 0x80
		// top boundary    - DCOCTL - 0xFF BCSCTL1 - 0x8F
		if (diff < e.error){
			e.error = diff;
			e.dcoctl = DCOCTL;
			e.bcsctl1 = BCSCTL1;
		}

		/*   speed = diff; */
		/*   speed = speed * 32768; */
		/*   PRINTF("Last TAR diff:%d target: %ld ", diff, DELTA_2); */
		/*   PRINTF("CPU Speed: %lu DCOCTL: %d\n", speed, DCOCTL); */

		/* resynchronize the DCO speed if not at target */
		if(DELTA_2 == diff) {	// Error equals 0
			break;                            /* if equal, leave "while(1)" */
		} else if(DELTA_2 < diff) {         /* DCO is too fast, slow it down */

			if(DCOCTL == 0x00) {              /* Did DCO will role under? */
				DCOCTL = 0xFF;
				if(BCSCTL1 > 0x80) BCSCTL1--;   // BCSCTL1 bottom boundary
				else break;
			}
			else DCOCTL--;
		} else if(DELTA_2 > diff) {
			if(DCOCTL == 0xFF) {              /* Did DCO will role over? */
				DCOCTL = 0x0;
				if(BCSCTL1 < 0x8F) BCSCTL1++;	  // BCSCTL1 top boundary
				else break;
			}
			else DCOCTL++;
		}
	}

	// If the error isn't 0, get the most accurate value
	if(DELTA_2 != diff){
		DCOCTL = e.dcoctl;
		BCSCTL1 = e.bcsctl1;
	}

	// Set SMCLK as DCOCLK/8, SMCLK can be used for timers and uarts
//	BCSCTL2 |= DIVS0 + DIVS1;

	/* Stop the timer - conserves energy according to user guide */
	TBCTL = 0;
	TBCCTL6 = 0;
}
#endif

/*---------------------------------------------------------------------------*/
/* msp430-ld may align _end incorrectly. Workaround in cpu_init. */
extern int _end;		/* Not in sys/unistd.h */
static char *cur_break = (char *)&_end;

//void __low_level_init(void);
volatile int dcoctl, bcsctl1, bcsctl2, bcsctl3;
void mcu_init(void) {
	WDTCTL = WDTPW+WDTHOLD;                   // Stop watchdog

	UserEnterCritical();

#if MCU == msp430f2617
	// Check if exist some calibration of DCO in the Flash
	if(CALBC1_16MHZ != 0xFF) {
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_16MHZ;                    /*Set DCO to 16MHz */
		DCOCTL = CALDCO_16MHZ;
	} else { /*start using reasonable values at 8 Mhz */
		// 8MHz
		DCOCTL = 0x00;
		BCSCTL1 = 0x8D;
		DCOCTL = 0x88;
	}
	BCSCTL2 = DIVS0;	// DCOCLK/2

	msp430_sync_dco();

#elif MCU == msp430f5437

	/* System clock generator 0. This bit may be used to enable/disable FLL */


	// Disable FLL
	__bis_SR_register(SCG0);

	UCSCTL0 = 0x0000;	   /* This register is configured automatically */
	UCSCTL1 = DCORSEL_4; /* Range selection from 1.3 (DCO=0 MOD=0) to 28.2 (DCO=31 MOD=0) MHz */

	UCSCTL2 = SYSTEM_CLOCK / 32768; /* Set the FFL divider 249 _10 ~ F9 _16  */

	UCSCTL4 = 0x0033;  /* MCLK e SMCLK from DCO clock source */ /* instead of 0x44 that is DCO/2 */
	UCSCTL5 = DIVS0; /* SMCLK = MCLK/2 */

	UCSCTL8 |= MCLKREQEN | SMCLKREQEN;

	// Enable FLL
	__bic_SR_register(SCG0);

	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 16 MHz / 32,768 Hz = 500000 = MCLK cycles for DCO to settle
	__delay_cycles(500000u);

	// Loop until XT1,XT2 & DCO fault flag is cleared
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
											  // Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

#else
#error "MCU not supported yet!"
#endif

	UserExitCritical();

#if 0
	dcoctl = DCOCTL;
	bcsctl1 = BCSCTL1;
	bcsctl2 = BCSCTL2;
	bcsctl3 = BCSCTL3;
#endif
	if((uintptr_t)cur_break & 1) { /* Workaround for msp430-ld bug! */
		cur_break++;
	}

}

/*
void __low_level_init(){

	WDTCTL = WDTPW+WDTHOLD;                   // Stop watchdog

}
*/

