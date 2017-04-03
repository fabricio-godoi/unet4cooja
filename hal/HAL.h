/**
* \file HAL.h
* \brief BRTOS Hardware Abstraction Layer defines and inline assembly
*
* This file contain the defines and inline assembly that are processor dependant.
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
*                                     OS HAL Header to Coldfire V1
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.2
*   Date:     01/10/2010
*
*********************************************************************************************************/

#ifndef OS_HAL_H
#define OS_HAL_H

#include "OS_types.h"
#include "hardware.h"
#include "BRTOSConfig.h"

// Supported processors
#define COLDFIRE_V1		1
#define HCS08			2
#define MSP430			3
#define ATMEGA			4
#define PIC18			5

/// Define the used processor
#define PROCESSOR 		MSP430

/// Define the CPU type
#define OS_CPU_TYPE 	INT32U

/// Define if nesting interrupt is active
#define NESTING_INT 0

/// Define if its necessary to save status register / interrupt info
#if NESTING_INT == 1
  #define OS_SR_SAVE_VAR INT16U CPU_SR = 0;
#else
  #define OS_SR_SAVE_VAR
#endif

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

extern INT8U iNesting;

/// Define CPU Stack Pointer
#if (SP_SIZE == 16)
extern INT16U SPvalue;                             ///< Used to save and restore a task stack pointer
#elif (SP_SIZE == 32)
extern INT32U SPvalue;
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Port Defines                                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/// Defines the change context command of the chosen processor
#define ChangeContext() SwitchContext()


/// Defines the disable interrupts command of the chosen microcontroller
#define UserEnterCritical() __dint()
// old: asm("	NOP	"); asm("	DINT	")
// Changed because of errors, check errata CPU39 for more information
// This way, nop instructions are put in right positions

/// Defines the enable interrupts command of the choosen microcontroller
#define UserExitCritical()  __eint()

#if (NESTING_INT == 0)
/// Defines the disable interrupts command of the chosen microcontroller
#define OSEnterCritical() UserEnterCritical()
/// Defines the enable interrupts command of the chosen microcontroller
#define OSExitCritical() UserExitCritical()
#endif

/// Defines the low power command of the chosen microcontroller
#define OS_Wait __bis_SR_register(CPUOFF + GIE)


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Functions Prototypes                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (TASK_WITH_PARAMETERS == 1)
void CreateVirtualStack(void(*FctPtr)(void *), INT16U NUMBER_OF_STACKED_BYTES, void *parameters);
#else
void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
#endif

/*****************************************************************************************//**
* \fn void TickTimerSetup(void)
* \brief Tick timer clock setup
* \return NONE
*********************************************************************************************/
void TickTimerSetup(void);

/*****************************************************************************************//**
* \fn void OSRTCSetup(void)
* \brief Real time clock setup
* \return NONE
*********************************************************************************************/
void OSRTCSetup(void);

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// fix MSP430 sw isr
/*****************************************************************************************//**
* \fn void SwitchContext(void)
* \brief Switch context function (mimic SW ISR in MSP430)
* \return NONE
*********************************************************************************************/
void SwitchContext(void);



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/////                                                     /////
/////          MSP430 Without Nesting Defines             /////
/////                                                     /////
/////                                                     /////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Context Define                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#define SaveContext() 			asm("	PUSHX.A	R10 \n\t"\
                                    "	PUSHX.A	R9 \n\t" \
                                    "	PUSHX.A	R8 \n\t" \
                                    "	PUSHX.A	R7 \n\t" \
                                    "	PUSHX.A	R6 \n\t" \
                                    "	PUSHX.A	R5 \n\t" \
                                    "	PUSHX.A	R4 \n\t" )


#define OS_SAVE_CONTEXT2() SaveContext()

#define OS_SAVE_CONTEXT()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Stack Pointer Define                   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// save top of stack
#define SaveCurrentSP() asm("	MOVX.A	R1,&SPvalue \n\t")

#define OS_SAVE_SP() SaveCurrentSP()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/*****************************************************************************************//**
* \fn asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
#define RestoreSP()		asm("	MOVX.A	&SPvalue,R4 \n\t"); asm("	MOVX.A	R4,R1 \n\t")

/// Restore Stack Pointer Define
#define OS_RESTORE_SP() RestoreSP()
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Restore Context Define                      /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// restore top of stack
// restore other CPU registers
// adjust stack pointer value
/*
									*/
#define RestoreContext() 		asm("	POPX.A	R4 \n\t" \
                                    "	POPX.A	R5 \n\t" \
                                    "	POPX.A	R6 \n\t" \
                                    "	POPX.A	R7 \n\t" \
                                    "	POPX.A	R8 \n\t" \
                                    "	POPX.A	R9 \n\t" \
                                    "	POPX.A	R10 \n\t")

#define OS_RESTORE_CONTEXT2() RestoreContext()

#define OS_RESTORE_CONTEXT()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


/*****************************************************************************************//**
* \fn OS_SAVE_ISR() and OS_RESTORE_ISR()
* \brief Used to mimic ISR call
* \return NONE
*********************************************************************************************/
/**
 * Since MSP430 doesn't have SW interrupt, it's emulated by an function.
 *
 * The function call (CALLA) save the PC as MSW (more significant word) first, then LSW
 * it's needed a SWAP it, and shift the PC.MSW to the left.
 *
 * Also, the CALLA save 4 bytes of R10 register that it'll not be used.
 * So, it's used that space to swap the information between PC.LSW and PC.MSW.
 *
 * TO shift the PC.MSW, the R15 register is saved in the stack,
 * then used to do the operations of shift and added the SR.
 *
 * Then the information is put back in the stack, and put the SP at the TOS.
 * Lastly, save the context from R14 to R11.
 *
 * MSW - Most  Significant Word  LSW - Least Significant Word
 * MSB - Most  Significant Byte  LSB - Least Significant Byte
 *
 *           SP+0        SP+2    SP+4    SP+6
 * [  ... 0x00000000    ......  PC.MSW  PC.LSW  ...  ]    Original frame
 *           SP+0        SP+2    SP+4    SP+6
 * [  ... 0x00000000    PC.LSW  PC.MSW  PC.LSW  ...  ]    Move PC.LSW
 *           SP+0        SP+2    SP+4    SP+6
 * [  ... 0x00000000    PC.LSW  PC.MSW  PC.MSW  ...  ]    Move PC.MSW
 *           SP+0        SP+2    SP+4    SP+6
 * [  ... 0x00000000    PC.LSW  PC.LSW  PC.MSW  ...  ]    Move PC.LSW
 *           SP-4        SP-2    SP+0    SP+2
 * [  ... 0x00000000    PC.LSW  PC.LSW  PC.MSW  ...  ]    Increment SP+=4
 *           SP-2        SP+0    SP+2    SP+4
 * [  ... 0x00000000      R15   PC.LSW  PC.MSW  ...  ]    Save R15
 *           SP-6        SP-4    SP-2    SP+0
 * [  ... 0x00000000      R15   PC.LSW  PC.MSW  ...  ]    Move SP
 *
 * R15 [ 0b0000  PC.MSW.MSB  PC.MSW.LSB         ]    Push PC.MSW from stack to R15
 * R15 [ 0b0000  PC.MSW.LSB  PC.MSW.MSB         ]    Swap PC.MSW bytes
 * R15 [ 0b0000  PC.N[15:12]|PC.MSW.MSB  0b0000 ]    Shift 4 bits to left from 0 to 15
 * R15 [ 0b0000  PC.N[15:12]|SR[11:0]           ]    Move SR with PC.N - MSW LSB Less Significant Nibble
 *
 *          SP-6       SP-4    SP-2     SP+0
 * [  ... 00000000      R15   PC.LSW  PC.N|SR  ...  ]    Pop R15 to stack
 *          SP-2       SP+0    SP+2     SP+4
 * [  ... 00000000      R15   PC.LSW  PC.N|SR  ...  ]    Set SP to top of stack
 *        SP0  SP2  SP4  SP6  SP8   SP10     SP12
 * [  ... R11  R12  R13  R14  R15  PC.LSW  PC.N|SR  ...  ]    Save R14-R11
 *
 *  Obs.: asm(	SUBX.A  #(n),Rx ) isn't properly working on Cooja!
 */
#define OS_SAVE_ISR()            asm("	MOV.W	6(SP),2(SP)	\n\t"	\
								     "	MOV.W	4(SP),6(SP)	\n\t"	\
								     "	MOV.W	2(SP),4(SP)	\n\t"	\
								     "	ADDX.A	#4,SP	\n\t"		\
								     "	PUSHX.A	R15 \n\t"			\
								     "	ADDX.A	#4,SP	\n\t"		\
								     "	POPX.W	R15 \n\t"			\
								     "	SWPB	R15 \n\t"			\
								     "	RLAM.W	#4,R15	\n\t"		\
								     "	ADD.W	SR,R15	\n\t"		\
								     "	PUSHX.W	R15	\n\t"			\
								     "	ADDX.A	#(-4),SP	\n\t"	\
                                     "	PUSHX.A	R14 \n\t"			\
                                     "	PUSHX.A	R13 \n\t"			\
                                     "	PUSHX.A	R12 \n\t"			\
                                     "	PUSHX.A	R11 \n\t")

/**
 * Since DelayTask disable interrupts, it's needed to enable interrupts at end
 */
#define OS_RESTORE_ISR() 		 asm("	POPX.A	R11 \n\t"	\
                                     "	POPX.A	R12 \n\t"	\
                                     "	POPX.A	R13 \n\t"	\
                                     "	POPX.A	R14 \n\t"	\
                                     "	POPX.A	R15 \n\t");	\
								 asm("	reti	")
									 
/*********************************************************************************************/									 
#define CriticalDecNesting() iNesting--      	


#define BTOSStartFirstTask() 		\
			OS_RESTORE_SP();		\
			OS_RESTORE_CONTEXT2(); 	\
			OS_RESTORE_ISR()
 
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/* Info:
 * 		If using MSPSim, the system clock (DCO) cannot be above 5MHz.
 * 		There is a workaround to be able to use above that, see link bellow.
 * Link: https://github.com/nfi/mspsim/commit/8892547ff262cdcf4f22e5a435a73da5ad5b9fd2
 *
 * MSP Simple Configuration
 * SYSTEM CLOCK 			16000000uL
 * SYSTEM_SECONDARY_CLOCK 	SYSTEM_CLOCK/8
 * CRYSTAL_CLOCK			32768uL
 * SYSTEM_TICK				CRYSTAL_CLOCK/1000 // 1ms
 *
*/
#define SYSTEM_CLOCK			configCPU_CLOCK_HZ 					// 16 MHz - SMCLK = MCLK/2
#define CRYSTAL_CLOCK			configCPU_ACLK_HZ					// 32.768 Hz
#define SYSTEM_TICK				CRYSTAL_CLOCK/configTICK_RATE_HZ	// 1ms

// Check variable boundary, the register only supports 16bits
#if (SYSTEM_TICK > 65535)
#error "SYSTEM_TICK must be at most 65535"
#endif

#endif
