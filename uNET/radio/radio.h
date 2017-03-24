/*
 * \file radio.h
 *
 */

#ifndef UNET_RADIO_RADIO_H_
#define UNET_RADIO_RADIO_H_

#include "NetConfig.h"
#include "BoardConfig.h"


extern BRTOS_Mutex  *Radio_IEEE802;
extern BRTOS_Sem* 	Radio_RX_Event;
extern BRTOS_Sem* 	Radio_TX_Event;

#define UNET_RADIO_RESOURCE   Radio_IEEE802

#define init_resourceRadio(priority)  if(OSMutexCreate(&(UNET_RADIO_RESOURCE),priority) != ALLOC_EVENT_OK){while(1);}
#define acquireRadio()   OSMutexAcquire(UNET_RADIO_RESOURCE,0)
#define releaseRadio()	 OSMutexRelease(UNET_RADIO_RESOURCE)

#define RADIO_ATOMIC_START()  OS_SR_SAVE_VAR; OSEnterCritical();
#define RADIO_ATOMIC_END()    OSExitCritical();

#define BUF_CLEAN(x)		  memset((x),0x00,sizeof(x));

enum eRADIO_FLAGS
{
	INIT = 1,
	AUTOACK = 2,
	TX_OK = 4,
	TX_ACK = 8,
	TX_ACKED = 16,
	RX_OK = 32,
	RX_DISABLED = 64,
	LOOPBACK = 128
};

enum eRADIO_STATUS
{
	eIDLE = 0,
	eRX = 1,
	eTX = 2,
	eSLEEP = 3,
	eUNKNOWN = 4
};

#define RADIO_TX_ERR_NONE            0
#define RADIO_TX_ERR_NOTSPECIFIED    1
#define RADIO_TX_ERR_COLLISION       2
#define RADIO_TX_ERR_MAXRETRY        3
#define RADIO_TX_WAIT                4


typedef enum
{
	RADIO_STATE = 0,
	CHANNEL = 1,
	MACADDR16 = 2,
	MACADDR16H = 2,
	MACADDR16L = 3,
	PANID = 4,
	PANID16H = 4,
	PANID16L = 5,
	MACADDR64 = 6,
	MACADDR64_7 = 6,
	MACADDR64_6 = 7,
	MACADDR64_5 = 8,
	MACADDR64_4 = 9,
	MACADDR64_3 = 10,
	MACADDR64_2 = 11,
	MACADDR64_1 = 12,
	MACADDR64_0 = 13,
	TXPOWER = 14,
	RSSI = 15,
	LQI = 16,
	TX_STATUS = 17,
	TX_RETRIES = 18,
	RADIO_STATUS = 19,
	CRC = 20,
	MAX_OPT
}radio_opt_t;

typedef union
{
	struct radio_params
	{
		uint8_t radio_state;
		uint8_t channel;
		uint8_t macaddr16h;
		uint8_t macaddr16l;
		uint8_t panidh;
		uint8_t panidl;
		uint8_t macaddr64_7;
		uint8_t macaddr64_6;
		uint8_t macaddr64_5;
		uint8_t macaddr64_4;
		uint8_t macaddr64_3;
		uint8_t macaddr64_2;
		uint8_t macaddr64_1;
		uint8_t macaddr64_0;
		uint8_t txpower;
		uint8_t last_rssi;
		uint8_t last_lqi;
		uint8_t last_tx_status;
		uint8_t last_retry_count;
		uint8_t radio_status;
		uint8_t crc;
	}st_radio_param;
	uint8_t param[MAX_OPT];
}radio_params_t;

/* helper functions */
#define is_radio_rxing(s)	 		(((s) & RX_OK) == RX_OK)
#define is_radio_rx_autoack(s)	 	(((s) & AUTOACK) == AUTOACK)
#define is_radio_txing(s)	 		(((s) & TX_OK) == TX_OK)
#define is_radio_tx_ack(s)	 		(((s) & TX_ACK) == TX_ACK)
#define is_radio_tx_acked(s)	 	(((s) & TX_ACKED) == TX_ACKED)
#define is_radio_rx_disabled(s)	 	(((s) & RX_DISABLED) == RX_DISABLED)

#define RADIO_STATE_SET(s)  \
do{ 						\
	uint8_t status = 0;		\
	UNET_RADIO.get(RADIO_STATE,	&status); 	\
	status |= (s);							\
	UNET_RADIO.set(RADIO_STATE,status);		\
}while(0);

#define RADIO_STATE_RESET(s)  	\
do{ 							\
	uint8_t status = 0;			\
	UNET_RADIO.get(RADIO_STATE,	&status); 	\
	status &= ~(s);							\
	UNET_RADIO.set(RADIO_STATE,status);		\
}while(0);

#define radio_rxing(s)	 	 do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(RX_OK);}else{RADIO_STATE_RESET(RX_OK);} RADIO_ATOMIC_END();} while(0);
#define radio_rx_autoack(s)	 do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(AUTOACK);}else{RADIO_STATE_RESET(AUTOACK);} RADIO_ATOMIC_END();} while(0)
#define radio_txing(s)	 	 do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(TX_OK);}else{RADIO_STATE_RESET(TX_OK);} RADIO_ATOMIC_END();} while(0)
#define radio_tx_ack(s)	 	 do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(TX_ACK);}else{RADIO_STATE_RESET(TX_ACK);} RADIO_ATOMIC_END();} while(0)
#define radio_tx_acked(s)	 do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(TX_ACKED);}else{RADIO_STATE_RESET(TX_ACKED);} RADIO_ATOMIC_END();} while(0)
#define radio_rx_disabled(s) do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(RX_DISABLED);}else{RADIO_STATE_RESET(RX_DISABLED);} RADIO_ATOMIC_END();} while(0)
#define radio_loopback(s)	 do { RADIO_ATOMIC_START(); if(s){RADIO_STATE_SET(LOOPBACK);}else{RADIO_STATE_RESET(LOOPBACK);} RADIO_ATOMIC_END();} while(0)


/**
 * The structure of a device driver for a radio.
 */
typedef struct radio_driver {

  /*  setup radio registers and isr handler */
  int (* init)(const void *isr_handler);

  /*  read radio buffer */
  int (* recv)(const void *buf, uint16_t *len);

  /*  write radio buffer and transmit */
  int (* send)(const void *buf, uint16_t len);

  /*  set a radio parameter */
  int (* set)(radio_opt_t opt, uint8_t value);

  /*  get a radio parameter */
  int (* get)(radio_opt_t opt, uint8_t *value);

  /*  set radio isr handler */
  int (* isr)(const void *isr_handler);

} radio_driver_t;

extern const radio_driver_t UNET_RADIO;



#endif /* UNET_RADIO_RADIO_H_ */
