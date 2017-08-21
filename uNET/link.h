/*
 * \file link.h
 *
 */

#ifndef UNET_LINK_H_
#define UNET_LINK_H_

#include "stdint.h"
#include "BRTOSConfig.h"
#include "NetConfig.h"

#if 0
#define ATOMIC_ENTER()		UserEnterCritical();
#define ATOMIC_EXIT()		UserExitCritical();
#else
#define ATOMIC_ENTER()
#define ATOMIC_EXIT()
#endif

#define ATOMIC_SET(x,y)		do{ATOMIC_ENTER(); (x) = (y); ATOMIC_EXIT();}while(0);
#define ATOMIC_ADD(x,y)		do{ATOMIC_ENTER(); (x) = (x) + (y); ATOMIC_EXIT();}while(0);


#define ROUTE_TO_BASESTATION_LOST (uint8_t)0xFE
#define NO_ROUTE_TO_BASESTATION   (uint8_t)0xFF
#define UNET_NWK_HEADER_SIZE  	  (20)

#define NEIGHBOR_LINK_PACKET_PERIOD_MS   	   (uint16_t)(1000)
#define NEIGHBOURHOOD_TIMEOUT_MS 			   (uint32_t)(NEIGHBOR_LINK_PACKET_PERIOD_MS*MAX_PING_TIME*3)
#define RSSI_THRESHOLD          			   (uint8_t)3


typedef uint16_t                neighbor_table_t;

typedef union _NEIGHBOR_STATUS
{
	uint8_t Val;
    struct
    {
		uint8_t Symmetric             :1;
		uint8_t RxAllowed             :1;
		uint8_t TxPending             :1;
		uint8_t Active                :1;
		uint8_t RxChannel             :4;
    } bits;
} nb_status_t;

typedef struct _UNET_NEIGHBOURHOOD
{
	uint16_t        Addr_16b;                   // 16 bit address from neighbor
    uint8_t         NeighborRSSI;               // Average of the Neighbor signal quality
    nb_status_t     NeighborStatus;             // Neighbor status
    uint8_t         NeighborMyRSSI;             // Average of the my signal quality at this neighbor
    uint8_t         NeighborLQI;                // Neighbor link quality indicator
    uint8_t         NeighborLastID;             // Last message ID used for this neighbor
    uint8_t		    NeighborDistance;			  // Distance from in the root node
} unet_neighborhood_t;

#define UNET_HEADER_END (32)
enum
{
	/* unet extension headers start for types neighbor/router req/adv */
	UNET_CTRL_MSG_PACKET_START = UNET_HEADER_END,
	UNET_CTRL_MSG_PACKET_TYPE = UNET_CTRL_MSG_PACKET_START+0,
	UNET_CTRL_MSG_ROUTER_ADV_START = UNET_HEADER_END,
	UNET_CTRL_MSG_ADDR16 = UNET_CTRL_MSG_PACKET_START+1,
	UNET_CTRL_MSG_ADDR16H = UNET_CTRL_MSG_PACKET_START+1,
	UNET_CTRL_MSG_ADDR16L = UNET_CTRL_MSG_PACKET_START+2,
	UNET_CTRL_MSG_DISTANCE = UNET_CTRL_MSG_PACKET_START+3,
	UNET_CTRL_MSG_NB_COUNT = UNET_CTRL_MSG_PACKET_START+4,
	UNET_CTRL_MSG_NB_START = UNET_CTRL_MSG_PACKET_START+5
};

#define MULTICAST_MASK	8
#define ACK_REQ_MASK	4
#define UP_MASK			2
#define DOWN_MASK		1

/* packet type number */
typedef enum
{
	BROADCAST_LOCAL_LINK = 0,
	UNICAST_DOWN = DOWN_MASK + ACK_REQ_MASK,
	UNICAST_UP = UP_MASK + ACK_REQ_MASK,
	UNICAST_ACK_DOWN = DOWN_MASK,
	UNICAST_ACK_UP = UP_MASK,
	MULTICAST_UP = MULTICAST_MASK+UP_MASK+ACK_REQ_MASK,
	MULTICAST_ACK_UP = MULTICAST_MASK+UP_MASK
}unet_packet_type_t;

/* packet type number */
typedef enum
{
	NEIGHBOR_REQ = 0,
	NEIGHBOR_ADV = 1,
	ROUTER_REQ = 2,
	ROUTER_ADV = 3,
	NEIGHBOR_ADV_RESET = 128+1
}unet_ctrl_msg_type_t;

/* partially based on IANA protocol numbers */
/* http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml */
enum
{

	NO_NEXT_HEADER = 59,
	NEXT_HEADER_TCP = 6,
	NEXT_HEADER_UDP = 17,
	NEXT_HEADER_UNET_CTRL_MSG = 253, /* reserved for experimentation */
	NEXT_HEADER_UNET_APP = 254, /* reserved for experimentation */
	NET_HEADER_RESERVED = 255
};
typedef struct
{
	uint8_t addr16h;
	uint8_t addr16l;
	uint8_t rssi;
}link_neighbor_t;


/** link layer functions */
void       link_packet_create(void);
void       link_packet_create_ack(packet_t *p);
void       link_packet_process(packet_t *p);
packet_t * link_packet_get(void);
packet_t * link_packet_create_association_request(void);
void       link_neighbor_table_init(void);
uint8_t    link_neighbor_table_count(void);
uint8_t    link_neighbor_table_find(packet_t *p);
void 	   link_set_neighbor_activity(packet_t *p);
void 	   link_set_neighbor_seqnum(packet_t *p);
void 	   link_verify_neighbourhood(void);
uint16_t   link_neighbor_table_addr16_get(uint8_t idx);
uint8_t    link_packet_is_duplicated(uint8_t *src_addr, uint8_t seq_num);
void       link_seqnum_reset(uint16_t src_addr);
uint8_t    link_is_symmetric_parent(void);
void       link_check_parent_update(void);
uint16_t   link_get_parent_addr16(void);

#endif /* UNET_LINK_H_ */
