/*
 * trickle.h
 *
 *  Created on: 6 de mai de 2016
 *      Author: gustavo
 */

#ifndef UNET_TRICKLE_H_
#define UNET_TRICKLE_H_

#include "stdint.h"
#include "BRTOS.h"

typedef struct _trickle_t{
	ostick_t interval;
	ostick_t t;
	ostick_t init_interval;
	ostick_t complete_interval;
	ostick_t max_interval;
	uint8_t interval_scaling;
}trickle_t;

// Prototypes
void open_trickle(trickle_t *timer, ostick_t interval, ostick_t max_interval);
void run_trickle(trickle_t *timer);
void reset_trickle(trickle_t *timer);
uint8_t is_trickle_reseted(trickle_t *timer);


#endif /* UNET_TRICKLE_H_ */
