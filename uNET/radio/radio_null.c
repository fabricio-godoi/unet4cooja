/*
 * \file radio_null.c
 *
 */

#include "string.h"
#include "radio.h"
#include "radio_null.h"

static radio_params_t _radio_params;

static uint32_t (*_isr_handler)(void);

static uint8_t radio_buf_tx[128];
static uint8_t radio_buf_rx[128];
static uint8_t radio_buf_rx_cnt;
static uint8_t radio_buf_tx_cnt;

uint32_t radio_null_isr(void);

#if DEBUG_PRINTF && WINNT
//#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__); fflush(stdout);
#elif DEBUG_PRINTF
//#include <stdio.h>
//#define PRINTF(...) printf(__VA_ARGS__); fflush(stdout);
#else
#define PRINTF(...)
#endif

#if WINNT
#define SIMULATE_TX_ISR 1
#endif
/*--------------------------------------------------------------------------------------------*/
static void radio_packet_print(uint8_t *pkt, uint8_t len)
{
	while(len > 0)
	{
		len--;
		PRINTF("%02X ", *pkt++);
	}
	PRINTF("\r\n");

}
/*--------------------------------------------------------------------------------------------*/
static int _init(const void *isr_handler)
{
	_isr_handler = isr_handler;
	_radio_params.param[RADIO_STATE] = INIT;
	BUF_CLEAN(radio_buf_tx);
	BUF_CLEAN(radio_buf_rx);
	radio_buf_rx_cnt = 0;
	radio_buf_tx_cnt = 0;
#if WINNT
	SetInterruptHandler(RADIO_NULL_ISR_NUMBER, radio_null_isr);
#endif
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
static int _recv (const void *buf, uint16_t *len)
{
	memcpy((void*)buf,(const void *)radio_buf_rx,radio_buf_rx_cnt);
	*len = radio_buf_rx_cnt;
	PRINTF("Packet rcv:\n");
	radio_packet_print(radio_buf_rx,radio_buf_rx_cnt);
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
static int _send (const void *buf, uint16_t len)
{
	if(len > sizeof(radio_buf_tx)) len = sizeof(radio_buf_tx);
	memcpy(radio_buf_tx, buf, len);
	radio_buf_tx_cnt = len;
	PRINTF("Packet snt:\n");
	radio_packet_print(radio_buf_tx,radio_buf_tx_cnt);
	radio_tx_acked(TRUE);

#if SIMULATE_TX_ISR
	GenerateSimulatedInterrupt(RADIO_NULL_ISR_NUMBER);
#else
	radio_null_isr();
#endif

	return 0;
}
/*--------------------------------------------------------------------------------------------*/
static int _set(radio_opt_t opt, uint8_t value)
{

	_radio_params.param[opt] = value;
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
static int _get(radio_opt_t opt, uint8_t *value)
{
	*value =  _radio_params.param[opt];
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
static int _isr(const void *isr_handler)
{
	_isr_handler = isr_handler;
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
const radio_driver_t radio_null =
{
	.init = _init,
	.send = _send,
	.recv = _recv,
	.set = _set,
	.get = _get,
	.isr = _isr
};
/*--------------------------------------------------------------------------------------------*/
uint32_t radio_null_isr(void)
{
	uint32_t res = 0;

#if !WINNT
	OS_SR_SAVE_VAR;
#endif
	OSEnterCritical();
	if(_isr_handler != NULL)
	{
		_isr_handler();
		res = TRUE;
	}else
	{
		res = FALSE;
	}
	OSExitCritical();
	return res;

}
/*--------------------------------------------------------------------------------------------*/
void simulate_radio_rx_event (const void *buf, uint16_t len)
{
	if(len > sizeof(radio_buf_rx)) len = sizeof(radio_buf_rx);
	memcpy((void*)radio_buf_rx,buf,len);
	radio_buf_rx_cnt = len;
	radio_packet_print(radio_buf_rx,radio_buf_rx_cnt);

#if SIMULATE_RX_ISR
	radio_rxing(TRUE);
	GenerateSimulatedInterrupt(RADIO_NULL_ISR_NUMBER);
#endif
	return;
}
/*--------------------------------------------------------------------------------------------*/
void read_radio_tx_buffer (const void *buf, uint16_t *len)
{

	memcpy((void*)buf,(const void *)radio_buf_tx,radio_buf_tx_cnt);
	*len = radio_buf_tx_cnt;
	radio_packet_print(radio_buf_tx,radio_buf_tx_cnt);
	return;
}


