/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */


#include "leds.h"

#ifdef ENABLE_LEDS

/*---------------------------------------------------------------------------*/
void
leds_init(void)
{
	// Turn LEDs off (according with Cooja)
	LEDS_RED_OUT  ^= LEDS_RED; LEDS_GREEN_OUT  ^= LEDS_GREEN; LEDS_YELLOW_OUT  ^= LEDS_YELLOW;
	// Configure pins to output
	LEDS_RED_DIR  ^= LEDS_RED; LEDS_GREEN_DIR  ^= LEDS_GREEN; LEDS_YELLOW_DIR  ^= LEDS_YELLOW;
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_get(void) {
	return ( (LEDS_RED_OUT&LEDS_RED) | (LEDS_GREEN_OUT&LEDS_GREEN) | (LEDS_YELLOW_OUT&LEDS_YELLOW));
}
/*---------------------------------------------------------------------------*/
void
leds_set(unsigned char ledv)
{
	switch(ledv){
		case LEDS_RED:
			LEDS_RED_OUT &= ~ledv;
			break;
		case LEDS_GREEN:
			LEDS_GREEN_OUT &= ~ledv;
			break;
		case LEDS_YELLOW:
			LEDS_YELLOW_OUT &= ~ledv;
			break;
		default:
			break;
	}
}
/*---------------------------------------------------------------------------*/
void
leds_on(unsigned char ledv)
{
	switch(ledv){
		case LEDS_RED:
			LEDS_RED_OUT &= ~ledv;
			break;
		case LEDS_GREEN:
			LEDS_GREEN_OUT &= ~ledv;
			break;
		case LEDS_YELLOW:
			LEDS_YELLOW_OUT &= ~ledv;
			break;
		default:
			break;
	}
}
/*---------------------------------------------------------------------------*/
void
leds_off(unsigned char ledv)
{
	switch(ledv){
		case LEDS_RED:
			LEDS_RED_OUT |= ledv;
			break;
		case LEDS_GREEN:
			LEDS_GREEN_OUT |= ledv;
			break;
		case LEDS_YELLOW:
			LEDS_YELLOW_OUT |= ledv;
			break;
		default:
			break;
	}
}
/*---------------------------------------------------------------------------*/
void
leds_toggle(unsigned char ledv)
{
	switch(ledv){
		case LEDS_RED:
			LEDS_RED_OUT ^= ledv;
			break;
		case LEDS_GREEN:
			LEDS_GREEN_OUT ^= ledv;
			break;
		case LEDS_YELLOW:
			LEDS_YELLOW_OUT ^= ledv;
			break;
		default:
			break;
	}
}
/*---------------------------------------------------------------------------*/


#else
// Diable all functions
void leds_init(void){}
void leds_blink(void){}
unsigned char leds_get(void){}
void leds_set(unsigned char ledv){}
void leds_on(unsigned char ledv){}
void leds_off(unsigned char ledv){}
void leds_toggle(unsigned char ledv){}
#endif // ENABLE_LEDS
