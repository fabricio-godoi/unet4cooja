/*
 * \file leds.h
 * \brief  Enable disable output to control LEDs
 * \author Fabricio Negrisolo de Godoi
 * \date   10/04/2017
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
//#define LEDS_YELLOW_DIR	P1DIR
#define LEDS_RED_OUT	P1OUT
#define LEDS_GREEN_OUT	P1OUT
//#define LEDS_YELLOW_OUT	P1OUT
#define LEDS_RED    BIT0
#define LEDS_GREEN  BIT6
//#define LEDS_YELLOW (LEDS_RED|LEDS_GREEN)

#elif (MCU == msp430f5437)
#define LEDS_RED_DIR	P8DIR
#define LEDS_GREEN_DIR	P2DIR
#define LEDS_YELLOW_DIR	P5DIR
#define LEDS_RED_OUT	P8OUT
#define LEDS_GREEN_OUT	P2OUT
#define LEDS_YELLOW_OUT	P5OUT
#define LEDS_CONF_DIR  P1DIR
#define LEDS_CONF_OUT  P1OUT
#define LEDS_RED    BIT6
#define LEDS_GREEN  BIT4
#define LEDS_YELLOW BIT2

#elif MCU == msp430fr5969
#define LEDS_RED_DIR	P4DIR
#define LEDS_GREEN_DIR	P1DIR
//#define LEDS_YELLOW_DIR
#define LEDS_RED_OUT	P4OUT
#define LEDS_GREEN_OUT	P2OUT
//#define LEDS_YELLOW_OUT
#define LEDS_RED    BIT6
#define LEDS_GREEN  BIT0
//#define LEDS_YELLOW

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
void leds_blink(void);
unsigned char leds_get(void);
void leds_set(unsigned char ledv);
void leds_on(unsigned char ledv);
void leds_off(unsigned char ledv);
void leds_toggle(unsigned char ledv);


#endif /* INCLUDES_LEDS_H_ */
