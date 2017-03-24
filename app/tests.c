/*
 * tests.c
 *
 */

//#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "stdio.h"
#include "node.h"
#include "ieee802154.h"
#include "link.h"
#include "unet_router.h"

int tests_run = 0;
void all_tests(void);

/* config TEST_ASSERT macro */
#if WINNT
#define TEST_ASSERT(x)      assert(x)
#else
#ifdef assert
#define TEST_ASSERT(x) assert(x)
#elif
#define TEST_ASSERT(x)		if(!(x)) while(1){}
#endif
#endif

#define run_test(test) do { test(); PRINTF("tests[%d]: %d OK\r\n",__LINE__, tests_run); /*fflush(stdout)*/; tests_run++; } while (0)

#if (PROCESSOR == X86)
void __cdecl _assert (const char *_Message, const char *_File, unsigned _Line)
{
 	PRINTF("Test failed at line %d\r\n", _Line);
 	fflush(stdout);
 	fflush(stderr);

 	exit(0);
}
#endif

#if (PROCESSOR == ARM_Cortex_M0)
void
__attribute__((noreturn))
__assert_func (const char *file, int line, const char *func,
               const char *failedexpr)
{
	PRINTF("Test failed at line %d\r\n", line);
	exit(0);
}
#endif

inline uint32_t atomic_inc(uint32_t * memory)
{

  uint32_t temp1, temp2;

#if (PROCESSOR == ARM_Cortex_M0)
  __asm__ __volatile__ (
    "1:\n"
    "\tldrex\t%[t1],[%[m]]\n"
    "\tadd\t%[t1],%[t1],#1\n"
    "\tstrex\t%[t2],%[t1],[%[m]]\n"
    "\tcmp\t%[t2],#0\n"
    "\tbne\t1b"
    : [t1] "=&r" (temp1), [t2] "=&r" (temp2)
    : [m] "r" (memory)
  );
#endif
  return temp1;
}


extern volatile unet_neighborhood_t  unet_neighbourhood[];

//uint32_t lock = 0;

void task_run_tests(void)
{
	for(;;)
	{
		 //atomic_inc(&lock);
		 all_tests();

		 PRINTF("ALL TESTS PASSED\n");
		 PRINTF("Tests run: %d\n", tests_run);

		 exit(0);
		 return;
	}

}

void test_link_packet_create_for_association_request(void)
{
	addr64_t pan_id_64_test;
	addr16_t pan_id_16_test;

	packet_t * p;

	memset(pan_id_64_test.u8,0xff,8);
	memset(pan_id_16_test.u8,0xff,2);

	const addr16_t node_addr16 = {.u8 = {0x12, 0x34}};
	const addr64_t node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x34}};

	node_addr64_set((uint8_t*)&node_addr64);
	node_data_set(NODE_ADDR16H,node_addr16.u8[0]);
	node_data_set(NODE_ADDR16L,node_addr16.u8[1]);

    p = link_packet_create_association_request();

    TEST_ASSERT(memcmp(&(p->packet[UNET_SRC_64]),&node_addr64.u8,8) == 0);

	TEST_ASSERT((p->packet[MAC_SRC_16] == node_addr16.u8[1]) && (p->packet[MAC_SRC_16+1] == node_addr16.u8[0]) );
    TEST_ASSERT(memcmp(&(p->packet[MAC_DEST_16]),&pan_id_16_test.u8,2) == 0);

	TEST_ASSERT(memcmp(&(p->packet[UNET_DEST_64]),&pan_id_64_test.u8,8) == 0 );

	TEST_ASSERT((p->packet[MAC_PANID_16] == pan_id_16_test.u8[1]) && (p->packet[MAC_PANID_16+1] == pan_id_16_test.u8[0]) );

	TEST_ASSERT(p->packet[UNET_HOP_LIMIT]==1);
	TEST_ASSERT(p->packet[UNET_PKT_TYPE]==BROADCAST_LOCAL_LINK);
	TEST_ASSERT(p->packet[UNET_CTRL_MSG_PACKET_TYPE]==NEIGHBOR_REQ);

	TEST_ASSERT(p->packet[MAC_FRAME_CTRL]==0x41);
	TEST_ASSERT(p->packet[MAC_FRAME_CTRL+1]==0x88);

	packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

	return;

}

void link_neighbor_table_add_one_for_test(uint8_t i)
{
	  REQUIRE_FOREVER(i < NEIGHBOURHOOD_SIZE);
      unet_neighbourhood[i].Addr_16b            			= (0xbb00)+i;
      unet_neighbourhood[i].NeighborRSSI        			= 100+i;
      unet_neighbourhood[i].NeighborLastID      			= 2+i;
      unet_neighbourhood[i].NeighborMyRSSI           		= 50+i;
      unet_neighbourhood[i].NeighborDistance       			= (node_data_get(NODE_DISTANCE) == 0) ? 1 :
    		  	  	  	  	  	  	  	  	  	  	  	  	  (node_data_get(NODE_DISTANCE) + (((i-NEIGHBOURHOOD_SIZE/2) > 0)? 1:(-1)));
      unet_neighbourhood[i].NeighborStatus.bits.Symmetric 	= TRUE;
      //memset((void*)&unet_neighbourhood[i].Addr_64b, 0xa0 + i, 8);
}

void link_neighbor_table_add_all_for_test(void)
{
	   uint8_t i;

	   for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	   {
		   link_neighbor_table_add_one_for_test(i);
	   }
}


void test_link_packet_create_for_one_panid(uint8_t n)
{
		packet_t * p;
		uint8_t k = n;

		link_neighbor_table_init();

		while(k > 0)
		{
			k--;
			link_neighbor_table_add_one_for_test(k);
		}

		const addr16_t node_addr16 = {.u8 = {0x12, 0x34}};
		const addr64_t node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x34}};
		const uint8_t pan_id_64[8] = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x34};
		const uint8_t pan_id_16[2] = {0x47,0x42};

		node_pan_id64_set((uint8_t*)pan_id_64);
		node_addr64_set((uint8_t*)&node_addr64);

		node_data_set(NODE_ADDR16H,node_addr16.u8[0]);
		node_data_set(NODE_ADDR16L,node_addr16.u8[1]);
		node_data_set(NODE_PANID16H,pan_id_16[0]);
		node_data_set(NODE_PANID16L,pan_id_16[1]);
		node_data_set(NODE_DISTANCE, 0);
		node_data_set(NODE_PARENTINDEX, NO_PARENT);

		link_packet_create();
		p = link_packet_get();

		TEST_ASSERT(memcmp(&p->packet[UNET_DEST_64],&pan_id_64,8) == 0 );
		TEST_ASSERT(memcmp(&p->packet[UNET_SRC_64],&node_addr64.u8,8) == 0);
		TEST_ASSERT((p->packet[MAC_SRC_16] == node_addr16.u8[1]) && (p->packet[MAC_SRC_16+1] == node_addr16.u8[0]) );
		TEST_ASSERT((p->packet[MAC_PANID_16] == pan_id_16[1]) && (p->packet[MAC_PANID_16+1] == pan_id_16[0]) );
		TEST_ASSERT(p->packet[UNET_HOP_LIMIT]==1);

		TEST_ASSERT(p->packet[UNET_PKT_TYPE]==BROADCAST_LOCAL_LINK);
		TEST_ASSERT(p->packet[UNET_CTRL_MSG_PACKET_TYPE]==NEIGHBOR_ADV_RESET);
		TEST_ASSERT(p->packet[UNET_CTRL_MSG_NB_COUNT] == n);

		k=n;
		while(k>0)
		{
			k--;
			TEST_ASSERT(p->packet[UNET_CTRL_MSG_NB_START+0+3*k] == unet_neighbourhood[k].Addr_16b>>8);
			TEST_ASSERT(p->packet[UNET_CTRL_MSG_NB_START+1+3*k] == (uint8_t)(unet_neighbourhood[k].Addr_16b));
			TEST_ASSERT(p->packet[UNET_CTRL_MSG_NB_START+2+3*k] == unet_neighbourhood[k].NeighborRSSI);
		}
		TEST_ASSERT(ieee802154_packet_length_get(p->packet) == p->info[PKTINFO_SIZE]);

		packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

		TEST_ASSERT(p->packet[MAC_FRAME_CTRL]==0x41);
		TEST_ASSERT(p->packet[MAC_FRAME_CTRL+1]==0x88);
}

void test_link_packet_create_for_one_panid_table_empty(void)
{
	test_link_packet_create_for_one_panid(0);
}

void test_link_packet_create_for_one_panid_table_one_neighbor(void)
{
	test_link_packet_create_for_one_panid(1);
}

void test_link_packet_create_for_one_panid_table_full(void)
{
	test_link_packet_create_for_one_panid(NEIGHBOURHOOD_SIZE);
}

/**
=================================== radio RX task ==========================================================
priority: 10

waits "Radio RX" event
Radio RX -> copy packet to buffer -> ieee802154_process -> llc_process -> network_process packet type -> case

	BROADCAST_LOCAL_LINK -> NEIGHBOR_ADV -> copy to link_local packet buffer -> post uNET local_link RX event -> return NO_ACK ->  end
	 	 	 	 	 	 -> NEIGHBOR_REQ -> copy to link_local packet buffer -> schedule broadcast local link (TX local link event) -> return NO_ACK -> end
	UNICAST_UP -> check buffer UP is empty -> YES: copy to packet buffer UP -> post uNET "route UP" event -> return ACK | NO: discards -> return NO_ACK -> end
	UNICAST_DOWN -> check buffer DOWN is empty -> YES: copy to packet buffer DOWN -> ROUTER_ADV? YES: store/update route -> post uNET "route DOWN" event -> return ACK | NO: discards -> return NO_ACK -> end
	UNICAST_ACK_UP -> check buffer UP is full -> YES: post "ACK UP" event | NO: discards -> return NO_ACK -> end
	UNICAST_ACK_DOWN -> check buffer DOWN is full -> YES: post "ACK DOWN" event | NO: discards -> return NO_ACK -> end
	MULTICAST_UP -> check buffer UP is empty -> YES: copy to packet buffer UP -> post uNET "route multicast UP" event -> return ACK | NO: discards -> return NO_ACK -> end
	MULTICAST_ACK_UP -> check buffer UP is full -> YES: post "MULTITASK ACK UP" event | NO: discards -> return NO_ACK -> end
*/

packet_t * link_create_packet_association_reply(uint8_t distance_of_replying_node)
{

	/* pan id should be 0xffff ? */
#if 0
	node_data_set(NODE_PANID16H,H(BROAD_ADDR));
	node_data_set(NODE_PANID16L,L(BROAD_ADDR));
#endif

	node_data_set(NODE_DISTANCE, distance_of_replying_node);

	link_packet_create();
	return link_packet_get();

}

void test_link_packet_reception_of_association_request(void)
{

	packet_t rx_pkt;
	packet_t *p;
	uint16_t fcs = 0;

	/* new node sends a request */
	link_neighbor_table_init();
	p = link_packet_create_association_request();

	/* receiving node has one neighbor */
	link_neighbor_table_add_one_for_test(0);

	memset(p->info, 0, PKTINFO_MAX_OPT);
	memset(rx_pkt.packet, 0, MAX_PACKET_SIZE);

	/* simulate packet rx */
	memcpy(rx_pkt.packet, p->packet, p->packet[PHY_PKT_SIZE]+1);

	rx_pkt.info[PKTINFO_RSSI] = 100;
	rx_pkt.info[PKTINFO_LQI] = 100;

	fcs = ieee802154_fcs_calc(&rx_pkt.packet[MAC_FRAME_CTRL], rx_pkt.packet[PHY_PKT_SIZE]);

	/* add fcs field */
	rx_pkt.info[PKTINFO_FCSH] = H(fcs);
	rx_pkt.info[PKTINFO_FCSL] = L(fcs);

	UNET_RADIO.set(CRC,1);

	packet_print(rx_pkt.packet,rx_pkt.packet[PHY_PKT_SIZE]+2);
	packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

	TEST_ASSERT(rx_pkt.packet[UNET_CTRL_MSG_PACKET_TYPE] == NEIGHBOR_REQ);

	UNET_RADIO.set(CRC,1);
	if(ieee802154_packet_input(&rx_pkt) == ACK_REQ_TRUE)
	{
		PRINTF("Packet in buffer - link layer ack required: \r\n");
		packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);
	}

}

void test_link_packet_reception_of_association_reply_from_node_with_distance(uint8_t node_dist)
{
		packet_t pkt_req;
		packet_t pkt_rep;
		packet_t *p;
		packet_t *q;
		uint16_t fcs = 0;

		uint16_t new_node = 0x1122;
		uint16_t parent_node = 0x4455;

		const addr64_t new_node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x11,0x22}};

		/* new node sends a request */
		link_neighbor_table_init();
		node_addr64_set((uint8_t*)&new_node_addr64);
		node_data_set(NODE_ADDR16H,H(new_node));
		node_data_set(NODE_ADDR16L,L(new_node));
		q = link_packet_create_association_request();

		/* receiving node has one neighbor */
		link_neighbor_table_init();
		link_neighbor_table_add_one_for_test(0);

		memset(q->info, 0, PKTINFO_MAX_OPT);
		memset(pkt_req.packet, 0, MAX_PACKET_SIZE);

		/* simulate packet rx */
		memcpy(pkt_req.packet, q->packet, q->packet[PHY_PKT_SIZE]+1);

		pkt_req.info[PKTINFO_RSSI] = 100;
		pkt_req.info[PKTINFO_LQI] = 100;

		fcs = ieee802154_fcs_calc(&pkt_req.packet[MAC_FRAME_CTRL], pkt_req.packet[PHY_PKT_SIZE]);

		/* add fcs field */
		pkt_req.info[PKTINFO_FCSH] = H(fcs);
		pkt_req.info[PKTINFO_FCSL] = L(fcs);

		//packet_print(rx_pkt.packet,rx_pkt.packet[PKT_SIZE]+2);
		//packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

		/* set parent node id16, panid16, panid64, addr64 */

		const addr64_t node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x35}};
		const uint8_t pan_id_64[8] = {0x00,0x00,0x12,0x34,0x00,0x00,0x47,0x42};
		const uint8_t pan_id_16[2] = {0x47,0x42};

		node_data_set(NODE_PANID16H,pan_id_16[0]);
		node_data_set(NODE_PANID16L,pan_id_16[1]);
		node_data_set(NODE_ADDR16H,H(parent_node));
		node_data_set(NODE_ADDR16L,L(parent_node));
		node_pan_id64_set((uint8_t*)pan_id_64);
		node_addr64_set((uint8_t*)&node_addr64);

		UNET_RADIO.set(CRC,1);
		TEST_ASSERT(ieee802154_packet_input(&pkt_req) == ACK_REQ_FALSE);

		/* possible parent node creates an association reply
		 * with its new neighbor added */
		p = link_create_packet_association_reply(node_dist);

		/* current (target) node has no neighbor, and is not associated to any pan id */
		node_pan_id64_init();
		link_neighbor_table_init();

		node_addr64_set((uint8_t*)&new_node_addr64);
		node_data_set(NODE_ADDR16H,H(new_node));
		node_data_set(NODE_ADDR16L,L(new_node));

		node_data_set(NODE_PANID16H,H(BROAD_ADDR));
		node_data_set(NODE_PANID16L,L(BROAD_ADDR));
		node_data_set(NODE_DISTANCE,NO_ROUTE_TO_BASESTATION);

		memset(p->info, 0, PKTINFO_MAX_OPT);
		memset(pkt_rep.packet, 0, MAX_PACKET_SIZE);

		/* simulate packet rx */
		memcpy(pkt_rep.packet, p->packet, p->packet[PHY_PKT_SIZE]+2);

		pkt_rep.info[PKTINFO_RSSI] = 100;
		pkt_rep.info[PKTINFO_LQI] = 100;

		fcs = ieee802154_fcs_calc(&pkt_rep.packet[MAC_FRAME_CTRL], pkt_rep.packet[PHY_PKT_SIZE]);

		/* add fcs field */
		pkt_rep.info[PKTINFO_FCSH] = H(fcs);
		pkt_rep.info[PKTINFO_FCSL] = L(fcs);

		packet_print(pkt_rep.packet,pkt_rep.packet[PHY_PKT_SIZE]+2);
		//packet_print(p->packet,ieee802154_packet_length_get(p->packet));


		UNET_RADIO.set(CRC,1);
		if(ieee802154_packet_input(&pkt_rep) == ACK_REQ_TRUE)
		{
			PRINTF("Packet in buffer - link layer ack required\r\n");
			packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);
		}

		TEST_ASSERT(pkt_rep.packet[UNET_CTRL_MSG_PACKET_TYPE] == NEIGHBOR_ADV);
		TEST_ASSERT(link_neighbor_table_count()>0);
		TEST_ASSERT(pkt_rep.packet[UNET_CTRL_MSG_DISTANCE] == node_dist); /* test reply from root node */
		TEST_ASSERT(node_data_get(NODE_DISTANCE) == (node_dist + 1)); /* test correct node distance */
}

void test_link_packet_reception_of_association_reply_from_root_node(void)
{

	test_link_packet_reception_of_association_reply_from_node_with_distance(0);
}

void test_link_packet_reception_of_association_reply_from_node_with_distance_2(void)
{
	test_link_packet_reception_of_association_reply_from_node_with_distance(2);
}

void network_packet_create_for_router_adv_for_test(void)
{
	const uint8_t pan_id_64[8] = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x34};
	const uint8_t pan_id_16[2] = {0x47,0x42};
	extern packet_t packet_down;

	node_pan_id64_set((uint8_t*)pan_id_64);

	node_data_set(NODE_PANID16H,pan_id_16[0]);
	node_data_set(NODE_PANID16L,pan_id_16[1]);
	node_data_set(NODE_DISTANCE, 0);

	link_neighbor_table_init();
	link_neighbor_table_add_one_for_test(0); /* router adv node */
	link_neighbor_table_add_one_for_test(1); /* parent node */

	addr16_t node_addr16;
	addr64_t node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x00,0x00}};

	node_addr16.u8[0] = H(unet_neighbourhood[0].Addr_16b);
	node_addr16.u8[1] = L(unet_neighbourhood[0].Addr_16b);
	node_addr64.u8[6] = node_addr16.u8[0];
	node_addr64.u8[7] = node_addr16.u8[1];

	node_addr64_set((uint8_t*)&node_addr64);
	node_data_set(NODE_ADDR16H,node_addr16.u8[0]);
	node_data_set(NODE_ADDR16L,node_addr16.u8[1]);

	node_data_set(NODE_PARENTINDEX, 1);
	node_data_set(NODE_DISTANCE, unet_neighbourhood[1].NeighborDistance + 1);


	unet_router_adv();
	packet_t *p = &packet_down;

	packet_print(p->packet,p->packet[PHY_PKT_SIZE]+2);
	TEST_ASSERT(packet_down.packet[UNET_PKT_TYPE] == UNICAST_DOWN);  //// <-- error here
	TEST_ASSERT(packet_down.packet[UNET_PAYLOAD_LEN] == 1);
	TEST_ASSERT(packet_down.packet[UNET_NEXT_HEADER] == NEXT_HEADER_UNET_CTRL_MSG);
	TEST_ASSERT(packet_down.packet[UNET_CTRL_MSG_PACKET_TYPE] == ROUTER_ADV);
	//TEST_ASSERT(packet_down.packet[UNET_HOP_LIMIT] == node_data_get(NODE_DISTANCE));

	TEST_ASSERT(memcmp(&p->packet[UNET_DEST_64],&pan_id_64,8) == 0 );
	TEST_ASSERT(memcmp(&p->packet[UNET_SRC_64],&node_addr64.u8,8) == 0);


	TEST_ASSERT((p->packet[MAC_SRC_16] == node_addr16.u8[1]) && (p->packet[MAC_SRC_16+1] == node_addr16.u8[0]) );
	TEST_ASSERT((p->packet[MAC_PANID_16] == pan_id_16[1]) && (p->packet[MAC_PANID_16+1] == pan_id_16[0]) );

	addr16_t parent_node_addr16;
	uint8_t p_idx = node_data_get(NODE_PARENTINDEX);
	parent_node_addr16.u8[0] = H(unet_neighbourhood[p_idx].Addr_16b);
	parent_node_addr16.u8[1] = L(unet_neighbourhood[p_idx].Addr_16b);

	TEST_ASSERT(p->info[PKTINFO_DEST16H] == parent_node_addr16.u8[0]);
	TEST_ASSERT(p->info[PKTINFO_DEST16L] == parent_node_addr16.u8[1]);

	TEST_ASSERT((p->packet[MAC_DEST_16] == parent_node_addr16.u8[1]) && (p->packet[MAC_DEST_16+1] == parent_node_addr16.u8[0]) );

	ieee802154_frame_control_t fc;
	fc.u8[0] = p->packet[MAC_FRAME_CTRL];
	fc.u8[1] = p->packet[MAC_FRAME_CTRL+1];
	TEST_ASSERT(fc.bits.ACKRequest == TRUE);

}

void test_network_packet_create_for_router_adv(void)
{
	extern packet_t packet_down;
	packet_t *p = &packet_down;
	network_packet_create_for_router_adv_for_test();
	TEST_ASSERT(p->state == PACKET_WAITING_ACK);
	packet_release_down();
}


void test_network_packet_create_for_router_adv_ack(void)
{
	extern packet_t packet_down;
	packet_t  *p = &packet_down;
	packet_t  pkt_rx;
	packet_t  pkt_backup_buffer_origin;

	/* create a router adv packet for testing */
	network_packet_create_for_router_adv_for_test();
	TEST_ASSERT(p->state == PACKET_WAITING_ACK);

	memcpy(&pkt_backup_buffer_origin, p, sizeof(packet_t));
	packet_release_down();
	TEST_ASSERT(p->state == PACKET_IDLE);

	/* current receiving node is the parent node */
	node_data_set(NODE_ADDR16H,H(unet_neighbourhood[1].Addr_16b));
	node_data_set(NODE_ADDR16L,L(unet_neighbourhood[1].Addr_16b));

	memset(p->info, 0, PKTINFO_MAX_OPT);
	memset(pkt_rx.packet, 0, MAX_PACKET_SIZE);

	packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

	/* simulate packet rx */
	memcpy(pkt_rx.packet, p->packet, p->packet[PHY_PKT_SIZE]+2);

	pkt_rx.info[PKTINFO_RSSI] = 100;
	pkt_rx.info[PKTINFO_LQI] = 100;

	uint16_t fcs = ieee802154_fcs_calc(&pkt_rx.packet[MAC_FRAME_CTRL], pkt_rx.packet[PHY_PKT_SIZE]);

	/* add fcs field */
	pkt_rx.info[PKTINFO_FCSH] = H(fcs);
	pkt_rx.info[PKTINFO_FCSL] = L(fcs);

	/* packet rx input */

	UNET_RADIO.set(CRC,1);
	TEST_ASSERT(ieee802154_packet_input(&pkt_rx) == ACK_REQ_TRUE);

	PRINTF("Link layer ACK \r\n");
	packet_print(pkt_rx.packet,ieee802154_packet_length_get(pkt_rx.packet)+2);

	/* simulate link layer ack packet rx */
	fcs = ieee802154_fcs_calc(&pkt_rx.packet[MAC_FRAME_CTRL], pkt_rx.packet[PHY_PKT_SIZE]);

	/* add fcs field */
	pkt_rx.info[PKTINFO_FCSH] = H(fcs);
	pkt_rx.info[PKTINFO_FCSL] = L(fcs);

	/* restore source node address */
	node_data_set(NODE_ADDR16H,H(unet_neighbourhood[0].Addr_16b));
	node_data_set(NODE_ADDR16L,L(unet_neighbourhood[0].Addr_16b));

	/* restore packet buffer */
	memcpy(p,&pkt_backup_buffer_origin, sizeof(packet_t));
	TEST_ASSERT(p->state == PACKET_WAITING_ACK);

	/* packet rx input */

	UNET_RADIO.set(CRC,1);
	TEST_ASSERT(ieee802154_packet_input(&pkt_rx) == ACK_REQ_FALSE);
	//TEST_ASSERT(p->state == PACKET_IDLE);
}

void test_network_packet_create_for_router_adv_forwarding(void)
{
	extern packet_t packet_down;
	packet_t  *p = &packet_down;
	packet_t  pkt_rx;
	packet_t  pkt_backup_buffer_origin;

	/* create a router adv packet for testing */
	network_packet_create_for_router_adv_for_test();
	TEST_ASSERT(p->state == PACKET_WAITING_ACK);

	memcpy(&pkt_backup_buffer_origin, p, sizeof(packet_t));
	packet_release_down();
	TEST_ASSERT(p->state == PACKET_IDLE);

	/* current receiving node is the parent node */
	node_data_set(NODE_ADDR16H,H(unet_neighbourhood[1].Addr_16b));
	node_data_set(NODE_ADDR16L,L(unet_neighbourhood[1].Addr_16b));

	memset(p->info, 0, PKTINFO_MAX_OPT);
	memset(pkt_rx.packet, 0, MAX_PACKET_SIZE);

	packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

	/* simulate packet rx */
	memcpy(pkt_rx.packet, p->packet, p->packet[PHY_PKT_SIZE]+2);

	pkt_rx.info[PKTINFO_RSSI] = 100;
	pkt_rx.info[PKTINFO_LQI] = 100;

	uint16_t fcs = ieee802154_fcs_calc(&pkt_rx.packet[MAC_FRAME_CTRL], pkt_rx.packet[PHY_PKT_SIZE]);

	/* add fcs field */
	pkt_rx.info[PKTINFO_FCSH] = H(fcs);
	pkt_rx.info[PKTINFO_FCSL] = L(fcs);

	/* packet rx input */

	UNET_RADIO.set(CRC,1);
	TEST_ASSERT(ieee802154_packet_input(&pkt_rx) == ACK_REQ_TRUE);

	PRINTF("Link layer ACK \r\n");
	packet_print(pkt_rx.packet,ieee802154_packet_length_get(pkt_rx.packet)+2);

	/* simulate link layer ack packet rx */
	fcs = ieee802154_fcs_calc(&pkt_rx.packet[MAC_FRAME_CTRL], pkt_rx.packet[PHY_PKT_SIZE]);

	/* add fcs field */
	pkt_rx.info[PKTINFO_FCSH] = H(fcs);
	pkt_rx.info[PKTINFO_FCSL] = L(fcs);

	/* restore source node address */
	node_data_set(NODE_ADDR16H,H(unet_neighbourhood[0].Addr_16b));
	node_data_set(NODE_ADDR16L,L(unet_neighbourhood[0].Addr_16b));

	/* restore packet buffer */
	memcpy(p,&pkt_backup_buffer_origin, sizeof(packet_t));
	TEST_ASSERT(p->state == PACKET_WAITING_ACK);

	/* packet rx input */
	UNET_RADIO.set(CRC,1);
	TEST_ASSERT(ieee802154_packet_input(&pkt_rx) == ACK_REQ_FALSE);
	//TEST_ASSERT(p->state == PACKET_IDLE);

	TEST_ASSERT(unet_router_up_table_entry_find((addr64_t *)&p->packet[UNET_SRC_64]) == 0);


}

void test_app_packet_create_for_router_up(void)
{

		extern packet_t packet_up;
		packet_t  *p = &packet_up;
		uint8_t payload_len = 64;
		addr64_t dest_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x34}};

		/* create a app packet for testing router up */
		TEST_ASSERT(packet_acquire_up() == PACKET_ACCESS_ALLOWED);

		/* add fake payload */
		REQUIRE_FOREVER((UNET_APP_HEADER_START+payload_len) < PACKET_END);
		memset(&(p->packet[UNET_APP_HEADER_START]), 0xAA, payload_len);

		uint8_t res = unet_packet_up_sendto(&dest_addr64, payload_len);

		PRINTF("Sending packet ");
		if(res == RESULT_PACKET_SEND_OK)
		{
			PRINTF("OK\r\n");

		}else
		{
			PRINTF("failed due to reason %d \r\n", res);
		}

		TEST_ASSERT(res == RESULT_DESTINATION_UNREACHABLE);

		/* set source node */
		addr16_t node_addr16;
		addr64_t node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x00,0x00}};

		node_addr16.u8[0] = H(unet_neighbourhood[1].Addr_16b);
		node_addr16.u8[1] = L(unet_neighbourhood[1].Addr_16b);
		node_addr64.u8[6] = node_addr16.u8[0];
		node_addr64.u8[7] = node_addr16.u8[1];

		node_addr64_set((uint8_t*)&node_addr64);
		node_data_set(NODE_ADDR16H,node_addr16.u8[0]);
		node_data_set(NODE_ADDR16L,node_addr16.u8[1]);

		/* set dest node */
		dest_addr64.u8[6] = H(unet_neighbourhood[0].Addr_16b);
		dest_addr64.u8[7] = L(unet_neighbourhood[0].Addr_16b);

		TEST_ASSERT(unet_router_up_table_entry_find(&dest_addr64) == 0);

		res = unet_packet_up_sendto(&dest_addr64, payload_len);

		TEST_ASSERT(res == RESULT_PACKET_SEND_OK);
		TEST_ASSERT(p->state == PACKET_WAITING_ACK);
		packet_release_up();

}

void test_app_packet_create_for_router_up_ack(void)
{

		extern packet_t packet_up;
		packet_t  *p = &packet_up;
		uint8_t payload_len = 16;
		addr64_t dest_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x12,0x34}};
		packet_t  pkt_backup_buffer_origin;
		packet_t  pkt_rx;

		/* create a app packet for testing router up */
		TEST_ASSERT(packet_acquire_up() == PACKET_ACCESS_ALLOWED);

		/* add fake payload */
		REQUIRE_FOREVER((UNET_APP_HEADER_START+payload_len) < PACKET_END);
		memset(&(p->packet[UNET_APP_HEADER_START]), 0xAA, payload_len);

		/* set source node */
		addr16_t node_addr16;
		addr64_t node_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x00,0x00}};

		node_addr16.u8[0] = H(unet_neighbourhood[1].Addr_16b);
		node_addr16.u8[1] = L(unet_neighbourhood[1].Addr_16b);
		node_addr64.u8[6] = node_addr16.u8[0];
		node_addr64.u8[7] = node_addr16.u8[1];

		node_addr64_set((uint8_t*)&node_addr64);
		node_data_set(NODE_ADDR16H,node_addr16.u8[0]);
		node_data_set(NODE_ADDR16L,node_addr16.u8[1]);

		/* set dest node */
		dest_addr64.u8[6] = H(unet_neighbourhood[0].Addr_16b);
		dest_addr64.u8[7] = L(unet_neighbourhood[0].Addr_16b);

		TEST_ASSERT(unet_router_up_table_entry_find(&dest_addr64) == 0);

		uint8_t res = unet_packet_up_sendto(&dest_addr64, payload_len);

		PRINTF("Sending packet ");
		if(res == RESULT_PACKET_SEND_OK)
		{
			PRINTF("OK\r\n");

		}else
		{
			PRINTF("failed due to reason %d \r\n", res);
		}

		TEST_ASSERT(res == RESULT_PACKET_SEND_OK);
		TEST_ASSERT(p->state == PACKET_WAITING_ACK);

		memcpy(&pkt_backup_buffer_origin, p, sizeof(packet_t));

		packet_release_up();
		TEST_ASSERT(p->state == PACKET_IDLE);

		/* set current receiving node */
		node_addr16.u8[0] = H(unet_neighbourhood[0].Addr_16b);
		node_addr16.u8[1] = L(unet_neighbourhood[0].Addr_16b);
		node_addr64.u8[6] = node_addr16.u8[0];
		node_addr64.u8[7] = node_addr16.u8[1];

	    node_addr64_set((uint8_t*)&node_addr64);
		node_data_set(NODE_ADDR16H,node_addr16.u8[0]);
		node_data_set(NODE_ADDR16L,node_addr16.u8[1]);

		memset(p->info, 0, PKTINFO_MAX_OPT);
		memset(pkt_rx.packet, 0, MAX_PACKET_SIZE);

		packet_print(p->packet,ieee802154_packet_length_get(p->packet)+2);

		/* simulate packet rx */
		memcpy(pkt_rx.packet, p->packet, p->packet[PHY_PKT_SIZE]+2);

		pkt_rx.info[PKTINFO_RSSI] = 100;
		pkt_rx.info[PKTINFO_LQI] = 100;

		uint16_t fcs = ieee802154_fcs_calc(&pkt_rx.packet[MAC_FRAME_CTRL], pkt_rx.packet[PHY_PKT_SIZE]);

		/* add fcs field */
		pkt_rx.info[PKTINFO_FCSH] = H(fcs);
		pkt_rx.info[PKTINFO_FCSL] = L(fcs);

		/* packet rx input */
		UNET_RADIO.set(CRC,1);
		TEST_ASSERT(ieee802154_packet_input(&pkt_rx) == ACK_REQ_TRUE);

		PRINTF("Link layer ACK \r\n");
		packet_print(pkt_rx.packet,ieee802154_packet_length_get(pkt_rx.packet)+2);

		/* simulate link layer ack packet rx */
		fcs = ieee802154_fcs_calc(&pkt_rx.packet[MAC_FRAME_CTRL], pkt_rx.packet[PHY_PKT_SIZE]);

		/* add fcs field */
		pkt_rx.info[PKTINFO_FCSH] = H(fcs);
		pkt_rx.info[PKTINFO_FCSL] = L(fcs);

		/* restore source node address */
		node_data_set(NODE_ADDR16H,H(unet_neighbourhood[1].Addr_16b));
		node_data_set(NODE_ADDR16L,L(unet_neighbourhood[1].Addr_16b));

		/* restore packet buffer */
		memcpy(p,&pkt_backup_buffer_origin, sizeof(packet_t));
		TEST_ASSERT(p->state == PACKET_WAITING_ACK);

		/* packet rx input */
		UNET_RADIO.set(CRC,1);
		TEST_ASSERT(ieee802154_packet_input(&pkt_rx) == ACK_REQ_FALSE);
		//TEST_ASSERT(p->state == PACKET_IDLE);

}

void test_ieee802154_bitfields(void)
{

	volatile ieee802154_frame_control_t fc;
	fc.u8[0] = 0;
	fc.u8[1] = 0;

	fc.bits.FrameType = DATA_FRAME;
	fc.bits.ACKRequest = FALSE;

	/* only intra PAN mode is supported */
	fc.bits.IntraPAN = TRUE;

	/* only 16-bit address mode is supported */
	fc.bits.SrcAddrMode = ADDR16_MODE;
	fc.bits.DstAddrMode = ADDR16_MODE;

	TEST_ASSERT(fc.u8[0] == 0x41);
	TEST_ASSERT(fc.u8[1] == 0x88);

	/* only data frame is supported */
	TEST_ASSERT((fc.bits.FrameType == DATA_FRAME));

	/* 64-bit addressing not supported */
	TEST_ASSERT(((fc.bits.SrcAddrMode == ADDR16_MODE) || (fc.bits.SrcAddrMode == ADDR16_MODE)));

	/* security not supported yet */
	TEST_ASSERT((fc.bits.SecurityEnabled == FALSE));

	/* ack not requested */
	TEST_ASSERT((fc.bits.ACKRequest == FALSE));

}

void test_ieee802154_bitmasks(void)
{

	volatile ieee802154_frame_control_t fc;
	fc.u8[0] = 0;
	fc.u8[1] = 0;

	//fc.u16 = 0;
	BITFIELD_SET(fc.u8[0], MASK_FRAME_TYPE, SHIFT_FRAME_TYPE, DATA_FRAME);
	BITFIELD_SET(fc.u8[0], MASK_ACK_REQ, SHIFT_ACK_REQ, ACK_REQ_FALSE);

	/* only intra PAN mode is supported */
	BITFIELD_SET(fc.u8[0], MASK_INTRA_PAN, SHIFT_INTRA_PAN, TRUE);

	/* only 16-bit address mode is supported */
	BITFIELD_SET(fc.u8[1], MASK_DEST_ADDR_MODE, SHIFT_DEST_ADDR_MODE, ADDR16_MODE);
	BITFIELD_SET(fc.u8[1], MASK_SRC_ADDR_MODE, SHIFT_SRC_ADDR_MODE, ADDR16_MODE);

	TEST_ASSERT(fc.u8[0] == 0x41);
	TEST_ASSERT(fc.u8[1] == 0x88);

	/* only data frame is supported */
	TEST_ASSERT((BitFieldGet(fc.u8[0],MASK_FRAME_TYPE,SHIFT_FRAME_TYPE) == DATA_FRAME));

	/* 64-bit addressing not supported */
	TEST_ASSERT((BitFieldGet(fc.u8[1],MASK_DEST_ADDR_MODE,SHIFT_DEST_ADDR_MODE) == ADDR16_MODE));
	TEST_ASSERT((BitFieldGet(fc.u8[1],MASK_SRC_ADDR_MODE,SHIFT_SRC_ADDR_MODE) == ADDR16_MODE));

	/* security not supported yet */
	TEST_ASSERT((BitFieldGet(fc.u8[0],MASK_SEC_ENABLED,SHIFT_SEC_ENABLED) == FALSE));

	/* ack not requested */
	TEST_ASSERT((BitFieldGet(fc.u8[0],MASK_ACK_REQ,SHIFT_ACK_REQ) == FALSE));

}

void all_tests(void)
{
	 /* test ieee 802.15.4 bitfields */
	 //run_test(test_ieee802154_bitfields);
	 run_test(test_ieee802154_bitmasks);

	 /* test neighbor advertisement (local link packet) */
	 run_test(test_link_packet_create_for_one_panid_table_empty);
	 run_test(test_link_packet_create_for_one_panid_table_one_neighbor);
	 run_test(test_link_packet_create_for_one_panid_table_full);

#define TEST_ROUTER_DOWN   1
#if TEST_ROUTER_DOWN
	 /* test association process and down routes setup */
     run_test(test_link_packet_create_for_association_request);
     run_test(test_link_packet_reception_of_association_request);
     run_test(test_link_packet_reception_of_association_reply_from_root_node);
     run_test(test_link_packet_reception_of_association_reply_from_node_with_distance_2);

	 /* test router advertisement, down router and up routes setup */
     run_test(test_network_packet_create_for_router_adv);
     run_test(test_network_packet_create_for_router_adv_ack);
     run_test(test_network_packet_create_for_router_adv_forwarding);
#endif


#define TEST_ROUTER_UP   0
#if TEST_ROUTER_UP

	 /* test up route */
     run_test(test_app_packet_create_for_router_up);
     run_test(test_app_packet_create_for_router_up_ack);
#endif
}


/* Config. files */
#include "BRTOSConfig.h"
#include "NetConfig.h"
#include "BoardConfig.h"

/* Function prototypes */
#include "unet_api.h"

/* task that simulates other node for testing;
 * network association and routing */

#include "radio_null.h"

packet_t pkt;

void copy_packet_txed(packet_t *p)
{

	OSSemPend(Radio_TX_Event, 0);

#if (RUN_TESTS == TRUE)
	uint16_t len = 0;
//	read_radio_tx_buffer((const void *)&(p->packet[MAC_FRAME_CTRL]),&len);
	UNET_RADIO.recv((const void *)&(p->packet[MAC_FRAME_CTRL]),&len);


	REQUIRE_FOREVER(((len > 0) && (len < PACKET_END)));

	/* todo: use only one field for pkt size */
	p->packet[PHY_PKT_SIZE] = len-2;
	p->info[PKTINFO_SIZE] = len-2;

	p->info[PKTINFO_FCSH] = p->packet[len + 0];
	p->info[PKTINFO_FCSL] = p->packet[len + 1];

#endif

	//packet_print(&p->packet[MAC_FRAME_CTRL],p->info[PKTINFO_SIZE]+2);

	OSSemPost(Radio_TX_Event);
}

void UNET_test(void *param)
{
	(void)param;
	packet_t * p = &pkt;

	#if (RUN_TESTS == TRUE)
		DelayTask(2000);  /* wait network starting */
	#endif

	#if (RUN_TESTS == TRUE)
	/* packet for testing association request processing,
	 * src addr = 0x1122, pan id = 0x4742 */
	const uint8_t pkt_assoc_request [] =
	{
		0x23, 0x41, 0x88, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0x11, 0x22, 0x01, 0x00,
		0x05, 0x01, 0xFD, 0x00, 0x00, 0x47, 0x42, 0x00, 0x00, 0x11, 0x22,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x11, 0x22,
		0xFF, 0x00, 0x71, 0xE5
	};


//		simulate_radio_rx_event(&pkt_assoc_request[1], sizeof(pkt_assoc_request));
		copy_packet_txed(p);
	#endif

	/* todo: check reply is ok */
	TEST_ASSERT(p->packet[UNET_PKT_TYPE] == BROADCAST_LOCAL_LINK);

	/* packet for router adv testing,
	 * src addr = 0x1122, pan id = 0x4742 */

	#if (RUN_TESTS == TRUE)
	const uint8_t pkt_router_adv [] =
	{
		0x1F, 0x61,0x88,0x02,0x47,0x42,0x12,0x34,0x11,0x22,0x01,0x05,0x01,0xFE,0xFD,
		0x00,0x00,0x47,0x42,0x00,0x00,0x11,0x22,0x00,0x00,0x47,0x42,0x00,0x00,
		0x12,0x34,0x03,0x56,0x65
	};
//		simulate_radio_rx_event(&pkt_router_adv[1], sizeof(pkt_router_adv));
		copy_packet_txed(p);
	#endif

	/* todo: check reply is ok */
	TEST_ASSERT(p->packet[UNET_PKT_TYPE] == UNICAST_ACK_DOWN);

	DelayTask(5000);

	extern packet_t packet_up;
	p = &packet_up;
	uint8_t payload_len = 16;
	addr64_t dest_addr64 = {.u8 = {0x00,0x00,0x47,0x42,0x00,0x00,0x11,0x22}};

	/* create a app packet for testing router up */
	TEST_ASSERT(packet_acquire_up() == PACKET_ACCESS_ALLOWED);

	/* add fake payload */
	REQUIRE_FOREVER((UNET_APP_HEADER_START+payload_len) < PACKET_END);
	memset(&(p->packet[UNET_APP_HEADER_START]), 0xAA, payload_len);

	uint8_t res = unet_packet_up_sendto(&dest_addr64, payload_len);

	PRINTF("Packet will be routed up\r\n");
	extern BRTOS_Sem * Router_Up_Route_Request;
	OSSemPost(Router_Up_Route_Request);


	PRINTF("Sending packet ");
	if(res == RESULT_PACKET_SEND_OK)
	{
		PRINTF("OK\r\n");

	}else
	{
		PRINTF("failed due to reason %d \r\n", res);
	}

	TEST_ASSERT(res == RESULT_PACKET_SEND_OK);
	TEST_ASSERT(p->state == PACKET_WAITING_ACK);


	PRINTF("all tests passed\r\n");

	for(;;)
	{

		DelayTask(1000);
	}
}



