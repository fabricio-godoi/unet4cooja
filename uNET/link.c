/*
 * \file link.c
 *
 */

#include <stdint.h>
#include <string.h>
#include "unet_api.h"

#include "packet.h"
#include "node.h"
#include "ieee802154.h"
#include "link.h"
#include "unet_router.h"

/* neighbor table */
volatile unet_neighborhood_t  unet_neighbourhood[NEIGHBOURHOOD_SIZE];
packet_t link_pkt;

volatile uint16_t							 ParentNeighborID = 0xFFFE;
volatile uint16_t							 ParentRSSI = 0;
volatile neighbor_table_t                	 NeighborTable = 0;
volatile uint8_t               			 	 NeighborLinkPacketTimeCnt  = 1;

#if UNET_DEVICE_TYPE == PAN_COORDINATOR
volatile uint8_t just_reset = TRUE;
volatile uint8_t link_reset_cnt = 0;
#endif

/*--------------------------------------------------------------------------------------------*/
packet_t * link_packet_get(void)
{
	return &link_pkt;
}
/*--------------------------------------------------------------------------------------------*/
void link_packet_create(void)
{
	uint8_t i = 0, neighbor_cnt = 0, payload_len = 0;
	uint8_t *address;

	memset(&link_pkt.packet, 0, sizeof(link_pkt.packet));

	/* create local link packet */
	link_pkt.packet[UNET_CTRL_MSG_ADDR16] = (uint8_t)(node_data_get(NODE_ADDR16H));
	link_pkt.packet[UNET_CTRL_MSG_ADDR16+1] = (uint8_t)(node_data_get(NODE_ADDR16L));
	link_pkt.packet[UNET_CTRL_MSG_DISTANCE] = (uint8_t)(node_data_get(NODE_DISTANCE));

	address = &link_pkt.packet[UNET_CTRL_MSG_NB_START];

	/* add neighbor table */
	for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	{
		if (unet_neighbourhood[i].Addr_16b != UNUSED_ADDR)
		{
		  *address++ = (uint8_t)(H((unet_neighbourhood[i].Addr_16b)));
		  *address++ = (uint8_t)(L((unet_neighbourhood[i].Addr_16b)));
		  *address++ = (uint8_t)(unet_neighbourhood[i].NeighborRSSI);
		  neighbor_cnt++;
		}
	}

	link_pkt.packet[UNET_CTRL_MSG_NB_COUNT] = neighbor_cnt;

	payload_len = neighbor_cnt*3 + 5;

	if(node_data_get(NODE_DISTANCE) == NO_ROUTE_TO_BASESTATION)
	{
		link_pkt.packet[UNET_CTRL_MSG_PACKET_TYPE] = NEIGHBOR_REQ;
	}else
	{
		#if UNET_DEVICE_TYPE == PAN_COORDINATOR
		if ((node_data_get(NODE_DISTANCE) == 0) && (just_reset == TRUE)){
			link_pkt.packet[UNET_CTRL_MSG_PACKET_TYPE] = NEIGHBOR_ADV_RESET;
			link_reset_cnt++;
			if (link_reset_cnt >= 3) just_reset = FALSE;
		}else{
			link_pkt.packet[UNET_CTRL_MSG_PACKET_TYPE] = NEIGHBOR_ADV;
		}
		#else
		link_pkt.packet[UNET_CTRL_MSG_PACKET_TYPE] = NEIGHBOR_ADV;
		#endif
	}

	/* add network header */
	unet_header_set(&link_pkt,
			BROADCAST_LOCAL_LINK,
			1,
			NEXT_HEADER_UNET_CTRL_MSG,
			node_addr64_get(),
			node_pan_id64_get(),
			payload_len
	);

    packet_info_set(&link_pkt, PKTINFO_SIZE,
    		payload_len + UNET_NWK_HEADER_SIZE + UNET_LLC_HEADER_SIZE + UNET_MAC_HEADER_SIZE);

    packet_info_set(&link_pkt, PKTINFO_SEQNUM, node_seq_num_get_next());
    ieee802154_dest16_set_broadcast(&link_pkt);
	ieee802154_header_set(&link_pkt, DATA_FRAME, ACK_REQ_FALSE);
}
/*--------------------------------------------------------------------------------------------*/
void link_neighbor_table_init(void)
{

   uint8_t i;

   for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
   {
      unet_neighbourhood[i].Addr_16b            			= 0xFFFE;
      unet_neighbourhood[i].NeighborRSSI        			= 0;
      unet_neighbourhood[i].NeighborLastID      			= 0;
      unet_neighbourhood[i].NeighborMyRSSI        			= 0;
      unet_neighbourhood[i].NeighborDistance       			= NO_ROUTE_TO_BASESTATION;
      unet_neighbourhood[i].NeighborStatus.bits.Symmetric 	= FALSE;
      unet_neighbourhood[i].NeighborStatus.bits.Active 		= 0;
   }
}
/*--------------------------------------------------------------------------------------------*/
packet_t * link_packet_create_association_request(void)
{

	node_pan_id64_init();
	link_neighbor_table_init();

	node_data_set(NODE_PANID16H,H(BROAD_ADDR));
	node_data_set(NODE_PANID16L,L(BROAD_ADDR));
	node_data_set(NODE_DISTANCE,NO_ROUTE_TO_BASESTATION);

	link_packet_create();
	return link_packet_get();

}
/*--------------------------------------------------------------------------------------------*/
void link_packet_create_ack(packet_t *p)
{
	/* revert src/dest */
	p->info[PKTINFO_DEST16H] = p->packet[MAC_SRC_16+1];
	p->info[PKTINFO_DEST16L] = p->packet[MAC_SRC_16];
	p->info[PKTINFO_SEQNUM] =  p->packet[MAC_SEQ_NUM];
	p->packet[UNET_PKT_TYPE] = p->packet[UNET_PKT_TYPE] & ~ACK_REQ_MASK;
    packet_info_set(p, PKTINFO_SIZE,
    		UNET_NWK_HEADER_SIZE + UNET_LLC_HEADER_SIZE + UNET_MAC_HEADER_SIZE);
	ieee802154_header_set(p, DATA_FRAME, ACK_REQ_TRUE);
}
/*--------------------------------------------------------------------------------------------*/
void link_neighbor_symmetry_update(packet_t *p, uint8_t idx)
{

    uint8_t j = 0;

    link_neighbor_t *neighbor = (link_neighbor_t *)&p->packet[UNET_CTRL_MSG_NB_START];

    for(j=0;j<(p->packet[UNET_CTRL_MSG_NB_COUNT]);j++)
    {

      if (neighbor->addr16h == node_data_get(NODE_ADDR16H) &&
    		  neighbor->addr16l == node_data_get(NODE_ADDR16L))
      {

		if(unet_neighbourhood[idx].NeighborMyRSSI < neighbor->rssi)
		{
			unet_neighbourhood[idx].NeighborMyRSSI = neighbor->rssi;
		}
		else
		{
			unet_neighbourhood[idx].NeighborMyRSSI = (unet_neighbourhood[idx].NeighborMyRSSI*7 + neighbor->rssi) >> 3;
		}

    	if((unet_neighbourhood[idx].NeighborMyRSSI >= RSSI_THRESHOLD) && (unet_neighbourhood[idx].NeighborRSSI >= RSSI_THRESHOLD))
    	{
			unet_neighbourhood[idx].NeighborStatus.bits.Symmetric = TRUE;
    	}else{
    	    unet_neighbourhood[idx].NeighborStatus.bits.Symmetric = FALSE;
    	}

		break;
      }
      neighbor++; /* go to next neighbor */
    }

}
/*--------------------------------------------------------------------------------------------*/
static uint8_t is_symmetric_neighbor(uint8_t idx)
{
	if (unet_neighbourhood[idx].NeighborStatus.bits.Symmetric){
		return TRUE;
	}else{
		return FALSE;
	}
}

uint8_t is_neighbor_parent(uint8_t idx)
{
	return (idx == node_data_get(NODE_PARENTINDEX));
}

uint8_t link_is_symmetric_parent(void)
{
	uint8_t parent_idx = node_data_get(NODE_PARENTINDEX);

	if (parent_idx == NO_PARENT) return FALSE;

	if (unet_neighbourhood[parent_idx].NeighborStatus.bits.Symmetric){
		return TRUE;
	}else{
		return FALSE;
	}
}
/*--------------------------------------------------------------------------------------------*/
void link_parent_switch(void)
{

	uint8_t _thisNodeDepth = node_data_get(NODE_DISTANCE);
	uint8_t i =0;
	// !!!! A meu ver n�o precisa pegar o id do parent
	// Vai ser substituido mesmo
	// R: � para o caso de n�o ser subst�tuido (i.e n�o entrar no 'if'),
	// a� mant�m o anterior. Idem p/ nodedepth
	uint8_t parent_idx = node_data_get(NODE_PARENTINDEX);

	if (parent_idx != NO_PARENT){
		if (unet_neighbourhood[parent_idx].NeighborDistance == NODE_DISTANCE_INIT){
			node_data_set(NODE_DISTANCE, NODE_DISTANCE_INIT);
			node_data_set(NODE_PARENTINDEX, NO_PARENT);
			parent_idx = NO_PARENT;
			_thisNodeDepth = NODE_DISTANCE_INIT;
		}
	}

	// Varre a lista de vizinhos
	for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	{
	  // Verifica se existe vizinhos com profundidade menor
	  if (unet_neighbourhood[i].NeighborDistance < _thisNodeDepth)
	  {
		  // Verifica se o nodo com menor profundidade � sim�trico
		  if(is_symmetric_neighbor(i))
		  {
			  if (_thisNodeDepth == (unet_neighbourhood[i].NeighborDistance + 1))
			  {
				  // Mesma profundidade, verificando o RSSI do nodo pai
				  if (ParentNeighborID == unet_neighbourhood[i].Addr_16b){
					  // Atualiza o RSSI do nodo pai
					  ParentRSSI = unet_neighbourhood[i].NeighborRSSI;
					  parent_idx=i;
				  }else{
					  if (unet_neighbourhood[i].NeighborRSSI > (ParentRSSI+5)){
						  // Troca nodo pai para um nodo de melhor enlace
						  ParentNeighborID = unet_neighbourhood[i].Addr_16b;
						  ParentRSSI = unet_neighbourhood[i].NeighborRSSI;
						  parent_idx=i;
					  }
				  }
			  }else{
				  // Reduziu a profundidade atraves do vizinho escolhido
				  _thisNodeDepth = unet_neighbourhood[i].NeighborDistance + 1;
				  ParentNeighborID = unet_neighbourhood[i].Addr_16b;
				  ParentRSSI = unet_neighbourhood[i].NeighborRSSI;
				  parent_idx=i;
			  }
		  }
	  }
	}

	node_data_set(NODE_DISTANCE, _thisNodeDepth);
	if (parent_idx != node_data_get(NODE_PARENTINDEX)){

		//// TODO here is updated the parent of the node
#ifdef COOJA_H_
		// Make link between node and parent
		// TODO probably should clear the last link
		PRINTF("#L %d 0;red\n",unet_neighbourhood[node_data_get(NODE_PARENTINDEX)].Addr_16b); // Clear last parent
		PRINTF("#L %d 1;red\n",unet_neighbourhood[parent_idx].Addr_16b);	// Setup the new parent
#endif

		node_data_set(NODE_PARENTINDEX, parent_idx);

		// Reduce the ping time in order to propagate the new info
        extern BRTOS_Sem* Link_Packet_TX_Event;
        if(Link_Packet_TX_Event != NULL)
        {
        	OSSemPost(Link_Packet_TX_Event);
        }
	}

    /* copy 64-bit pan id address */
   // node_pan_id64_set(&p->packet[PANID_64]);
}
/*--------------------------------------------------------------------------------------------*/

void link_parent_update(uint8_t idx){
	node_data_set(NODE_DISTANCE, (uint8_t)(unet_neighbourhood[idx].NeighborDistance + 1));
	node_data_set(NODE_PARENTINDEX, idx);
}

void link_node_distance_update(uint8_t i)
#if EXPERIMENTAL
{

		if(is_neighbor_parent(i))
		{
			if(is_symmetric_neighbor(i) && unet_neighbourhood[i].NeighborDistance < node_data_get(NODE_DISTANCE))
			{
				/* If it is the parent and remains symmetrical, update the depth of the node based on parent's depth */
				node_data_set(NODE_DISTANCE, unet_neighbourhood[i].NeighborDistance + 1);
			}else
			{
				/* parent symmetry lost or increased distance requires a full table search for a new parent selection */
				link_parent_switch();
				return;
			}
		}

		/* it is a new parent candidate ? */
		if(is_symmetric_neighbor(i))
		{
			uint8_t p_idx = node_data_get(NODE_PARENTINDEX);

			if((p_idx == NO_PARENT) || (node_data_get(NODE_DISTANCE) >= ROUTE_TO_BASESTATION_LOST))
			{
				if(unet_neighbourhood[i].NeighborDistance < ROUTE_TO_BASESTATION_LOST)
				{
					/* If the node has no parent or route to the coordinator and,
					 * if the neighbor sending this link message has a path to the
					 * coordinator, choose such node as parent */
					link_parent_update(i);
				}
			}
			else
			{
				if(unet_neighbourhood[i].NeighborDistance < unet_neighbourhood[p_idx].NeighborDistance)
				{
					if((unet_neighbourhood[i].NeighborRSSI >= unet_neighbourhood[p_idx].NeighborRSSI) &&
						(unet_neighbourhood[i].NeighborMyRSSI >= unet_neighbourhood[p_idx].NeighborMyRSSI))
					{
						/* If the node has a parent, but the neighbor sending this
						* link message has a shorter path to the coordinator and
						* better link, choose such node as parent */
						link_parent_update(i);
					}else
					{
						// !!!!!!!!!!!!!!!!!!!! Esse segundo teste n�o parece fazer sentido,
						// pois se o n� � sim�trico, esse teste j� foi feito
						// A menos que simetria possa ser somente existir na tarefa do vizinho
						// R: o teste deixado, pois futuramente a id�ia � ter dois valores configur�veis
						// de threshold, um RSSI_THRESHOLD para simetria (que pode inclusive ser 0) e outro
						// PARENT_RSSI_THRESHOLD que pode ser maior que RSSI_THRESHOLD.
						// Ent�o, o teste passar� a ser com PARENT_RSSI_THRESHOLD. A� pode ser
						// simetrico, mas n�o ter RSSI suficiente para 'parent'.
						// Obs.: foi corrigido o teste para ficar mais clara a inten��o

						#define PARENT_RSSI_THRESHOLD RSSI_THRESHOLD
						if((unet_neighbourhood[i].NeighborRSSI > PARENT_RSSI_THRESHOLD) &&
								(unet_neighbourhood[i].NeighborMyRSSI > PARENT_RSSI_THRESHOLD))
						{
							/* If the node has a parent, but the neighbor sending this
							* link message has a shorter path to the coordinator and
							* a link quality above the threshold, choose such node as parent */
							link_parent_update(i);
						}
					}
				}
				else
				{
					if(unet_neighbourhood[i].NeighborDistance == unet_neighbourhood[p_idx].NeighborDistance)
					{
						if((unet_neighbourhood[i].NeighborRSSI > unet_neighbourhood[p_idx].NeighborRSSI) &&
								(unet_neighbourhood[i].NeighborMyRSSI > unet_neighbourhood[p_idx].NeighborMyRSSI))
						{
							/* If the node has a parent, but the neighbor sending this
							* link message has a equal path to the coordinator, but
							* better link, choose such node as parent */
							link_parent_update(i);
						}
					}else
					{
						if(unet_neighbourhood[i].NeighborDistance == node_data_get(NODE_DISTANCE))
						{
							// !!!!!! De novo n�o faz sentido, pois se o parent � sim�trico, sei RSSI
							// !!!!!! j� � acima do threshold
							// R: mesma da anterior

							if(unet_neighbourhood[p_idx].NeighborRSSI <= PARENT_RSSI_THRESHOLD ||
									unet_neighbourhood[p_idx].NeighborMyRSSI <= PARENT_RSSI_THRESHOLD)
							{

								if(unet_neighbourhood[i].NeighborRSSI > unet_neighbourhood[p_idx].NeighborRSSI &&
									unet_neighbourhood[i].NeighborMyRSSI > unet_neighbourhood[p_idx].NeighborMyRSSI)
								{
			#if 0
									if((unet_neighbourhood[i].ParentAddr16b != unet_neighbourhood[p_idx].Addr_16b)
											&& (unet_neighbourhood[i].ParentAddr16b != node_data_get_16b(NODE_ADDR16))
											&& (unet_neighbourhood[i].ParentRSSI > unet_neighbourhood[p_idx].NeighborRSSI)
											&& (unet_neighbourhood[i].ParentMyRSSI > unet_neighbourhood[p_idx].NeighborMyRSSI))
									{
										link_parent_update(i);
									}
			#endif
								}
							}
						}

					}

				}

			}
		}

}
#else
{
	(void)i;
	link_parent_switch();
}
#endif
/*--------------------------------------------------------------------------------------------*/
void link_packet_process(packet_t *p)
{
    uint8_t i = 0;

    i = link_neighbor_table_find(p);

    if ( i < NEIGHBOURHOOD_SIZE)
    {
		if(unet_neighbourhood[i].NeighborRSSI < p->info[PKTINFO_RSSI])
		{
	    	unet_neighbourhood[i].NeighborRSSI = p->info[PKTINFO_RSSI];
		}else
		{
	    	unet_neighbourhood[i].NeighborRSSI = (uint8_t)(((unet_neighbourhood[i].NeighborRSSI * 7) + p->info[PKTINFO_RSSI])>> 3);
		}

		unet_neighbourhood[i].NeighborLQI  = p->info[PKTINFO_LQI];
        unet_neighbourhood[i].NeighborDistance = p->packet[UNET_CTRL_MSG_DISTANCE];
        unet_neighbourhood[i].NeighborStatus.bits.Active = 1;

        /* removido, pois o SN s� � armazenado p/ detectar duplicatas de pacotes em roteamento */
        //unet_neighbourhood[i].NeighborLastID = p->info[PKTINFO_SEQNUM];

        REQUIRE_OR_EXIT((p->packet[UNET_CTRL_MSG_NB_COUNT] < NEIGHBOURHOOD_SIZE), exit_on_require_error);

        link_neighbor_symmetry_update(p,i);
        NeighborTable = (neighbor_table_t)(NeighborTable | (neighbor_table_t)(0x01 << i));

        if(p->packet[UNET_CTRL_MSG_PACKET_TYPE] == NEIGHBOR_ADV)
        {
        	link_node_distance_update(i);
        }else
        {
        	if(p->packet[UNET_CTRL_MSG_PACKET_TYPE] == NEIGHBOR_ADV_RESET)
        	{
        		link_node_distance_update(i);
        		extern BRTOS_Sem* Link_Packet_TX_Event;
        		OSSemPost(Link_Packet_TX_Event);
        		return; /* now schedule a link packet TX */
        	}

        	if(p->packet[UNET_CTRL_MSG_PACKET_TYPE] == NEIGHBOR_REQ)
        	{
				#if UNET_DEVICE_TYPE != PAN_COORDINATOR
        		// If the parent reseted, lost the kinship
        		if (ParentNeighborID == unet_neighbourhood[i].Addr_16b){
            		node_data_set(NODE_DISTANCE, NODE_DISTANCE_INIT);
            		node_data_set(NODE_PARENTINDEX, NO_PARENT);
            		ParentNeighborID = 0xFFFE;
            		ParentRSSI = 0;
            		// If lost the parent due to parent reset, reset the neighbor
            		// table in order to not associate with a former child
            		link_neighbor_table_init();
            		unet_router_up_table_clear();
        		}
				#endif

				#if UNET_DEVICE_TYPE != PAN_COORDINATOR
        		if (node_data_get(NODE_PARENTINDEX) != NO_PARENT){
				#endif
        			extern BRTOS_Sem* Link_Packet_TX_Event;
        			if(Link_Packet_TX_Event != NULL)
        			{
        				OSSemPost(Link_Packet_TX_Event);
        			}
				#if UNET_DEVICE_TYPE != PAN_COORDINATOR
        		}
				#endif
        		return; /* now schedule a link packet TX */
        	}
        }
    }
    exit_on_require_error:
	return;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t link_neighbor_table_find(packet_t *p)
{
    uint8_t i = 0;
    uint8_t j = 0;

    uint16_t nb_addr16 =((p->packet[UNET_CTRL_MSG_ADDR16H] << 8) +
    		p->packet[UNET_CTRL_MSG_ADDR16L]);

	i = NEIGHBOURHOOD_SIZE;

	for(j=0;j<NEIGHBOURHOOD_SIZE;j++)
	{
	  if (unet_neighbourhood[j].Addr_16b == nb_addr16)
	  {
		i = j;
		break;
	  }
	}

	/* try to add, if new neighbor */
	if (i == NEIGHBOURHOOD_SIZE)
	{
		for(j=0;j<NEIGHBOURHOOD_SIZE;j++)
		{
		  if (unet_neighbourhood[j].Addr_16b == UNUSED_ADDR)
		  {
			unet_neighbourhood[j].Addr_16b = nb_addr16;
			i = j;

			/* set RSSI and 64-bit src address */
			unet_neighbourhood[i].NeighborRSSI =  p->info[PKTINFO_RSSI];
			unet_neighbourhood[i].NeighborMyRSSI = 0;

			/* increase link packet rate to update topology */
			ATOMIC_SET(NeighborLinkPacketTimeCnt,1);
			break;
		  }
		}
	}

	return i;
}
/*--------------------------------------------------------------------------------------------*/
void link_set_neighbor_activity(packet_t *p)
{
    uint8_t i = 0;
    uint8_t j = 0;
	uint16_t addr = (p->info[PKTINFO_SRC16H] << 8) + p->info[PKTINFO_SRC16L];

	i = NEIGHBOURHOOD_SIZE;

	for(j=0;j<NEIGHBOURHOOD_SIZE;j++)
	{
	  if (unet_neighbourhood[j].Addr_16b == addr)
	  {
		i = j;
		if(unet_neighbourhood[i].NeighborRSSI < p->info[PKTINFO_RSSI]){
	    	unet_neighbourhood[i].NeighborRSSI = p->info[PKTINFO_RSSI];
		}else{
	    	unet_neighbourhood[i].NeighborRSSI = (uint8_t)(((unet_neighbourhood[i].NeighborRSSI * 7) + p->info[PKTINFO_RSSI])>> 3);
		}
        unet_neighbourhood[i].NeighborLQI  = p->info[PKTINFO_LQI];
        unet_neighbourhood[i].NeighborStatus.bits.Active = 1;

        //unet_neighbourhood[i].NeighborLastID  = p->info[PKTINFO_SEQNUM];

        NeighborTable = (neighbor_table_t)(NeighborTable | (neighbor_table_t)(0x01 << i));
		break;
	  }
	}
}

/*--------------------------------------------------------------------------------------------*/
void link_set_neighbor_seqnum(packet_t *p)
{
    uint8_t i = 0;
    uint8_t j = 0;
	uint16_t addr = (p->info[PKTINFO_SRC16H] << 8) + p->info[PKTINFO_SRC16L];

	i = NEIGHBOURHOOD_SIZE;

	for(j=0;j<NEIGHBOURHOOD_SIZE;j++)
	{
	  if (unet_neighbourhood[j].Addr_16b == addr)
	  {
		i = j;
        unet_neighbourhood[i].NeighborLastID  = p->info[PKTINFO_SEQNUM];
		break;
	  }
	}
}
/*--------------------------------------------------------------------------------------------*/
void link_verify_neighbourhood(void)
{
      uint8_t i = 0;
	  #if (UNET_DEVICE_TYPE == ROUTER)
      uint8_t reset_neighbor_table = FALSE;
	  #endif

      // Verify if there are inactive neighbors
      for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
      {
		#if (UNET_DEVICE_TYPE == ROUTER)
    	// Verify if the parent node is symmetric
    	if (unet_neighbourhood[i].Addr_16b != 0xFFFE){
			if (ParentNeighborID == unet_neighbourhood[i].Addr_16b)
			{
				if(is_symmetric_neighbor(i) == FALSE){
					node_data_set(NODE_DISTANCE, NODE_DISTANCE_INIT);
					node_data_set(NODE_PARENTINDEX, NO_PARENT);
					ParentNeighborID = 0xFFFE;
					ParentRSSI = 0;
				}
			}
    	}
		#endif

    	// If the neighbor is inactive for a while
        if ((NeighborTable & (0x01 << i)) == 0)
        {
          // Remove from the neighborhood if it is a former active node
          if (unet_neighbourhood[i].Addr_16b != 0xFFFE)
          {
            // Update Base Station Tables
			#if (UNET_DEVICE_TYPE == ROUTER)
        	if (ParentNeighborID == unet_neighbourhood[i].Addr_16b)
        	{
        		node_data_set(NODE_DISTANCE, NODE_DISTANCE_INIT);
        		node_data_set(NODE_PARENTINDEX, NO_PARENT);
        		ParentNeighborID = 0xFFFE;
        		ParentRSSI = 0;
        		// If lost the parent due to inactivity, reset the neighbor
        		// table in order to not associate with a former child
        		reset_neighbor_table = TRUE;
        	}
			#endif

            // Delete Neighbor information
            unet_neighbourhood[i].Addr_16b            			= 0xFFFE;
            unet_neighbourhood[i].NeighborRSSI        			= 0;
            unet_neighbourhood[i].NeighborLastID      			= 0;
            unet_neighbourhood[i].NeighborMyRSSI        		= 0;
            unet_neighbourhood[i].NeighborDistance       		= NO_ROUTE_TO_BASESTATION;
            unet_neighbourhood[i].NeighborStatus.bits.Symmetric = FALSE;
            unet_neighbourhood[i].NeighborStatus.bits.Active 	= 0;

            // Reduce the ping time in order to propagate the new info
            extern BRTOS_Sem* Link_Packet_TX_Event;
            OSSemPost(Link_Packet_TX_Event);
          }
        }
      }

	  #if (UNET_DEVICE_TYPE == ROUTER)
      if (node_data_get(NODE_DISTANCE) == NODE_DISTANCE_INIT)
      {
    	  if (reset_neighbor_table == TRUE){
    		  link_neighbor_table_init();
    		  unet_router_up_table_clear();
    	  }else{
    		  // Update this node parent and depth
    		  link_parent_switch();
    	  }
      }
	  #endif

      // Clear table to refresh neighbors activity
      NeighborTable = 0;
}
/*--------------------------------------------------------------------------------------------*/
uint16_t link_neighbor_table_addr16_get(uint8_t idx)
{
	REQUIRE_FOREVER(idx < NEIGHBOURHOOD_SIZE);
	return unet_neighbourhood[idx].Addr_16b;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t link_neighbor_table_count(void)
{

	   uint8_t i;
	   uint8_t n=0;
	   for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	   {
		   if(unet_neighbourhood[i].Addr_16b  != UNUSED_ADDR)
		   {
			   n++;
		   }
	   }
	   node_data_set(NODE_NEIGHBOR_COUNT, n);
	   return n;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t link_packet_is_duplicated(uint8_t *psrc_addr, uint8_t seq_num)
{
	uint8_t i = 0;
	uint16_t src_addr = (uint16_t)(psrc_addr[0] + (psrc_addr[1] << 8));

    for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
    {
        if (unet_neighbourhood[i].Addr_16b == src_addr)
        {
          if (unet_neighbourhood[i].NeighborLastID == seq_num)
          {
            return TRUE;
          }else
          {
            //unet_neighbourhood[i].NeighborLastID = seq_num;
            return FALSE;
          }
        }
    }
    return FALSE;
}
/*--------------------------------------------------------------------------------------------*/
void link_seqnum_reset(uint16_t src_addr)
{

	uint8_t i = 0;
	for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	{
		if (unet_neighbourhood[i].Addr_16b == src_addr)
		{
            unet_neighbourhood[i].NeighborLastID = 0;
		}
	}
}
/*--------------------------------------------------------------------------------------------*/
