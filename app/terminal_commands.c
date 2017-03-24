/* The License
 * 
 * Copyright (c) 2015 Universidade Federal de Santa Maria
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

*/
/*
 * terminal_commands.c
 *
 */
#include "BRTOS.h"
#include "AppConfig.h"
#include "NetConfig.h"
#include "terminal.h"
//#include "stdio.h"

#define printf_terminal(...)  TERM_PRINT(__VA_ARGS__);
//#define SPRINTF(a,...)		  sprintf(a,__VA_ARGS__)
#define SPRINTF(a,...)		  snprintf_lib(a,256,__VA_ARGS__)


void terminal_newline(void)
{
	printf_terminal("\n\r");
}

#include "transport.h"
#include "unet_router.h"
#include "unet_api.h"

addr64_t 		  dest_addr64;
unet_transport_t  client = {.src_port = 22, .dst_port = 23, .dest_address = &dest_addr64};
uint8_t   		  payload_tx[16];
uint8_t   		  payload_rx[98];

char* packet_state_to_string(uint8_t state)
{
	if(state == PACKET_IDLE)
	{
		return "idle";
	}else
	{
		return "busy";
	}
}

#if ((UNET_DEVICE_TYPE == PAN_COORDINATOR) || (BRTOS_PLATFORM == BOARD_ROTEADORCFV1))

// BRTOS version Command
CMD_FUNC(ver)
{
  printf_terminal("%s", (CHAR8*)version);

  return NULL;
}

// TOP Command (similar to the linux command)
#include "OSInfo.h"
char big_buffer[1024];
CMD_FUNC(top)
{
  OSCPULoad(big_buffer);
  printf_terminal(big_buffer);

  OSUptimeInfo(big_buffer);
  printf_terminal(big_buffer);

  OSAvailableMemory(big_buffer);
  printf_terminal(big_buffer);

  OSTaskList(big_buffer);
  printf_terminal(big_buffer);

  return NULL;
}

CMD_FUNC(runst)
{
#if (COMPUTES_TASK_LOAD == 1)
  OSRuntimeStats(big_buffer);
  printf_terminal(big_buffer);
#endif

  return NULL;
}

// Show stats
CMD_FUNC(netst)
{
	uint8_t len = 0;
	struct netstat_t *ns = (struct netstat_t *)GetUNET_Statistics(&len);
	if(len > 0)
	{
		printf_terminal("Net statistics:\n\r");
		printf_terminal("rxed:     %u\n\r",ns->rxed);
		printf_terminal("txed:     %u\n\r",ns->txed);
		printf_terminal("txfailed: %u\n\r",ns->txfailed);
		printf_terminal("routed:   %u\n\r",ns->routed);
		printf_terminal("apptxed:  %u\n\r",ns->apptxed);
		printf_terminal("dropped:  %u\n\r",ns->dropped);
		printf_terminal("overbuf:  %u\n\r",ns->overbuf);
		printf_terminal("routdrop: %u\n\r",ns->routdrop);
		printf_terminal("duplink:  %u\n\r",ns->duplink);
		printf_terminal("dupnet:   %u\n\r",ns->dupnet);
		printf_terminal("rxbps:    %u\n\r",ns->rxbps);
		printf_terminal("txbps:    %u\n\r",ns->txbps);
		printf_terminal("resets:   %u\n\r",ns->radioresets);
		printf_terminal("hellos:   %u\n\r",ns->hellos);
		printf_terminal("rxedbytes:%u\n\r",ns->rxedbytes);
		printf_terminal("txedbytes:%u\n\r",ns->txedbytes);
	}
	printf_terminal("Buffer UP %s, DOWN %s \n\r", packet_state_to_string(packet_state_up()), packet_state_to_string(packet_state_down()));
	printf_terminal("Last RX seq. num.:\r\n");

	uint8_t i = 0;
	for(i=0;i<MAX_SEQUENCES_NUM;i++)
	{
		extern uint8_t last_rx_sequences[];
		printf_terminal("%u-", last_rx_sequences[i]);
	}

	if(argv[1]!= NULL)
	{
		char argv1;
		sscanf(argv[1],"%c", &argv1);
		if(argv1 == 'c') /* clean stats */
		{
			NODESTAT_RESET();
			extern uint8_t last_rx_sequences[];
			memset(last_rx_sequences,0x00, MAX_SEQUENCES_NUM);
		}
	}
	return NULL;
}

// Show net role, addr16, addr64, rx channel
CMD_FUNC(netaddr)
{
	printf_terminal("\n\rAddr64b: ");
	node_addr64_print(node_addr64_get());
	printf_terminal("\n\rPanId64: ");
	node_addr64_print(node_pan_id64_get());

	return NULL;
}

// Print neighbor table
CMD_FUNC(netnt)
{
	uint8_t i;
	uint8_t nb_cnt = 0;
	unet_neighborhood_t *nb;
	extern volatile unet_neighborhood_t unet_neighbourhood[];

	printf_terminal("==========================\n\r");
	for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	{
		nb = (unet_neighborhood_t*)&unet_neighbourhood[i];
		if(nb->Addr_16b != UNUSED_ADDR)
		{
			++nb_cnt;
			printf_terminal("[%u] - addr16: %u dist: %u rssi(in): %u rssi(out): %u sn: %u ", i, nb->Addr_16b,
					nb->NeighborDistance, nb->NeighborRSSI, nb->NeighborMyRSSI, nb->NeighborLastID);

			if(nb->NeighborStatus.bits.Symmetric == TRUE)
			{
				printf_terminal("S");
			}else
			{
				printf_terminal("A");
			}
			terminal_newline();
		}
	}
	printf_terminal("Num. of neighbors: %d\n\r", nb_cnt);
	printf_terminal("==========================\n\r");

	return NULL;
}

// Print route table
CMD_FUNC(netrt)
{
	uint16_t i;
	uint16_t route_cnt = 0;
	unet_routing_table_up_t *r = (unet_routing_table_up_t *)unet_rtable_up_get();

	printf_terminal("==========================\n\r");
	for(i=0;i<RTABLE_UP_ENTRIES_MAX_NUM;i++)
	{
		if(r->next_hop != 0)
		{
			++route_cnt;
			printf_terminal("[%u] - destaddr64: ", i);
			node_addr64_print(r->dest_addr64.u8);
			printf_terminal(" - nexthop: %u\n\r", r->next_hop);
		}
		r++;
	}
	printf_terminal("Num. of routes: %d\n\r", route_cnt);
	printf_terminal("==========================\n\r");

	return NULL;
}

// start/stop listening rx packets, arg1 timeout
CMD_FUNC(netrx)
{
	int timeout = 1000;

	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%d", &timeout);
	}

	unet_connect(&client);

	if (unet_recv(&client,payload_rx,timeout) >= 0)
	{
		printf_terminal("Packet received from (port/address):  %d/",client.sender_port); print_addr64(&(client.sender_address));
		printf_terminal("Packet Content: %s\n\r",payload_rx);
	}

	unet_close(&client);

	return NULL;
}

// tx a packet, arg payload
CMD_FUNC(nettx)
{
	uint8_t i,size= 0;
	char *s;
	ostick_t init,final;
	unsigned int total_time;

	/* copy argv to buffer */
	s = (char*)payload_tx;
	for(i=1;i<argc;i++)
	{
		strncpy((char*)(s+size), argv[i],sizeof(payload_tx) - size);
		size += strlen(argv[i]);
		s[size++] = ' ';
	}
	s[size] = '\0';

#if 0
	if(argv[1]!= NULL)
	{
		strncpy((char*)payload_tx, argv[1],sizeof(payload_tx));
	}
#endif

	unet_connect(&client);
#if 0
	s = payload_tx;
	size = 0;
	while(*s){
		size++;
		s++;
	}
#endif

	init = OSGetTickCount();

	unet_send(&client, payload_tx, size, 30);
	printf_terminal("Response:\n\r");
	size = 0;
	do{
		if (unet_recv(&client, payload_rx, 5000) >= 0)
		{
			size += client.payload_size + PACKET_OVERHEAD_SIZE;

			char *p;
			if((p=strchr(payload_rx, ETX)) != NULL)
			{
				*p = '\0';
				client.payload_size = 0;
			}
			printf_terminal("%s", payload_rx);
		}else
		{
			client.payload_size = 0;
		}
	}while(client.payload_size == MAX_APP_PAYLOAD_SIZE);
	unet_close(&client);

	final = OSGetTickCount();
	total_time = (unsigned int)(final - init);

	if(size == 0)
	{
		printf_terminal("ERROR: Timeout\n\r");
	}else
	{
		printf_terminal("\n\rBytes sent: %u bytes\n\r", size);
	}
	printf_terminal("\n\rResponse Time: %ums\n\r", total_time);
	printf_terminal("\n\rBit rate: %u bps\n\r", (size*8)/total_time);

	return NULL;
}

// set a destination to tx a packet, arg is dest idx from route table
CMD_FUNC(netsd)
{
	unsigned int idx = 0;
	unet_routing_table_up_t *r = (unet_routing_table_up_t *)unet_rtable_up_get();
	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%u", &idx);
		if(r[idx].next_hop != UNUSED_ADDR)
		{
			memcpy(dest_addr64.u8,r[idx].dest_addr64.u8,8);
			client.dest_address = &dest_addr64;
		}
	}
	printf_terminal("Dest. addr: ");
	node_addr64_print(dest_addr64.u8);

	return NULL;
}

CMD_FUNC(netch)
{
	unsigned int chan = 0;
	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%u", &chan);
		if((chan >= 11) && (chan <= 26))
		{
			UNET_RADIO.set(CHANNEL,chan);
		}
	}
	UNET_RADIO.get(CHANNEL, (uint8_t*)&chan);
	printf_terminal("RX Channel: %u", chan);
	return NULL;
}

CMD_FUNC(netpl)
{
	unsigned int power_level_val = 0;
	uint8_t power_level = 0;
	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%u", &power_level_val);
		power_level = (power_level_val & 0xff);
		UNET_RADIO.set(TXPOWER,power_level);
	}
	UNET_RADIO.get(TXPOWER, (uint8_t*)&power_level);
	printf_terminal("TX Power Level: %u", (power_level & 0xFF));
	return NULL;
}

CMD_FUNC(nettst)
{
	return NULL;
}

CMD_FUNC(netdbv)
{
	int verb_level = 0;
	int verb_layer = 0;
	extern uint16_t unet_verbose;

	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%d", &verb_layer);
		if(verb_layer >= 0 && verb_layer <= 7)
		{
			if(argv[2]!= NULL)
			{
				sscanf(argv[2],"%d", &verb_level);
				if(verb_level > 3) verb_level = 3;
				unet_verbose = (unet_verbose &~(0x3 << verb_layer*2)) | (verb_level  << (verb_layer*2));
			}
		}else
		{
			unet_verbose = (uint16_t)(-1);
		}
	}


	printf_terminal("UNET debug verbose: %u", unet_verbose);
	return NULL;
}

#else
char big_buffer[1024];

// Print neighbor table
CMD_FUNC(netnt)
{
	uint8_t i;
	uint8_t nb_cnt = 0;
	unet_neighborhood_t *nb;
	int idx = 0;
	extern volatile unet_neighborhood_t unet_neighbourhood[];

	idx += SPRINTF(big_buffer,"==========================\n\r");
	for(i=0;i<NEIGHBOURHOOD_SIZE;i++)
	{
		nb = (unet_neighborhood_t*)&unet_neighbourhood[i];
		if(nb->Addr_16b != UNUSED_ADDR)
		{
			++nb_cnt;
			idx += SPRINTF(&big_buffer[idx],"[%u] - addr16: %u dist: %u rssi(in): %u rssi(out): %u sn: %u ", i, nb->Addr_16b,
					nb->NeighborDistance, nb->NeighborRSSI, nb->NeighborMyRSSI, nb->NeighborLastID);

			if(nb->NeighborStatus.bits.Symmetric == TRUE)
			{
				idx += SPRINTF(&big_buffer[idx],"S");
			}else
			{
				idx += SPRINTF(&big_buffer[idx],"A");
			}
			idx += SPRINTF(&big_buffer[idx],"\n\r");
		}
	}
	idx += SPRINTF(&big_buffer[idx],"Num. of neighbors: %d\n\r", nb_cnt);
	idx += SPRINTF(&big_buffer[idx],"==========================\n\r");

	big_buffer[idx] = '\0';
	return big_buffer;
}

// BRTOS version Command
CMD_FUNC(ver)
{
	SPRINTF(big_buffer,"%s", (CHAR8*)version);

	return big_buffer;
}

// TOP Command (similar to the linux command)
#include "OSInfo.h"
CMD_FUNC(top)
{
  int idx = 0;
  char *s;

  OSCPULoad(big_buffer);
  s = big_buffer;
  while(*s){
	  idx++;
	  s++;
  }

  OSUptimeInfo(&big_buffer[idx]);
  s = &big_buffer[idx];
  while(*s){
	  idx++;
	  s++;
  }

  OSAvailableMemory(&big_buffer[idx]);
  s = &big_buffer[idx];
  while(*s){
	  idx++;
	  s++;
  }

  OSTaskList(&big_buffer[idx]);
  s = &big_buffer[idx];
  while(*s){
	  idx++;
	  s++;
  }
  big_buffer[idx] = '\0';

  return big_buffer;
}

CMD_FUNC(runst)
{
#if (COMPUTES_TASK_LOAD == 1)
  OSRuntimeStats(big_buffer);
  //printf_terminal(big_buffer);
#endif

  return big_buffer;
}

// Show stats
CMD_FUNC(netst)
{
	uint8_t len = 0;
	int idx = 0;
	struct netstat_t *ns = (struct netstat_t *)GetUNET_Statistics(&len);
	if(len > 0)
	{
		idx += SPRINTF(big_buffer,"Net statistics:\n\r");
		idx += SPRINTF(&big_buffer[idx],"rxed:     %u\n\r",ns->rxed);
		idx += SPRINTF(&big_buffer[idx],"txed:     %u\n\r",ns->txed);
		idx += SPRINTF(&big_buffer[idx],"txfailed: %u\n\r",ns->txfailed);
		idx += SPRINTF(&big_buffer[idx],"routed:   %u\n\r",ns->routed);
		idx += SPRINTF(&big_buffer[idx],"apptxed:  %u\n\r",ns->apptxed);
		idx += SPRINTF(&big_buffer[idx],"dropped:  %u\n\r",ns->dropped);
		idx += SPRINTF(&big_buffer[idx],"overbuf:  %u\n\r",ns->overbuf);
		idx += SPRINTF(&big_buffer[idx],"routdrop: %u\n\r",ns->routdrop);
		idx += SPRINTF(&big_buffer[idx],"duplink:  %u\n\r",ns->duplink);
		idx += SPRINTF(&big_buffer[idx],"dupnet:   %u\n\r",ns->dupnet);
		idx += SPRINTF(&big_buffer[idx],"rxbps:    %u\n\r",ns->rxbps);
		idx += SPRINTF(&big_buffer[idx],"txbps:    %u\n\r",ns->txbps);
		idx += SPRINTF(&big_buffer[idx],"resets:   %u\n\r",ns->radioresets);
		idx += SPRINTF(&big_buffer[idx],"hellos:   %u\n\r",ns->hellos);
		idx += SPRINTF(&big_buffer[idx],"rxedbytes:%u\n\r",ns->rxedbytes);
		idx += SPRINTF(&big_buffer[idx],"txedbytes:%u\n\r",ns->txedbytes);
	}
	idx += SPRINTF(&big_buffer[idx],"Buffer UP %s, DOWN %s \n\r", packet_state_to_string(packet_state_up()), packet_state_to_string(packet_state_down()));
	idx += SPRINTF(&big_buffer[idx],"Last RX seq. num.:\r\n");

	uint8_t i = 0;
	for(i=0;i<128;i++)
	{
		extern uint8_t last_rx_sequences[];
		idx += SPRINTF(&big_buffer[idx],"%u-", last_rx_sequences[i]);
	}

	if(argv[1]!= NULL)
	{
		char argv1;
		sscanf(argv[1],"%c", &argv1);
		if(argv1 == 'c') /* clean stats */
		{
			NODESTAT_RESET();
			extern uint8_t last_rx_sequences[];
			memset(last_rx_sequences,0x00, MAX_SEQUENCES_NUM);
		}
	}

	big_buffer[idx] = '\0';
	return big_buffer;
}

// Show net role, addr16, addr64, rx channel
#include "terminal_cfg.h"
int node_addr64_sprint(char *buffer, uint8_t *addr)
{
	REQUIRE_FOREVER(addr != NULL);
	uint8_t i = 0;
	int idx = 0;
	for (i=0;i<7;i++)
	{
		idx += SPRINTF(&buffer[idx],"%02X:", *addr++);
	}
	idx += SPRINTF(&buffer[idx],"%02X", *addr++);

	return idx;
}
CMD_FUNC(netaddr)
{
	int idx = 0;
	idx += SPRINTF(big_buffer,"\n\rAddr64b: ");
	idx += node_addr64_sprint(&big_buffer[idx],node_addr64_get());
	idx += SPRINTF(&big_buffer[idx],"\n\rPanId64: ");
	idx += node_addr64_sprint(&big_buffer[idx],node_pan_id64_get());

	big_buffer[idx] = '\0';
	return big_buffer;
}

// Print route table
CMD_FUNC(netrt)
{
	uint16_t i;
	int idx = 0;
	uint16_t route_cnt = 0;
	unet_routing_table_up_t *r = (unet_routing_table_up_t *)unet_rtable_up_get();

	idx += SPRINTF(big_buffer,"==========================\n\r");
	for(i=0;i<RTABLE_UP_ENTRIES_MAX_NUM;i++)
	{
		if(r->next_hop != 0)
		{
			++route_cnt;
			idx += SPRINTF(&big_buffer[idx],"[%u] - destaddr64: ", i);
			idx += node_addr64_sprint(&big_buffer[idx],(uint8_t*)&(r->dest_addr64.u8));
			idx += SPRINTF(&big_buffer[idx]," - nexthop: %u\n\r", r->next_hop);
		}
		r++;
	}
	idx += SPRINTF(&big_buffer[idx],"Num. of routes: %d\n\r", route_cnt);
	idx += SPRINTF(&big_buffer[idx],"==========================\n\r");

	big_buffer[idx] = '\0';
	return big_buffer;
}

// start/stop listening rx packets, arg1 timeout
CMD_FUNC(netrx)
{
	return NULL;
}

// tx a packet, arg payload
CMD_FUNC(nettx)
{
	return NULL;
}

// set a destination to tx a packet, arg is dest idx from route table
CMD_FUNC(netsd)
{
	return NULL;
}

CMD_FUNC(netch)
{
	int idx = 0;
	unsigned int chan = 0;
	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%u", &chan);
		if((chan >= 11) && (chan <= 26))
		{
			UNET_RADIO.set(CHANNEL,chan);
		}
	}
	UNET_RADIO.get(CHANNEL, (uint8_t*)&chan);
	idx += SPRINTF(&big_buffer[idx],"RX Channel: %u", chan);

	big_buffer[idx] = '\0';
	return big_buffer;
}

CMD_FUNC(netpl)
{
	int idx = 0;
	unsigned int power_level = 0;
	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%u", &power_level);
		UNET_RADIO.set(TXPOWER,power_level);
	}
	UNET_RADIO.get(TXPOWER, (uint8_t*)&power_level);
	idx += SPRINTF(&big_buffer[idx],"TX Power Level: %u", power_level);

	big_buffer[idx] = '\0';
	return big_buffer;
}

CMD_FUNC(nettst)
{
	int idx = 0;

	while(idx < (MAX_APP_PAYLOAD_SIZE * 10))
	{
		big_buffer[idx++] = 'H';
	}

	big_buffer[idx] = '\0';
	return big_buffer;
}

CMD_FUNC(netdbv)
{
	int idx = 0;
	int verb_level = 0;
	int verb_layer = 0;
	extern uint16_t unet_verbose;

	if(argv[1]!= NULL)
	{
		sscanf(argv[1],"%d", &verb_layer);
		if(verb_layer >= 0 && verb_layer <= 7)
		{
			if(argv[2]!= NULL)
			{
				sscanf(argv[2],"%d", &verb_level);
				if(verb_level > 3) verb_level = 3;
				unet_verbose = (unet_verbose &~(0x3 << verb_layer*2)) | (verb_level  << (verb_layer*2));
			}
		}else
		{
			unet_verbose = (uint16_t)(-1);
		}
	}

	idx += SPRINTF(&big_buffer[idx],"Unet debug verbose: %u", unet_verbose);
	printf_terminal("UNET debug verbose: %u", unet_verbose);
	return big_buffer;
}


#endif
