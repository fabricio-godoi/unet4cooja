/*
 * leds.h
 *
 *  Created on: Nov 7, 2016
 *      Author: user
 */

#ifndef INCLUDES_LEDS_H_
#define INCLUDES_LEDS_H_

#include "hardware.h"

#define ENABLE_LEDS

#if (MCU == msp430f2617)
#define LEDS_RED_DIR	P5DIR
#define LEDS_GREEN_DIR	P5DIR
#define LEDS_YELLOW_DIR	P5DIR
#define LEDS_RED_OUT	P5OUT
#define LEDS_GREEN_OUT	P5OUT
#define LEDS_YELLOW_OUT	P5OUT
#define LEDS_RED    0x10
#define LEDS_GREEN  0x40
#define LEDS_YELLOW 0x20

#elif (MCU == msp430g2553)
#define LEDS_RED_DIR	P1DIR
#define LEDS_GREEN_DIR	P1DIR
#define LEDS_YELLOW_DIR	P1DIR
#define LEDS_RED_OUT	P1OUT
#define LEDS_GREEN_OUT	P1OUT
#define LEDS_YELLOW_OUT	P1OUT
#define LEDS_RED    BIT0
#define LEDS_GREEN  BIT6
#define LEDS_YELLOW (LEDS_RED|LEDS_GREEN)

#elif (MCU == msp430f5437)

#define LEDS_RED_DIR	P8DIR
#define LEDS_GREEN_DIR	P2DIR
#define LEDS_YELLOW_DIR	P5DIR
#define LEDS_RED_OUT	P8OUT
#define LEDS_GREEN_OUT	P2OUT
#define LEDS_YELLOW_OUT	P5OUT

#define LEDS_RED    BIT6
#define LEDS_GREEN  BIT4
#define LEDS_YELLOW BIT2

#else
//#error "leds: Unrecognized processor."
#undef ENABLE_LEDS
#define LEDS_RED	0
#define LEDS_GREEN  0
#define LEDS_YELLOW	0

#endif

// Leds yellow is how the Cooja software was described,
// but the color showed in the simulator is blue
#define LEDS_BLUE LEDS_YELLOW

// LEDs by program UI
#define LEDS_FIRST  LEDS_YELLOW
#define LEDS_SECOND LEDS_GREEN
#define LEDS_THIRD  LEDS_RED


void leds_init(void);
unsigned char leds_get(void);
void leds_set(unsigned char ledv);
void leds_on(unsigned char ledv);
void leds_off(unsigned char ledv);
void leds_toggle(unsigned char ledv);


#endif /* INCLUDES_LEDS_H_ */
