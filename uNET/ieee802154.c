/*
 * \file ieee802154.c
 *
 */

#include "NetConfig.h"
#include "ieee802154.h"

#include "packet.h"
#include "node.h"
#include "link.h"
#include "unet_router.h"
#include "unet_api.h"

#if (!defined PANID_INIT_VALUE) || (!defined MAC16_INIT_VALUE) || (!defined ROUTC_INIT_VALUE)
#error "Please define 'PANID_INIT_VALUE', 'MAC16_INIT_VALUE' and 'ROUTC_INIT_VALUE'"
#endif

static uint8_t byte_reverse(uint8_t b)
{
    return   b = (uint8_t)(((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
}

uint8_t BitFieldGet(uint8_t var, uint8_t mask, uint8_t shift)
{
    return (var & mask) >> shift;
}

uint8_t BitFieldSet(uint8_t var, uint8_t mask, uint8_t shift, uint8_t value)
{
    return (var & ~mask) | ((value << shift) & mask);
}

/*--------------------------------------------------------------------------------------------*/
uint8_t ieee802154_dest16_set(packet_t * pkt, uint16_t addr)
{
	packet_info_set(pkt, PKTINFO_DEST16H, H(addr));
	packet_info_set(pkt, PKTINFO_DEST16L, L(addr));
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t ieee802154_dest16_set_broadcast(packet_t * pkt)
{
	return ieee802154_dest16_set(pkt, BROAD_ADDR);
}
/*--------------------------------------------------------------------------------------------*/
uint8_t ieee802154_packet_length_get(uint8_t * _packet)
{
	return _packet[PHY_PKT_SIZE];
}
/*--------------------------------------------------------------------------------------------*/
uint8_t last_rx_sequences[MAX_SEQUENCES_NUM];
uint8_t last_rx_seq_idx = 0;

uint8_t ieee802154_packet_input(packet_t *p)
{

	uint8_t ack_req = ACK_REQ_FALSE;
#if RADIO_AUTOCRC == TRUE
	uint8_t crc = 0;
#endif

	REQUIRE_FOREVER(p != NULL);

	static ieee802154_frame_control_t fc;
	fc.u8[0] = p->packet[MAC_FRAME_CTRL];
	fc.u8[1] = p->packet[MAC_FRAME_CTRL+1];
	p->info[PKTINFO_SIZE] = p->packet[PHY_PKT_SIZE];

#if USE_BITMASK == 1
	/* only data frame is supported */
	if(BitFieldGet(fc.u8[0],MASK_FRAME_TYPE,SHIFT_FRAME_TYPE) != DATA_FRAME) return ACK_REQ_FALSE;

	/* 64-bit addressing not supported */
	if((BitFieldGet(fc.u8[1],MASK_DEST_ADDR_MODE,SHIFT_DEST_ADDR_MODE) != ADDR16_MODE)) return ACK_REQ_FALSE;
	if((BitFieldGet(fc.u8[1],MASK_SRC_ADDR_MODE,SHIFT_SRC_ADDR_MODE) != ADDR16_MODE)) return ACK_REQ_FALSE;

	/* security not supported yet */
	if((BitFieldGet(fc.u8[0],MASK_SEC_ENABLED,SHIFT_SEC_ENABLED) != FALSE)) return ACK_REQ_FALSE;

#else
	/* only data frame is supported */
	if(fc.bits.FrameType != DATA_FRAME) return ACK_REQ_FALSE;

	/* 64-bit addressing not supported */
	if((fc.bits.SrcAddrMode != ADDR16_MODE) || (fc.bits.DstAddrMode != ADDR16_MODE)) return ACK_REQ_FALSE;

	/* security not supported yet */
	if(fc.bits.SecurityEnabled == TRUE) return ACK_REQ_FALSE;
#endif


#if RADIO_AUTOCRC == TRUE
	UNET_RADIO.get(CRC, &crc);
	if(crc == FALSE){
		//// TODO RIME, wrong CRC
		PRINTF_MAC(1,"CRC error!\r\n");
		return ACK_REQ_FALSE;
	}
#else
	if(((p->info[PKTINFO_FCSH]<<8) + p->info[PKTINFO_FCSL]) !=
			ieee802154_fcs_calc(&p->packet[MAC_FRAME_CTRL], p->info[PKTINFO_SIZE]))
	{
		//// TODO RIME, this inform if the CRC of the packet is wrong!!!

		PRINTF_MAC(1,"FCS error! FCS RX: %02X, FCS CALC: %02X\r\n",(p->info[PKTINFO_FCSH]<<8) + p->info[PKTINFO_FCSL],
				ieee802154_fcs_calc(&p->packet[MAC_FRAME_CTRL], p->info[PKTINFO_SIZE]));
		return ACK_REQ_FALSE;
	}
#endif

	/* only UNET nwk protocol currently supported */
	if(p->packet[LLC_PROTO] != UNET_LLC_PROTO_NUMBER) return ACK_REQ_FALSE;

	p->info[PKTINFO_SEQNUM] = p->packet[MAC_SEQ_NUM];
	p->info[PKTINFO_SRC16H] = p->packet[MAC_SRC_16+1];
	p->info[PKTINFO_SRC16L] = p->packet[MAC_SRC_16];

	/* todo: colocar uma macro p/ desabilitar isto. � usado s� para debug. */
	last_rx_sequences[last_rx_seq_idx++%MAX_SEQUENCES_NUM] = p->info[PKTINFO_SEQNUM];

	PRINTF_MAC(2,"PACKET RX from %u! SN: %u\r\n", BYTESTOSHORT(p->packet[MAC_SRC_16+1],p->packet[MAC_SRC_16]), p->packet[MAC_SEQ_NUM]);
	PRINTF_LINK(2,"PACKET RX from %u! SN: %u\r\n", BYTESTOSHORT(p->info[PKTINFO_SRC16H],p->info[PKTINFO_SRC16L]), p->info[PKTINFO_SEQNUM]);

	p->info[PKTINFO_DUPLICATED] = FALSE;

#if USE_BITMASK == 1
	if((BitFieldGet(fc.u8[0],MASK_ACK_REQ,SHIFT_ACK_REQ) == TRUE))
#else
	if(fc.bits.ACKRequest == TRUE)
#endif
	{
		/* todo: futuramente este teste poder� ser feito mais adiante, pois o resultado n�o � usado aqui. */
		if(link_packet_is_duplicated(&(p->packet[MAC_SRC_16]), p->packet[MAC_SEQ_NUM]) == TRUE)
		{
			NODESTAT_UPDATE(duplink);
			p->info[PKTINFO_DUPLICATED] = TRUE;
			PRINTF_LINK(1,"PKT DUPLICATED! from: %u, SN: %u\r\n", BYTESTOSHORT(p->packet[MAC_SRC_16+1],p->packet[MAC_SRC_16]), p->packet[MAC_SEQ_NUM]);
		}
	}

	ack_req = unet_packet_input(p);

	return ack_req;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t ieee802154_header_length_get(uint8_t * _packet)
{
	return _packet[MAC_HEADER_SIZE];
}
/*--------------------------------------------------------------------------------------------*/
void ieee802154_header_set(packet_t * _packet, uint8_t type, uint8_t ack_req)
{
	ieee802154_frame_control_t fc;
	fc.u8[0] = 0x00;
	fc.u8[1] = 0x00;

#if USE_BITMASK == 1
	BITFIELD_SET(fc.u8[0], MASK_FRAME_TYPE, SHIFT_FRAME_TYPE, type);
	BITFIELD_SET(fc.u8[0], MASK_ACK_REQ, SHIFT_ACK_REQ, ack_req);

	/* only intra PAN mode is supported */
	BITFIELD_SET(fc.u8[0], MASK_INTRA_PAN, SHIFT_INTRA_PAN, TRUE);

	/* only 16-bit address mode is supported */
	BITFIELD_SET(fc.u8[1], MASK_DEST_ADDR_MODE, SHIFT_DEST_ADDR_MODE, ADDR16_MODE);
	BITFIELD_SET(fc.u8[1], MASK_SRC_ADDR_MODE, SHIFT_SRC_ADDR_MODE, ADDR16_MODE);
#else
	//fc.u16 = 0;
	fc.bits.FrameType = type;
	fc.bits.ACKRequest = ack_req;

	/* only intra PAN mode is supported */
	fc.bits.IntraPAN = TRUE;
	/* only 16-bit address mode is supported */
	fc.bits.SrcAddrMode = ADDR16_MODE;
	fc.bits.DstAddrMode = ADDR16_MODE;
#endif

	_packet->packet[MAC_FRAME_CTRL] = fc.u8[0];
	_packet->packet[MAC_FRAME_CTRL+1] = fc.u8[1];
	_packet->packet[MAC_SEQ_NUM] = packet_info_get(_packet,PKTINFO_SEQNUM);
	_packet->packet[MAC_PANID_16] = 0xff;//node_data_get(NODE_PANID16L);
	_packet->packet[MAC_PANID_16+1] = 0xff;//node_data_get(NODE_PANID16H);
	_packet->packet[MAC_DEST_16] = packet_info_get(_packet,PKTINFO_DEST16L);
	_packet->packet[MAC_DEST_16+1] = packet_info_get(_packet,PKTINFO_DEST16H);
	_packet->packet[MAC_SRC_16] =  node_data_get(NODE_ADDR16L);
	_packet->packet[MAC_SRC_16+1] = node_data_get(NODE_ADDR16H);
	_packet->packet[MAC_HEADER_SIZE] = UNET_MAC_HEADER_SIZE;
	_packet->packet[PHY_PKT_SIZE] = packet_info_get(_packet,PKTINFO_SIZE);
}


/*--------------------------------------------------------------------------------------------*/
const uint16_t _ccitt_crc16_table[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

uint16_t ieee802154_fcs_calc(uint8_t *data, uint8_t len)
{

    uint8_t tmp = 0;
    uint8_t short_data = 0;
    uint16_t fcs_tmp = 0;
    uint16_t fcs = 0;
    uint8_t tmp2 = 0;
    uint8_t tmp3 = 0;

    REQUIRE_FOREVER(data != NULL);
    while(len > 0)
    {
    	len--;
    	short_data = byte_reverse(*data++);
    	tmp = (uint8_t)((uint8_t)(fcs >> 8) ^ short_data);
    	fcs_tmp = (uint16_t)_ccitt_crc16_table[tmp];
    	fcs = (uint16_t)((uint16_t)(fcs << 8) ^ fcs_tmp);
    }

    tmp2 = byte_reverse((uint8_t)(fcs >> 8));
    tmp3 = byte_reverse((uint8_t)(fcs & 0xFF));

	#if (BRTOS_ENDIAN == BRTOS_LITTLE_ENDIAN)
    fcs = ((uint16_t)((tmp2 << 8) | tmp3));
	#else
    fcs = ((uint16_t)((tmp2 << 8) | tmp3));
	#endif

    return fcs;
}
/*--------------------------------------------------------------------------------------------*/
