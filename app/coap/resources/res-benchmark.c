/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Benchmark resource
 * \author
 *      Fabrício N. Godoi <fabricio.n.godoi@gmail.com>
 */


#include <string.h>
#include "../benchmark-coap.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "er-coap-transactions.h"

/*---------------------------------------------------------------------------*/
/* Variables */
static void *data;
static BRTOS_Sem *callback;
static uint8_t res_have_data = 0;

/*---------------------------------------------------------------------------*/
/* Resource Implementation */

static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	if(request != NULL && ((coap_packet_t*)request)->payload_len > 0){
		res_have_data = 1;
		// Process the data
		data = request;

		// Wake up the benchmark process
		OSSemPost(callback);

		// Return "OK" the message has been received
//		REST.set_response_status(response, REST.status.OKAY);
	}
}

/*---------------------------------------------------------------------------*/
/* Resource access methods */
uint8_t res_benchmark_is_pend(){
	return res_have_data;
}

void res_benchmark_post(void){
	res_have_data = 0;
}

void res_benchmark_set_callback(BRTOS_Sem *c){
	callback = c;
}
void* res_benchmark_get_data(){
	return data;
}


/*---------------------------------------------------------------------------*/
/* Benchmark resource */
RESOURCE(res_benchmark, "", NULL, res_post_handler, NULL, NULL);
/*---------------------------------------------------------------------------*/
