/**
* \file HAL.c
* \brief BRTOS Hardware Abstraction Layer Functions.
*
* This file contain the functions that are processor dependant.
*
*
**/

/*********************************************************************************************************
*                                               BRTOS
*                                Brazilian Real-Time Operating System
*                            Acronymous of Basic Real-Time Operating System
*
*                              
*                                  Open Source RTOS under MIT License
*
*
*
*                                   OS HAL Functions to Coldfire V1
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*   Author:   Carlos Henrique Barriquello
*   Revision: 1.62
*   Date:     14/12/2010
*
*********************************************************************************************************/

//#include <isr_compat.h>
#include <signal.h>
#include "BRTOS.h"
#include "leds.h"
#include "HAL.h"

/// Define if its necessary to save status register / interrupt info
#if (SP_SIZE == 16)
INT16U SPvalue;                             ///< Used to save and restore a task stack pointer
#elif (SP_SIZE == 32)
INT32U SPvalue;                             ///< Used to save and restore a task stack pointer
#endif

/**
 * Timer configuration by MCU selection
 */
#if MCU == msp430f5437
#define TIMER_CTL	TA0CTL                      // Control
#define TIMER_CCTL	TA0CCTL1                    // Counter control
#define TIMER_CCR	TA0CCR1                     // Counter
#define TIMER_REG	TA0R                        // Register
#define TIMER_INT	TIMER0_A1_VECTOR            // Interrupt
#define TIMER_IV	TA0IV                       // Interrupt vector

#elif MCU == msp430f2617
#define TIMER_CTL	TACTL						// Control
#define TIMER_CCTL	TACCTL1						// Counter control
#define TIMER_CCR	TACCR1						// Counter
#define TIMER_REG	TAR							// Register
#define TIMER_INT	TIMERA1_VECTOR				// Interrupt
#define TIMER_IV	TAIV						// Interrupt vector

#elif MCU == msp430fr5969
#define TIMER_CTL	TA0CTL						// Control
#define TIMER_CCTL	TA0CCTL1					// Counter control
#define TIMER_CCR	TA0CCR1						// Counter
#define TIMER_REG	TA0R						// Register
#define TIMER_INT	TIMER0_A1_VECTOR			// Interrupt
#define TIMER_IV	TA0IV						// Interrupt vector
#else
#error "Timer not supported in this MCU!"
#endif


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Tick Timer Setup                         /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
volatile int debug_ticktimersetup;
void TickTimerSetup(void)
{
	// Select ACLK as source clock and clean other configurtions
	TIMER_CTL = TASSEL0 | TACLR;// | TACLR;

	/* Interrupt after X ms. */
	TIMER_CCR = SYSTEM_TICK;

	/* Start Timer_A in continuous mode. */
	TIMER_CTL |= MC1;

	TIMER_IV = 0; // Clear all interruptions

	/* Initialize ccr1 to create the X ms interval. */
	/* CCR1 interrupt enabled, interrupt occurs when timer equals CCR. */
	TIMER_CCTL = CCIE;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS RTC Setup                                /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void OSRTCSetup(void)
{  
	// not used
	__asm ("nop");

  //OSResetTime(&Hora);counter
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Timer A0 interrupt service routine
//volatile int debug_timer=0;


#define CLOCK_LT(a, b) ((int16_t)((a)-(b)) < 0)


/* last_tar is used for calculating clock_fine */
static volatile uint16_t last_tar = 0;
/*---------------------------------------------------------------------------*/
static inline uint16_t
read_tar(void)
{
  /* Same as clock_counter(), but can be inlined */
  uint16_t t1, t2;
  do {
    t1 = TIMER_REG;
    t2 = TIMER_REG;
  } while(t1 != t2);
  return t1;
}


/**
 * TimerA0 Interrupt
 *   Clock Source: ACLK (32768 Hz)
 *   Frequency: 1000 Hz (1 ms)
 */
#define interrupt(x) void __attribute__((interrupt (x)))
interrupt(TIMER_INT) TickTimer(void)
{
	// ************************
	// Interrupt entrance
	// ************************
	OS_INT_ENTER();

	if(TIMER_IV == 2) { //// Capture/Compare 1
		TIMER_IV = 0; // Clear all interruptions
		/* HW timer bug fix: Interrupt handler called before TR==CCR.
		 * Occurs when timer state is toggled between STOP and CONT. */
		while(TIMER_CTL & MC1 && TIMER_CCR - read_tar() == 1);

		last_tar = read_tar();
		/* Make sure interrupt time is future */
		while(!CLOCK_LT(last_tar, TIMER_CCR)) {
			TIMER_CCR += SYSTEM_TICK;
			OSIncCounter();

			OS_TICK_HANDLER();

			last_tar = read_tar();
		}
	} else TIMER_IV = 0;

	// ************************
	// Interrupt exit
	// ************************
	OS_INT_EXIT();
	// ************************
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////   Software Interrupt to provide Switch Context   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
 /***********************************************************
* \fn fake interrupt void SwitchContext(void)
* \brief Software interrupt handler routine (Internal kernel function).
*  Used to switch the tasks context.
****************************************************************/
void SwitchContext(void)
{
  // as MSP430 does not have sw interrupt, we save 7 regs to make it appear like one.
  OS_SAVE_ISR();

  // ************************
  // Entrada de interrupcao
  // ************************
  OS_INT_ENTER();

  // ************************
  // Interrupt Handling
  CriticalDecNesting();
  if (!iNesting)
  {
    SelectedTask = OSSchedule();
    if (currentTask != SelectedTask){
//    	PRINTF("Context: FROM=%d TO=%d\n",currentTask,SelectedTask); for(i=0;i<200;i++);
        OS_SAVE_CONTEXT2();
        OS_SAVE_SP();
        ContextTask[currentTask].StackPoint = SPvalue;
	    currentTask = SelectedTask;
        SPvalue = ContextTask[currentTask].StackPoint;
        OS_RESTORE_SP();
        OS_RESTORE_CONTEXT2();
    }
  }

  // ************************
  // Interrupt Exit
  // ************************
  OS_RESTORE_ISR();
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////  Task Installation Function                      /////
/////                                                  /////
/////  Parameters:                                     /////
/////  Function pointer, task priority and task name   /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if (TASK_WITH_PARAMETERS == 1)
	void CreateVirtualStack(void(*FctPtr)(void*), INT16U NUMBER_OF_STACKED_BYTES, void * parameters)
#else
	void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES)
#endif
{  
#if (TASK_WITH_PARAMETERS == 1)
   (void)parameters;
#endif

   // First SR should be 0
   OS_CPU_TYPE *stk_pt = (OS_CPU_TYPE*)&STACK[iStackAddress + (NUMBER_OF_STACKED_BYTES / sizeof(OS_CPU_TYPE))];

   // Add mask to check for stack overflow
   /*stk_pt += 4;
   *--stk_pt = 0xaaaaaaaa;
   *--stk_pt = 0xaaaaaaaa;
   *--stk_pt = 0xaaaaaaaa;
   *--stk_pt = 0xaaaaaaaa;*/

   // Pointer to Task Entry
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
   *--stk_pt = (0x0000ffff&((OS_CPU_TYPE)FctPtr)) << 16; // copy PC LSB
   *stk_pt |= ((0x000f0000&((OS_CPU_TYPE)FctPtr)) >> 4); // copy PC MSB
   *stk_pt |= (0x00000008); // copy SR (ITE)
	// Initialize registers
   *--stk_pt = 0x00000015;
   *--stk_pt = 0x00000014;
   *--stk_pt = 0x00000013;
   *--stk_pt = 0x00000012;
   *--stk_pt = 0x00000011;
   *--stk_pt = 0x00000010;
   *--stk_pt = 0x00000009;
   *--stk_pt = 0x00000008;
   *--stk_pt = 0x00000007;
   *--stk_pt = 0x00000006;
   *--stk_pt = 0x00000005;
   *--stk_pt = 0x00000004;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
