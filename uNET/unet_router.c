/*
 * \file unet_router.c
 *
 */

#include <stdint.h>
#include <string.h>
#include "packet.h"
#include "node.h"
#include "ieee802154.h"
#include "link.h"
#include "unet_router.h"
#include "unet_api.h"

packet_t packet_up;
packet_t packet_down;
packet_t packet_multicast_up;


//#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#define PRINTD(...) PRINTF(__VA_ARGS__)
#else
#define PRINTD(...)
#endif


struct
{
	uint16_t entry_index;
	unet_routing_table_up_t unet_routing_table_up[RTABLE_UP_ENTRIES_MAX_NUM];
}rtable_up = {.entry_index=0};

const char* route_error_tostring[] =
{
		"PACKET_SEND_OK",
		"DESTINATION_UNREACHABLE",
		"HOP_LIMIT_EXCEEDED",
		"TIMEOUT_IN_TRANSIT",
		"UNKNOWN"
};
/*--------------------------------------------------------------------------------------------*/
void* unet_rtable_up_get(void)
{
	return &(rtable_up.unet_routing_table_up);
}
/*--------------------------------------------------------------------------------------------*/
const char* packet_type_tostring[] =
{
		"BROADCAST_LOCAL_LINK",
		"UNICAST_DOWN",
		"UNICAST_UP",
		"UNICAST_ACK_DOWN",
		"UNICAST_ACK_UP",
		"MULTICAST_UP",
		"MULTICAST_ACK_UP"
};
uint8_t packet_type_toindex(unet_packet_type_t t)
{
	uint8_t idx = 0;
	switch(t)
	{
		case BROADCAST_LOCAL_LINK: idx=0; break;
		case UNICAST_DOWN: idx=1; break;
		case UNICAST_UP:  idx=2; break;
		case UNICAST_ACK_DOWN: idx=3; break;
		case UNICAST_ACK_UP: idx=4; break;
		case MULTICAST_UP: idx=5; break;
		case MULTICAST_ACK_UP: idx =6; break;
	}
	return idx;
}
const char* next_header_tostring[] =
{
		"NO_NEXT_HEADER",
		"NEXT_HEADER_TCP",
		"NEXT_HEADER_UDP",
		"NEXT_HEADER_UNET_CTRL_MSG",
		"NEXT_HEADER_UNET_APP",
		"NET_HEADER_RESERVED"
};

uint8_t next_header_toindex(uint8_t t)
{
	uint8_t idx = 0;
	switch(t)
	{
		case NO_NEXT_HEADER: idx=0; break;
		case NEXT_HEADER_TCP: idx=1; break;
		case NEXT_HEADER_UDP:  idx=2; break;
		case NEXT_HEADER_UNET_CTRL_MSG: idx=3; break;
		case NEXT_HEADER_UNET_APP: idx=4; break;
		case NET_HEADER_RESERVED: idx=5; break;
	}
	return idx;
}
void unet_header_print(packet_t *p)
{
#if UNET_HEADER_PRINT_ENABLE
	PRINTF("Received packet of type %s\r\n", packet_type_tostring[packet_type_toindex(p->packet[UNET_PKT_TYPE])]);
	PRINTF("payload length: %d\r\n", p->packet[UNET_PAYLOAD_LEN]);
	PRINTF("payload: ");
	packet_print(&p->packet[UNET_HEADER_END], p->packet[UNET_PAYLOAD_LEN]);
	PRINTF("hop limit:   %d\r\n", p->packet[UNET_HOP_LIMIT]);
	PRINTF("next header: %s\r\n", next_header_tostring[next_header_toindex(p->packet[UNET_NEXT_HEADER])]);
	PRINTF("from:  "); print_addr64(&p->packet[UNET_SRC_64]);
	PRINTF("panid: "); print_addr64(&p->packet[UNET_PANID_64]);
#else
	(void)p;
#endif
}
/*--------------------------------------------------------------------------------------------*/
void unet_header_set(packet_t * _pkt, uint8_t type, uint8_t hop_limit, uint8_t next_header, uint8_t *src, uint8_t *dst, uint8_t payload_len)
{

	_pkt->packet[UNET_PKT_TYPE] = type;
	_pkt->packet[UNET_HOP_LIMIT] = hop_limit;
	_pkt->packet[UNET_PAYLOAD_LEN] = payload_len;
	_pkt->packet[UNET_NEXT_HEADER] = next_header;

	memcpy(&_pkt->packet[UNET_SRC_64], src ,8);
	memcpy(&_pkt->packet[UNET_DEST_64], dst ,8);

	/* set LLC protocol field */
	_pkt->packet[LLC_PROTO] = UNET_LLC_PROTO_NUMBER;
}
/*--------------------------------------------------------------------------------------------*/
void unet_router_up_table_clear(void)
{
	uint8_t addr[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint16_t i;
	for(i=0;i<RTABLE_UP_ENTRIES_MAX_NUM; i++)
	{
		rtable_up.entry_index = 0;
		rtable_up.unet_routing_table_up[i].next_hop = 0;
		memcpy(rtable_up.unet_routing_table_up[i].dest_addr64.u8, addr, 8);
	}
}

/*--------------------------------------------------------------------------------------------*/
uint16_t unet_router_up_table_entry_find(addr64_t *dest64)
{

	uint16_t i;
//	PRINTF("table up:\n");
	for(i=0;i<RTABLE_UP_ENTRIES_MAX_NUM; i++)
	{
//		print_addr64(rtable_up.unet_routing_table_up[i].dest_addr64.u8);
		if(memcmp(rtable_up.unet_routing_table_up[i].dest_addr64.u8,dest64->u8,8) == 0)
		{
			break;
		}
	}
	return i;
}
/*--------------------------------------------------------------------------------------------*/
void unet_router_up_table_entry_add(uint16_t next_hop, addr64_t *dest64)
{
	uint16_t i;

	i = unet_router_up_table_entry_find(dest64);

	if(i < RTABLE_UP_ENTRIES_MAX_NUM)
	{
		/* update entry found */
		rtable_up.unet_routing_table_up[i].next_hop = next_hop;
		PRINTF_ROUTER(2,"Updated ");
	}
	else
	{
		/* entry not found, add at the end */
		i = rtable_up.entry_index++;
		rtable_up.entry_index = rtable_up.entry_index%RTABLE_UP_ENTRIES_MAX_NUM;
		rtable_up.unet_routing_table_up[i].next_hop = next_hop;
		memcpy(rtable_up.unet_routing_table_up[i].dest_addr64.u8, dest64->u8, 8);
		PRINTF_ROUTER(2, "Added ");
	}

	PRINTF_ROUTER(2,"route to: "); PRINTF_ROUTER_ADDR64(2,dest64->u8);
	PRINTF_ROUTER(2, " by neighbor %X \r\n", next_hop);
}
/*--------------------------------------------------------------------------------------------*/
uint16_t unet_router_up_table_entry_get(uint16_t idx)
{
	REQUIRE_FOREVER (idx < RTABLE_UP_ENTRIES_MAX_NUM);
	return rtable_up.unet_routing_table_up[idx].next_hop;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t unet_router_up(void)
{
	uint8_t  idx;
	addr64_t * dest_addr64;
	uint16_t next_hop_addr16;

	dest_addr64 = (addr64_t *)&packet_up.packet[UNET_DEST_64];
	idx = unet_router_up_table_entry_find(dest_addr64);

	if(idx == RTABLE_UP_ENTRIES_MAX_NUM)      return RESULT_DESTINATION_UNREACHABLE;
	if(packet_up.packet[UNET_HOP_LIMIT] == 0) return RESULT_HOP_LIMIT_EXCEEDED;

	packet_up.packet[UNET_HOP_LIMIT]--;

    packet_info_set(&packet_up, PKTINFO_SEQNUM, node_seq_num_get_next());

    // packet_info_set(&packet_up, PKTINFO_LAST_SEQNUM, packet_up.info[PKTINFO_SEQNUM]);

    packet_up.state = PACKET_IN_ROUTE;

    /* set next hop */
    next_hop_addr16 = unet_router_up_table_entry_get(idx);
    ieee802154_dest16_set(&packet_up, next_hop_addr16);

    ieee802154_header_set(&packet_up, DATA_FRAME, ACK_REQ_TRUE);

    packet_up.state = PACKET_WAITING_TX;
#if 0
    UNET_RADIO.send(&packet_up.packet[MAC_FRAME_CTRL], packet_up.packet[PHY_PKT_SIZE]);
#endif

    return RESULT_PACKET_SEND_OK;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t unet_packet_up_sendto(addr64_t * dest_addr64, uint8_t payload_len)
{
	uint8_t res = RESULT_PACKET_SEND_OK;
	uint8_t hop_limit = 0xFF;

	REQUIRE_OR_EXIT(dest_addr64 != NULL, exit_on_require_error);

	/* add network header */
	unet_header_set(&packet_up,
			UNICAST_UP,
			hop_limit,
			NEXT_HEADER_UNET_APP,
			node_addr64_get(),
			dest_addr64->u8,
			payload_len
	);

	packet_info_set(&packet_up, PKTINFO_SIZE,
	    		payload_len + UNET_NWK_HEADER_SIZE + UNET_LLC_HEADER_SIZE + UNET_MAC_HEADER_SIZE);

	res = unet_router_up();
	NODESTAT_UPDATE(netapptx);
	return res;

	exit_on_require_error:
	return RESULT_DESTINATION_NULL;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t unet_router_down(void)
{
	uint8_t p_idx;
	uint16_t next_hop_addr16;

	if(packet_down.packet[UNET_HOP_LIMIT] == 0) return FALSE;
	packet_down.packet[UNET_HOP_LIMIT]--;

    p_idx = node_data_get(NODE_PARENTINDEX);
    if (p_idx == NO_PARENT) return FALSE;

    packet_info_set(&packet_down, PKTINFO_SEQNUM, node_seq_num_get_next());

    //packet_info_set(&packet_down, PKTINFO_LAST_SEQNUM, packet_down.info[PKTINFO_SEQNUM]);

    packet_down.state = PACKET_IN_ROUTE;

    /* set next hop */
    next_hop_addr16 = link_neighbor_table_addr16_get(p_idx);
    ieee802154_dest16_set(&packet_down, next_hop_addr16);
    ieee802154_header_set(&packet_down, DATA_FRAME, ACK_REQ_TRUE);

    //packet_down.state = PACKET_WAITING_TX;

    return TRUE;
}
/*--------------------------------------------------------------------------------------------*/
void unet_update_packet_down_dest(void){
	uint8_t p_idx;
	uint16_t next_hop_addr16;
    p_idx = node_data_get(NODE_PARENTINDEX);
    if (p_idx == NO_PARENT) return; // no parent to be updated

    /* set next hop */
    next_hop_addr16 = link_neighbor_table_addr16_get(p_idx);
    ieee802154_dest16_set(&packet_down, next_hop_addr16);
    packet_down.packet[MAC_DEST_16] = packet_info_get(&packet_down,PKTINFO_DEST16L);
    packet_down.packet[MAC_DEST_16+1] = packet_info_get(&packet_down,PKTINFO_DEST16H);
}
/*--------------------------------------------------------------------------------------------*/
uint8_t unet_packet_down_send(uint8_t payload_len)
{
	extern packet_t packet_down;

	uint8_t hop_limit = node_data_get(NODE_DISTANCE);

	/// Retornar algum erro
	if(hop_limit >= ROUTE_TO_BASESTATION_LOST) return RESULT_DESTINATION_UNREACHABLE;

	hop_limit = 0xFF; /* set max hops */

	/* add network header */
	unet_header_set(&packet_down,
			UNICAST_DOWN,
			hop_limit,
			NEXT_HEADER_UNET_APP,
			node_addr64_get(),
			node_pan_id64_get(),
			payload_len
	);

    packet_info_set(&packet_down, PKTINFO_SIZE,
    		payload_len + UNET_NWK_HEADER_SIZE + UNET_LLC_HEADER_SIZE + UNET_MAC_HEADER_SIZE);

    /// Essa fun��o deve retornar o estado tb
    if (unet_router_down() == TRUE)
    {
    	NODESTAT_UPDATE(netapptx);
    	// Por enquanto retornando ok
    	return RESULT_PACKET_SEND_OK;
    }else{
    	return RESULT_DESTINATION_UNREACHABLE;
    }
}
/*--------------------------------------------------------------------------------------------*/
uint8_t unet_router_adv(void)
{
	extern packet_t packet_down;
	uint8_t payload_len = 0;

	uint8_t hop_limit = node_data_get(NODE_DISTANCE);

//	PRINTF("debug: router_adv\n");

	if(hop_limit >= ROUTE_TO_BASESTATION_LOST) return RESULT_DESTINATION_UNREACHABLE;
//	PRINTF("debug: reachable\n");

	hop_limit = 0xFF; /* set max hops */

	if(packet_acquire_down() == PACKET_ACCESS_DENIED) return RESULT_DESTINATION_UNREACHABLE;
//	PRINTF("debug: access granted\n");

	/* create router adv packet */
	memset(&packet_down.packet, 0, sizeof(packet_down.packet));

	payload_len = 1;

	packet_down.packet[UNET_CTRL_MSG_PACKET_TYPE] = ROUTER_ADV;

	/* add network header */
	unet_header_set(&packet_down,
			UNICAST_DOWN,
			hop_limit,
			NEXT_HEADER_UNET_CTRL_MSG,
			node_addr64_get(),
			node_pan_id64_get(),
			payload_len
	);

    packet_info_set(&packet_down, PKTINFO_SIZE,
    		payload_len + UNET_NWK_HEADER_SIZE + UNET_LLC_HEADER_SIZE + UNET_MAC_HEADER_SIZE);

    unet_router_down();

    return RESULT_PACKET_SEND_OK;

}
/*--------------------------------------------------------------------------------------------*/
//void RadioReset(void);
//void IncUNET_NodeStat_radioresets(void);
volatile ostick_t tick, ticked;
volatile int radio_status = RADIO_TX_WAIT;
uint8_t unet_packet_output(packet_t *pkt, uint8_t tx_retries, uint16_t delay_retry)
{
	uint8_t state = 0;
	uint8_t res = PACKET_SEND_ERR;
//	int radio_status = RADIO_TX_WAIT;

	acquireRadio();

		while(tx_retries-- > 0)
		{
			UNET_RADIO.get(RADIO_STATUS,&state);
			PRINTF_PHY(1,"RADIO STATE: %u \r\n", state);

			radio_status = UNET_RADIO.send(&(pkt->packet[MAC_FRAME_CTRL]), pkt->info[PKTINFO_SIZE]);
			// TODO set this as driver error in err list
			if(radio_status == -1){
				// Driver fail to put packet in radio
				break;
			}
			else if(radio_status == RADIO_TX_ERR_NONE){
				// Packet TX successful, ACK not needed
				NODESTAT_UPDATE(txed);
//				radio_tx_acked(TRUE);
				res =  PACKET_SEND_OK;
				break;
			}
			else if(radio_status == RADIO_TX_WAIT){
				// Packet TX successful, will wait ACK
				if(OSSemPend(Radio_TX_Event,TX_TIMEOUT) != TIMEOUT)
				{
					NODESTAT_UPDATE(txed);
					UNET_RADIO.get(RADIO_STATE,&state);
					if (is_radio_tx_acked(state))
					{
						radio_tx_acked(FALSE);
						res =  PACKET_SEND_OK;
						break;
					}else
					{
						NODESTAT_UPDATE(radionack);
						UNET_RADIO.get(TX_STATUS,&state);
						PRINTF_MAC(1,"TX SEM OK, but not acked! Cause: %u \r\n", state);
					}
				}
				else
				{
					NODESTAT_UPDATE(txfailed);
					UNET_RADIO.get(RADIO_STATUS,&state);
					PRINTF_MAC(1,"TX ISR Timeout. RADIO STATE: %u \r\n", state);

					PRINTD("DEBUG: fatal error, radio stuck\n");

					/* isso nunca deve acontecer, pois indica travamento do r�dio */
	//				NODESTAT_UPDATE(radioresets);
					extern void RadioReset(void);
					RadioReset();

				}
			}

			// Some error occurred, just delay it to another try
			DelayTask(delay_retry);
		}
		if(tx_retries == 0){
			NODESTAT_UPDATE(txmaxretries);
		}

	releaseRadio();

	return res;
}

/*--------------------------------------------------------------------------------------------*/
uint8_t unet_packet_input(packet_t *p)
{
	uint8_t ack_req = ACK_REQ_FALSE;
	unet_packet_type_t type = (unet_packet_type_t) p->packet[UNET_PKT_TYPE];

	packet_t *r = NULL;

    // Trafego da rede
    #ifdef ACTIVITY_LED
	     ACTIVITY_LED_TOGGLE;
    #endif

	IF_VERBOSE_LEVEL(UNET_VERBOSE_LINK,2,unet_header_print(p));

	switch(type)
	{
		case BROADCAST_LOCAL_LINK:
			if(p->packet[UNET_NEXT_HEADER] == NEXT_HEADER_UNET_CTRL_MSG)
			{
				if(p->packet[UNET_CTRL_MSG_PACKET_TYPE] == NEIGHBOR_REQ ||
						(p->packet[UNET_CTRL_MSG_PACKET_TYPE]&0x7F) == NEIGHBOR_ADV)
				{
						/* BROADCAST_LOCAL_LINK ->
						 * -> NEIGHBOR_ADV ? -> updates neighbor table -> updates parent/distance ->
						 * updates default down route -> schedule broadcast local link
						 * -> return NO_ACK ->  end
						 * -> NEIGHBOR_REQ ? -> updates neighbor table ->
						 * schedule broadcast local link ("TX local_link packet" event)
						 * -> return NO_ACK ->  end
						 */
						link_packet_process(p);
				}
			}
			ack_req = ACK_REQ_FALSE;
			break;
		case UNICAST_UP:
			/* check buffer UP is empty ->
			 * -> YES: copy to packet buffer UP ->
			 * post uNET "route UP" event -> return ACK
			 * -> NO: discards -> return NO_ACK -> end
			*/
			if(packet_acquire_up() == PACKET_ACCESS_DENIED)
			{
				ack_req = ACK_REQ_FALSE;
			}
			else
			{
				r=&packet_up;
				ack_req = ACK_REQ_TRUE;
				memcpy(r,p,sizeof(packet_t));

				/* backup last seq number */
				//r->info[PKTINFO_LAST_SEQNUM] = r->info[PKTINFO_SEQNUM];

				PRINTF_ROUTER(2,"RX ROUTE UP\r\n");

				/* todo: set node activity  flag */
				link_set_neighbor_activity(r);

				/* packet is in buffer, start sending ack */
				extern BRTOS_Sem* Router_Up_Ack_Request;

				if(Router_Up_Ack_Request != NULL)
				{
					OSSemPost(Router_Up_Ack_Request);
					PRINTF_ROUTER(2,"RX UP in buffer, SN %d \r\n", r->info[PKTINFO_SEQNUM]);
				}else
				{
					packet_release_up();
				}
			}
			break;
		case UNICAST_DOWN:
			/* check buffer DOWN is empty -> YES: copy to packet buffer DOWN ->
			 * ROUTER_ADV? YES: store/update route -> post uNET "route DOWN" event
			 * -> return ACK | NO: discards -> return NO_ACK -> end
			 */
			r=&packet_down; /* try to use output buffer */

			/* todo: fazer uma fun��o apenas, passando o ponteiro do pacote e
			 * o estado que vai estar em caso de sucesso no acesso ao buffer */
			if(packet_acquire_down() == PACKET_ACCESS_DENIED)
			{
				PRINTF_LINK(1,"BUFFER FULL! RX DROPPED! from: %u, SN: %u\r\n",
						BYTESTOSHORT(p->info[PKTINFO_SRC16H],p->info[PKTINFO_SRC16L]), p->info[PKTINFO_SEQNUM]);

				PRINTF_LINK(2,"RX PACKET STATE: %u\r\n", p->state);
				PRINTF_LINK(2,"BUFFER PACKET STATE: %u\r\n", r->state);

				NODESTAT_UPDATE(overbuf);
				ack_req = ACK_REQ_FALSE;
			}
			else
			{
				ack_req = ACK_REQ_TRUE;

				/* todo: este c�digo poder� ser colocado mais adiante, pois o resultado n�o � usado aqui
				 * s� para debug.  */
				if(p->info[PKTINFO_DUPLICATED] == TRUE)
				{
					PRINTF_LINK(1,"RX PACKET DUPLICATE! from %u, SN: %u\r\n",
							BYTESTOSHORT(p->info[PKTINFO_SRC16H],p->info[PKTINFO_SRC16L]), p->info[PKTINFO_SEQNUM]);
				}

				/* it will be stored in buffer, change state to sending ack */
				p->state = PACKET_SENDING_ACK;

				memcpy(r,p,sizeof(packet_t));

				/* todo: move the code below to the router down task ? */
				if(r->packet[UNET_NEXT_HEADER] == NEXT_HEADER_UNET_CTRL_MSG &&
						r->packet[UNET_CTRL_MSG_PACKET_TYPE] == ROUTER_ADV)
				{
					PRINTF_ROUTER(2,"RX ROUTE_ADV\r\n");
				}


				uint16_t src_addr16 = (r->info[PKTINFO_SRC16H] << 8) + r->info[PKTINFO_SRC16L];

				addr64_t * dest_addr64 = (addr64_t *) &(r->packet[UNET_SRC_64]);
				unet_router_up_table_entry_add(src_addr16, dest_addr64);

				PRINTF_ROUTER(2,"Route added or updated\r\n");
				PRINTF_ROUTER(1,"RX DOWN in buffer, from %04X SN %02X \r\n", src_addr16, r->info[PKTINFO_SEQNUM]);

				/* todo: set node activity  flag */
				link_set_neighbor_activity(r);

				/* packet is in buffer, start sending ack */
				extern BRTOS_Sem* Router_Down_Ack_Request;

				if(Router_Down_Ack_Request != NULL)
				{
					OSSemPost(Router_Down_Ack_Request);
				}else
				{
					packet_release_down();
				}
			}
			break;
		case UNICAST_ACK_UP:
			 /* -> check buffer UP is full ->
			  * YES: post "ACK UP" event |
			  * NO: discards -> return NO_ACK -> end
			  */
			r=&packet_up;

			PRINTF_ROUTER(1,"RX ACK UP, from %u, SN %d, ACK SN: %d\r\n",
					BYTESTOSHORT(r->info[PKTINFO_DEST16H],r->info[PKTINFO_DEST16L]),
					r->info[PKTINFO_SEQNUM], p->info[PKTINFO_SEQNUM]);

			if(p->info[PKTINFO_SEQNUM] == r->info[PKTINFO_SEQNUM])
			{
				if(r->state == PACKET_WAITING_ACK)
				{
				    /* post a "ack up" event */
					extern BRTOS_Sem* Router_Up_Ack_Received;
					OSSemPost(Router_Up_Ack_Received);
				}
			}
			ack_req = ACK_REQ_FALSE;
			break;
		case UNICAST_ACK_DOWN:
			 /* -> check buffer DOWN is full ->
			  * YES: post "ACK DOWN" event |
			  * NO: discards -> return NO_ACK -> end
			  */
			r=&packet_down;
			PRINTF_ROUTER(1,"RX ACK DOWN, from %u, SN %d, ACK SN: %d\r\n",
					BYTESTOSHORT(r->info[PKTINFO_DEST16H],r->info[PKTINFO_DEST16L]),
					r->info[PKTINFO_SEQNUM], p->info[PKTINFO_SEQNUM]);

			/* se o pacote do buffer est� esperando um ack, e o ack recebido � para este pacote (mesmo SN e SRC == DEST),
			 * ent�o marca ele como ack'ed e posta o sem�foro de ack recebido. */
			if(r->state == PACKET_WAITING_ACK)
			{
				if((p->info[PKTINFO_SEQNUM] == r->info[PKTINFO_SEQNUM]) &&
						(p->info[PKTINFO_SRC16H] == r->info[PKTINFO_DEST16H]) &&
						(p->info[PKTINFO_SRC16L] == r->info[PKTINFO_DEST16L]))
				{
					r->state = PACKET_ACKED;

					/* post a "ack down" event */
					extern BRTOS_Sem* Router_Down_Ack_Received;
					OSSemPost(Router_Down_Ack_Received);
				}
				else {
					NODESTAT_UPDATE(txnacked);
				}
			}else
			{
				/* todo: isso � s� para debug e pode ser removido futuramente. */
				PRINTF_LINK(1,"PACKET STATE ERROR: at %u \r\n", __LINE__);
			}
			ack_req = ACK_REQ_FALSE;
			break;
		case MULTICAST_UP: /* todo: for future extension */
			ack_req = ACK_REQ_TRUE;
			break;
		case MULTICAST_ACK_UP: /* todo: for future extension */
			ack_req = ACK_REQ_FALSE;
			break;
		default:
			break;
	}
	return ack_req;
}
/*--------------------------------------------------------------------------------------------*/
