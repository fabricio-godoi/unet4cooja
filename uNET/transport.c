/*
 * transport.c
 *
 *  Created on: 29 de abr de 2016
 *      Author: gustavo
 */
#include "transport.h"

#ifndef NULL
#define NULL  (void*)0
#endif

unet_transport_t *unet_tp_tail=NULL;
unet_transport_t *unet_tp_head=NULL;

extern packet_t packet_up;
extern packet_t packet_down;

void IncludeServerClient(unet_transport_t *p){
        if(unet_tp_tail != NULL){
          /* Insert server/client into list */
          unet_tp_tail->next = p;
          p->previous = unet_tp_tail;
          unet_tp_tail = p;
          unet_tp_tail->next = NULL;
        }
        else{
           /* Init server/client list */
           unet_tp_tail = p;
           unet_tp_head = p;
           p->next = NULL;
           p->previous = NULL;
        }
}

void RemoveServerClient(unet_transport_t *p){
	if(p == unet_tp_head){
	  if(p == unet_tp_tail){
		unet_tp_head = NULL;
		unet_tp_tail = NULL;
	  }
	  else{
		unet_tp_head = p->next;
		unet_tp_head->previous = NULL;
	  }
	}
	else{
	  if(p == unet_tp_tail){
		unet_tp_tail = p->previous;
		unet_tp_tail->next = NULL;
	  }
	  else{
		p->next->previous = p->previous;
		p->previous->next = p->next;
	  }
	}
}


int unet_listen(unet_transport_t *server){
	OSSemCreate(0,&(server->wake_up));
	IncludeServerClient(server);

	return 0;
}

int unet_connect(unet_transport_t *client){
	OSSemCreate(0,&(client->wake_up));
	IncludeServerClient(client);

	return 0;
}

int unet_close(unet_transport_t *server_client){
	RemoveServerClient(server_client);
	OSSemDelete(&(server_client->wake_up));

	return 0;
}

int unet_recv(unet_transport_t *server_client, uint8_t *buffer, uint16_t timeout){
	int ret;
	if ((ret = OSSemPend(server_client->wake_up,timeout)) == OK)
	{
		memcpy(buffer,server_client->packet,server_client->payload_size);
		//memset(server_client->packet,0x00,server_client->payload_size);
	}
	return ret;
}

volatile ostick_t last_transmition_time = 0;
int unet_send(unet_transport_t *server_client, uint8_t *buffer, uint8_t length, uint16_t ret_time){
	packet_t  *p;
	uint8_t up_route = 0;
	int ret = -1;
	int ret_cnt = 0;
	int ret_limit;

	// Corrige poss�veis falhas na passagem de par�metros
	if (!ret_time) ret_time = 10;
	if (ret_time > MAX_TRANSP_TIME) ret_time = MAX_TRANSP_TIME;

	// C�lcula quantidade de retentativas
	ret_limit = MAX_TRANSP_TIME / ret_time;

	if (memcmp(node_pan_id64_get(),server_client->dest_address,8) == 0)
	{
		up_route = FALSE;
		p = &packet_down;
	}else
	{
		up_route = TRUE;
		p = &packet_up;
	}

	do
	{
		if (up_route == TRUE)
		{
			if(packet_acquire_up() == PACKET_ACCESS_DENIED)
			{
				ret = -1;
			}else
			{
				break;
			}
		}else
		{
			if(packet_acquire_down() == PACKET_ACCESS_DENIED)
			{
				ret = -1;
			}else{
				break;
			}
		}
		OSDelayTask(ret_time);
		ret_cnt++;
		if (ret_cnt >= ret_limit)
		{
			return -1;
		}
	}while(ret == -1);

	ostick_t transmition_time = OSGetTickCount();
	// Deixa um tempo para o pacote anterior ser emcaminhado
	if ((transmition_time - last_transmition_time) < 30) OSDelayTask(30);

	p->packet[UNET_SOURCE_PORT] = server_client->src_port;
	p->packet[UNET_DEST_PORT] = server_client->dst_port;
	p->packet[UNET_APP_PAYLOAD_LEN] = length;
	memcpy(&p->packet[UNET_APP_HEADER_START],buffer,length);

	uint8_t payload_len = length+(UNET_TRANSP_HEADER_END-UNET_TRANSP_HEADER_START);
	uint8_t res = 0;
	if (up_route == TRUE)
	{
		res = unet_packet_up_sendto(server_client->dest_address, payload_len);
		if(res == RESULT_PACKET_SEND_OK)
		{
			extern BRTOS_Sem * Router_Up_Route_Request;
			OSSemPost(Router_Up_Route_Request);
		}else
		{
			packet_release_up();
		}
	}else
	{
		res = unet_packet_down_send(payload_len);
		if(res == RESULT_PACKET_SEND_OK)
		{
			extern BRTOS_Sem * Router_Down_Route_Request;
			OSSemPost(Router_Down_Route_Request);
		}else
		{
			packet_release_down();
		}
	}

	last_transmition_time = OSGetTickCount();

	ret = (int)res;

	return ret;
}
