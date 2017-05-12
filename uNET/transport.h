/*
 * transport.h
 *
 *  Created on: 29 de abr de 2016
 *      Author: gustavo
 */

#ifndef UNET_TRANSPORT_H_
#define UNET_TRANSPORT_H_

#include <string.h>
#include "BRTOS.h"
#include "ieee802154.h"
#include "packet.h"
#include "unet_router.h"

#define MAX_TRANSP_TIME		 1000 //400 -> mudei de 400 p/ 1000 devido ao link burstiness de 500ms

struct _UNET_TRANSPORT_s
{
	uint8_t 	  		 	 src_port;
	uint8_t 	  		     dst_port;
	uint8_t	  		 		 *packet;
	uint8_t  		 		 payload_size;
	addr64_t				 *dest_address;
	addr64_t				 sender_address;
	uint8_t					 sender_port;
	BRTOS_Sem 		 		 *wake_up;
	struct _UNET_TRANSPORT_s *next;
	struct _UNET_TRANSPORT_s *previous;
};

typedef struct _UNET_TRANSPORT_s unet_transport_t;

extern unet_transport_t *unet_tp_head;

void IncludeServerClient(unet_transport_t *p);
void RemoveServerClient(unet_transport_t *p);
int unet_listen(unet_transport_t *server);
int unet_connect(unet_transport_t *client);
int unet_close(unet_transport_t *server_client);
int unet_recv(unet_transport_t *server_client, uint8_t *buffer, ostick_t timeout);
int unet_send(unet_transport_t *server_client, uint8_t *buffer, uint8_t length, uint16_t ret_time);


#endif /* UNET_TRANSPORT_H_ */
