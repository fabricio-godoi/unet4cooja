/**
* \file hardware.h
* \brief Processor macros, defines and registers declaration.
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
*                                         Hardware Header Files
*
*
*   Author: Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*********************************************************************************************************/

#ifndef HARDWARE_H
#define HARDWARE_H


#include <msp430.h>

/**
 * MSP430 supported ucontrollers
 */
#define msp430f1611	1
#define msp430f2617	2
#define msp430f5437	3
#define msp430g2553 4


/**
 * Select specific uprocessor from options above
 * WARNING: Don't forget to change Makefile MCU and PLATFORM options
 * msp430f1611 - Sky
 * msp430f2617 - Zolertia Z1
 * msp430f5437 - Wismote
 * msp430g2553 - LaunchPad
 */
#define MCU msp430f5437

#define __WISMOTE__ MCU
#define __SKY__     MCU
#define __Z1__      MCU

/**
 * Automatic select MCU option
 */
#if (MCU == msp430f1611)
#include "msp430f1611.h" // Sky
//#define __SKY__
#define PLATFORM __SKY__
#elif (MCU == msp430f2617)
#include "msp430f2617.h" // Z1
//#define __Z1__
#define PLATFORM __Z1__
#elif (MCU == msp430f5437)
#include "msp430f5437.h" // Wismote
//#define __WISMOTE__
#define PLATFORM __WISMOTE__
#elif (MCU == msp430g2553)
#include "msp430g2553.h"
#else
#define PLATFORM
#endif


#define BRTOS_PLATFORM 	PLATFORM

#define WDTCTL_INIT     WDTPW|WDTHOLD
#endif
