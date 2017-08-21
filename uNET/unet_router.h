/*
 * \file unet_router.h
 *
 */

#ifndef UNET_ROUTER_H_
#define UNET_ROUTER_H_

#include "node.h"

#define NO_PARENT					((uint8_t)(-1))
#define MAX_PARENT_ALLOWED_FAILURES	(uint8_t)10
//#define RTABLE_UP_ENTRIES_MAX_NUM 	16
#define RTABLE_UP_ENTRIES_MAX_NUM 	100	// For cooja simulations
#define ROUTING_UP_TABLE_SIZE   	(uint8_t)RTABLE_UP_ENTRIES_MAX_NUM
#define NWK_TX_RETRIES          	(uint8_t)3
#define NWK_TX_RETRIES_UP       	(uint8_t)(NWK_TX_RETRIES)

#if UNET_DEVICE_TYPE == PAN_COORDINATOR
#define NODE_DISTANCE_INIT    0
#else
#define NODE_DISTANCE_INIT    0xFF
#endif

enum
{
	RESULT_PACKET_SEND_OK = 0,
	RESULT_DESTINATION_UNREACHABLE,
	RESULT_HOP_LIMIT_EXCEEDED,
	RESULT_TIMEOUT_IN_TRANSIT,
	RESULT_DESTINATION_NULL,
	RESULT_PACKET_SEND_ERROR,
	RESULT_UNKNOWN
};

typedef struct
{
    uint16_t      next_hop;                   // 16 bit address from intermediate neighbor
    addr64_t	  dest_addr64;				  // 64 bit address of the destination node
} unet_routing_table_up_t;

/** unet networking layer functions */
void     unet_header_set(packet_t * _pkt, uint8_t type, uint8_t hop_limit, uint8_t next_header, uint8_t *src, uint8_t *dst, uint8_t payload_len);
void     unet_header_print(packet_t *p);
uint8_t  unet_packet_input(packet_t *p);
uint8_t  unet_packet_output(packet_t *pkt, uint8_t tx_retries, uint16_t delay_retry);
uint8_t  unet_packet_up_sendto(addr64_t * dest_addr64, uint8_t payload_len);
uint8_t  unet_router_down(void);
void     unet_update_packet_down_dest(void);
uint8_t  unet_packet_down_send(uint8_t payload_len);
uint8_t  unet_router_adv(void);
void* 	 unet_rtable_up_get(void);
uint16_t unet_router_up_table_entry_find(addr64_t *dest64);
void     unet_router_up_table_entry_add(uint16_t next_hop, addr64_t *dest64);
uint16_t unet_router_up_table_entry_get(uint16_t idx);
void 	 unet_router_up_table_clear(void);
uint8_t  unet_router_up(void);

#endif /* UNET_ROUTER_H_ */
