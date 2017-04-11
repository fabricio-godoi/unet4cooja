/**
* \file hardware.h
* \brief Processor macros, defines and registers declaration.
* \date 10/04/2017
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
*   Author: Gustavo Weber Denardin, Fabricio Negrisolo de Godoi
*   Revision: 1.0        , 1.1
*   Date:     20/03/2009 , 10/04/2017
*
*********************************************************************************************************/

#ifndef HARDWARE_H
#define HARDWARE_H


#include <msp430.h>

/**
 * MSP430 supported MCU
 */
#define msp430f1611  1
#define msp430f2617  2
#define msp430f5437  3
#define msp430g2553  4
#define msp430fr5969 5


#define __UNKOWN__  0
#define __SKY__     1
#define __Z1__      2
#define __WISMOTE__ 3

/**
 * Select specific microprocessor from options above
 * WARNING: Don't forget to change Makefile MCU and PLATFORM options
 * msp430f1611  - Sky
 * msp430f2617  - Zolertia Z1
 * msp430f5437  - Wismote
 * msp430g2553  - LaunchPad
 * msp430fr5969 - LaunchPad
 */
#define MCU msp430f5437


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
#elif (MCU == msp430fr5969)
#include "msp430fr5969.h"
#define PLATFORM __UNKOWN__
#else
#define PLATFORM __UNKOWN__
#endif


#define BRTOS_PLATFORM 	PLATFORM

#define WDTCTL_INIT     WDTPW|WDTHOLD
#endif
