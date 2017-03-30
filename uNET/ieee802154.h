/*
 * \file ieee802154.h
 *
 */

#ifndef UNET_IEEE802154_H_
#define UNET_IEEE802154_H_

#include "stdint.h"
#include "packet.h"

/* macros */
#define H(x)				 		(uint8_t)(((x))>>8)
#define L(x)				 		(uint8_t)(((x)))
#define BROAD_ADDR  		 		(0xFFFF)
#define UNUSED_ADDR  		 		(0xFFFE)
#define UNET_MAC_HEADER_SIZE  	    (9)
#define UNET_LLC_HEADER_SIZE  	    (1)
#define UNET_MAC_FOOTER_SIZE  	    (2)
#define UNET_LLC_PROTO_NUMBER   	(0x01)

typedef union
{
	uint8_t   u8[8];
}addr64_t;

typedef union
{
	uint8_t   u8[2];
}addr16_t;

/* ieee 802.15.4 mac frame control format */
typedef union
{
    //uint16_t 	u16;
    uint8_t     u8[2];
    struct /* bitfields are not portable. Use bit mask instead! */
    {
        // 0 to 7
    	uint8_t   FrameType       :3;
        uint8_t   SecurityEnabled :1;
        uint8_t   FramePending    :1;
        uint8_t   ACKRequest      :1;
        uint8_t   IntraPAN        :1;
        uint8_t          		  :1;
    	// 8 to 15
        uint8_t          		  :1;
        uint8_t          		  :1;
        uint8_t   DstAddrMode     :2;
        uint8_t   FrameVersion    :2;
        uint8_t   SrcAddrMode     :2;
    } bits;
} ieee802154_frame_control_t;

enum
{
	MASK_FRAME_TYPE = 0x07,
	MASK_SEC_ENABLED = 0x08,
	MASK_FRAME_PENDING = 0x10,
	MASK_ACK_REQ = 0x20,
	MASK_INTRA_PAN = 0x40,
	MASK_DEST_ADDR_MODE = 0xC0,
	MASK_FRAME_VERSION = 0x30,
	MASK_SRC_ADDR_MODE = 0x0C
};

#define	SHIFT_FRAME_TYPE    	0
#define	SHIFT_SEC_ENABLED   	3
#define	SHIFT_FRAME_PENDING 	4
#define	SHIFT_ACK_REQ			5
#define	SHIFT_INTRA_PAN 		6
#define	SHIFT_DEST_ADDR_MODE 	6
#define	SHIFT_FRAME_VERSION 	4
#define	SHIFT_SRC_ADDR_MODE 	2

/* ieee 802.15.4 mac frame type */
enum
{
	 BEACON_FRAME = 0,
	 DATA_FRAME = 1,
	 ACK_FRAME = 2,
	 MAC_FRAME = 3
};

/* ieee 802.15.4 mac src/dest len */
enum
{
	ADDR_NOT_PRESENT = 0,
	ADDR16_MODE = 2,
	ADDR64_MODE = 3
};

/* ieee 802.15.4 ack request option */
enum
{
	ACK_REQ_FALSE = 0,
	ACK_REQ_TRUE = 1
};

/** ieee802154 mac layer functions */
uint8_t  ieee802154_packet_length_get(uint8_t * _packet);
uint8_t  ieee802154_header_length_get(uint8_t * _packet);
uint8_t  ieee802154_dest16_set(packet_t * pkt, uint16_t addr);
uint8_t  ieee802154_dest16_set_broadcast(packet_t * pkt);
uint16_t ieee802154_fcs_calc(uint8_t *data, uint8_t len);
uint8_t  ieee802154_packet_is_duplicated(uint16_t src_addr, uint8_t seq_num);
uint8_t  ieee802154_packet_input(packet_t *p);
void     ieee802154_seqnum_reset(uint16_t src_addr);
void     ieee802154_header_set(packet_t * _packet, uint8_t type, uint8_t ack_req);

uint8_t BitFieldGet(uint8_t var, uint8_t mask, uint8_t shift);
uint8_t BitFieldSet(uint8_t var, uint8_t mask, uint8_t shift, uint8_t value);

#define BITFIELD_SET(a,b,c,d)  (a) = BitFieldSet((a),(b),(c),(d));
#define BYTESTOSHORT(a,b)      ((uint16_t)((uint16_t)((a)<<8) + (uint16_t)(b)))

#endif /* UNET_IEEE802154_H_ */
