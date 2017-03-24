/*
 * \file radio_null.h
 *
 */

#ifndef UNET_RADIO_RADIO_NULL_H_
#define UNET_RADIO_RADIO_NULL_H_

#include "stdint.h"
#define RADIO_NULL_ISR_NUMBER   2

extern const struct radio_driver radio_null;

void simulate_radio_rx_event (const void *buf, uint16_t len);
void read_radio_tx_buffer (const void *buf, uint16_t *len);

#endif /* UNET_RADIO_RADIO_NULL_H_ */
