/*
 * packet.h
 *
 *  Created on: 12/04/2016
 *      Author: Avell
 */

#ifndef UNET_PACKET_H_
#define UNET_PACKET_H_

#include "BRTOSConfig.h"
#include "NetConfig.h"
#include "stdint.h"

#define MAX_PACKET_SIZE 	 		(128)
#define UNET_PHY_HEADER_SIZE 	 	(1)

#define BIG_ENDIAN 			BRTOS_BIG_ENDIAN
#define LITTLE_ENDIAN		BRTOS_LITTLE_ENDIAN
#define BYTE_ORDER			BRTOS_ENDIAN

#ifndef HTONS
#   if BYTE_ORDER == BIG_ENDIAN
#      define HTONS(n) (n)
#      define HTONL(n) (n)
#   else /* BYTE_ORDER == BIG_ENDIAN */
#      define HTONS(n) (uint16_t)((((uint16_t) (n)) << 8) | (((uint16_t) (n)) >> 8))
#      define HTONL(n) (((uint32_t)HTONS(n) << 16) | HTONS((uint32_t)(n) >> 16))
#   endif /* BYTE_ORDER == BIG_ENDIAN */
#else
#error "HTONS already defined!"
#endif /* HTONS */

#if DEBUG_PRINTF && WINNT
//#include <stdio.h>
//#define PRINTF(...) printf(__VA_ARGS__); fflush(stdout);
#elif DEBUG_PRINTF
// configured at brtosconfig.h
//#include "stdio.h"
//#define PRINTF(...) printf(__VA_ARGS__);
#else
#define PRINTF(...)
#endif


enum {UNET_VERBOSE_PHY = 0, UNET_VERBOSE_MAC, UNET_VERBOSE_LINK, UNET_VERBOSE_ROUTER, UNET_VERBOSE_TRANSPORT, UNET_VERBOSE_APP, UNET_VERBOSE_LEVEL_MAX = 0xFFFF};

#define IF_VERBOSE_LEVEL(verb_layer, level, dothis) do { extern uint16_t unet_verbose; if(((unet_verbose & (0x3 << (verb_layer*2))) >> (verb_layer*2)) >= (level)) {dothis;} } while(0);

#define PRINTF_VERB(verb_layer, level, ...)   IF_VERBOSE_LEVEL(verb_layer, level, PRINTF(__VA_ARGS__));
#define PRINTF_PHY(level, ...)  PRINTF_VERB(UNET_VERBOSE_PHY,(level),__VA_ARGS__);
#define PRINTF_MAC(level, ...)  PRINTF_VERB(UNET_VERBOSE_MAC,(level),__VA_ARGS__);
#define PRINTF_LINK(level,...)  PRINTF_VERB(UNET_VERBOSE_LINK,(level),__VA_ARGS__);
#define PRINTF_ROUTER(level,...)  PRINTF_VERB(UNET_VERBOSE_ROUTER,(level),__VA_ARGS__);
#define PRINTF_TRANSPORT(level,...)  PRINTF_VERB(UNET_VERBOSE_TRANSPORT,(level),__VA_ARGS__);
#define PRINTF_APP(level,...)  PRINTF_VERB(UNET_VERBOSE_APP,(level),__VA_ARGS__);
#define print_addr64(addr) 	PRINTF("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n\r", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])

#define PRINTF_VERB_ADDR64(verb_layer, level, arg)  IF_VERBOSE_LEVEL(verb_layer, level, print_addr64(arg)) //do { extern uint16_t unet_verbose; if(((unet_verbose & (0x3 << (verb_layer))) >> (verb_layer*2)) >= (level)) {print_addr64(arg);} } while(0);
#define PRINTF_APP_ADDR64(x,arg) PRINTF_VERB_ADDR64(UNET_VERBOSE_APP, x, arg)
#define PRINTF_ROUTER_ADDR64(x,arg) PRINTF_VERB_ADDR64(UNET_VERBOSE_ROUTER, x, arg)


#if WINNT
#define REQUIRE(x,dothis)			if(!(x)) { PRINTF("Invalid value at %s:%d\r\n", __FILE__, __LINE__); dothis}
#define REQUIRE_FOREVER(x)			REQUIRE(x, while(1){})
#define REQUIRE_OR_EXIT(x, exit)	REQUIRE(x, goto exit;)
#else
#define REQUIRE(x, dothis)			if(!(x)) { PRINTF("Invalid value at %s:%d\r\n", __FILE__, __LINE__); dothis}
#define REQUIRE_FOREVER(x)			REQUIRE(x, while(1){DelayTask(1000);})
#define REQUIRE_OR_EXIT(x, exit)	REQUIRE(x, goto exit;)
#endif

enum
{
	PACKET_ACCESS_ALLOWED,
	PACKET_ACCESS_DENIED
};

enum
{
	PACKET_SEND_OK,
	PACKET_SEND_ERR
};

enum
{
	MAC_HEADER_SIZE = 0,
	/* ieee802.15.4 phy header start */
	PACKET_BEGIN = 1,
	PHY_PKT_SIZE = 1,
	/* ieee802.15.4 mac header start - a fixed header size is used */
	MAC_FRAME_CTRL = 2,
	MAC_SEQ_NUM = 4,
	MAC_PANID_16 = 5,
	MAC_DEST_16 = 7,
	MAC_SRC_16 = 9,
	MAC_HEADER_END = 11,
	/* unet llc header start */
	LLC_HEADER = 11,
	LLC_PROTO = 11,
	/* unet network header start */
	UNET_HEADER_START = 12,
	UNET_PKT_TYPE = 12,
	UNET_PAYLOAD_LEN = 13,
	UNET_HOP_LIMIT = 14,
	UNET_NEXT_HEADER = 15,
	UNET_SRC_64 = 16,
	UNET_DEST_64 = 24,
	UNET_PANID_64 = 24,
	UNET_HEADER_END = 32,
	/* unet transport header start */
	UNET_TRANSP_HEADER_START = 32,
	UNET_SOURCE_PORT = 32,
	UNET_DEST_PORT = 33,
	UNET_APP_PAYLOAD_LEN = 34,
	UNET_TRANSP_HEADER_END = 35,
	/* unet app header start */
	UNET_APP_HEADER_START = 35,
	PACKET_END = MAX_PACKET_SIZE
};


typedef enum
{
	PACKET_IDLE,
	PACKET_BUSY,
	PACKET_START_ROUTE,
	PACKET_IN_ROUTE,
	PACKET_WAITING_TX,
	PACKET_WAITING_ACK,
	PACKET_SENDING_ACK,
	PACKET_ACKED,
	PACKET_NOT_ACKED,
	PACKET_DELIVERED,
} packet_state_t;

typedef enum
{
	//PKTINFO_STATE,
	PKTINFO_SIZE,
	PKTINFO_SRC16H,
	PKTINFO_SRC16L,
	PKTINFO_DEST16H,
	PKTINFO_DEST16L,
	PKTINFO_DUPLICATED,
	PKTINFO_TXCHANNEL,
	//PKTINFO_LAST_SEQNUM,
	PKTINFO_SEQNUM,
	PKTINFO_RSSI,
	PKTINFO_LQI,
	PKTINFO_FCSH,
	PKTINFO_FCSL,
	PKTINFO_MAX_OPT
}packet_info_t;

typedef struct
{
	uint8_t 		info[PKTINFO_MAX_OPT];
	uint8_t 		packet[MAX_PACKET_SIZE];
	packet_state_t  state;
} packet_t;

#define UNET_MAC_HEADER_SIZE  	    (9)
#define UNET_LLC_HEADER_SIZE  	    (1)
#define UNET_NWK_HEADER_SIZE  	    (20)
#define UNET_TRANSP_HEADER_SIZE  	(3)

#define  MAX_PACKET_ACK_SIZE  (UNET_MAC_HEADER_SIZE+UNET_LLC_HEADER_SIZE+UNET_NWK_HEADER_SIZE)

typedef struct
{
	uint8_t info[PKTINFO_MAX_OPT];
	uint8_t packet[MAX_PACKET_ACK_SIZE];
} packet_ack_t;

/** generic packet functions */
uint8_t  packet_info_set(packet_t *pkt, packet_info_t opt, uint8_t val);
uint8_t  packet_info_get(packet_t *pkt, packet_info_t opt);
uint8_t  packet_acquire_down(void);
void     packet_release_down(void);
uint8_t  packet_acquire_up(void);
void     packet_release_up(void);
packet_state_t  packet_state_down(void);
packet_state_t  packet_state_up(void);
void     packet_print(uint8_t *pkt, uint8_t len);
uint16_t packet_get_source_addr16(packet_t *pkt);
uint16_t packet_get_dest_addr16(packet_t *pkt);


#endif /* UNET_PACKET_H_ */
