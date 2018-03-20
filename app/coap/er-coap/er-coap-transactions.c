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
 *      CoAP module for reliable transport
 *      This is an adaptation of Contiki source code to BRTOS.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 *      Fabricio N. de Godoi <fabricio.n.godoi@gmail.com>
 */

#include "memb.h"
#include "list.h"
#include "random.h"
#include "er-coap-transactions.h"
#include "er-coap-observe.h"
#include "stimer.h"

#define COAP_DEBUG 0
#if COAP_DEBUG
#ifndef PRINTF
#include "stdio.h"
#define PRINTF(...) printf(__VA_ARGS__)
#endif
#define PRINT6ADDR(addr) PRINTF("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n\r", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#ifdef PRINTF
#undef PRINTF
#endif
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*---------------------------------------------------------------------------*/

#if COAP_MAX_OPEN_TRANSACTIONS > BRTOS_MAX_TIMER
#warning "er-coap-transactions: There are more max transactions than timers, consider adjusting it!"
#endif

MEMB(transactions_memb, coap_transaction_t, COAP_MAX_OPEN_TRANSACTIONS); // 672 bytes
LIST(transactions_list);

/*---------------------------------------------------------------------------*/
/*- Internal API ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void
coap_register_as_transaction_handler()
{
//  transaction_handler_process = PROCESS_CURRENT();
	PRINTF("TO BE IMPLEMENTED: coap_register_as_transaction_handler()\n");
}
coap_transaction_t *
coap_new_transaction(uint16_t mid, addr64_t *addr, uint16_t port)
{
  coap_transaction_t *t = memb_alloc(&transactions_memb);

  if(t) {
    t->mid = mid;
    t->retrans_counter = 0;
    t->callback = NULL;

    /* save client address */
    memcpy(&t->addr,addr,sizeof(addr64_t));

    t->port = port;

    // Setup the timer without starting it
//	PRINTF("er-coap-transactions: state before - %d!\n",t->retrans_timer->state);
    if(OSTimerSet(&t->retrans_timer, coap_check_transactions, 0) != 0){
    	PRINTF("er-coap-transactions: OSTimer not available - %d!\n",t->retrans_timer->state);
    }

    list_add(transactions_list, t); /* list itself makes sure same element is not added twice */
  }

  return t;
}
/*---------------------------------------------------------------------------*/
void
coap_send_transaction(coap_transaction_t *t)
{
  PRINTF("Sending transaction %u\n", t->mid);

  coap_send_message(&t->addr, t->port, t->packet, t->packet_len);

  if(COAP_TYPE_CON ==
     ((COAP_HEADER_TYPE_MASK & t->packet[0]) >> COAP_HEADER_TYPE_POSITION)) {
    if(t->retrans_counter < COAP_MAX_RETRANSMIT) {
      /* not timed out yet */
      PRINTF("Keeping transaction %u\n", t->mid);

      if(t->retrans_counter == 0) {
        t->retrans_interval =
          COAP_RESPONSE_TIMEOUT_TICKS + (random_get() %
                              (ostick_t) COAP_RESPONSE_TIMEOUT_BACKOFF_MASK);

          // Create and start the timer
          // INFO: timer is reseted at the return of coap_check_transactions()
		  if(OSTimerStart(t->retrans_timer, t->retrans_interval) != OK){
			  PRINTF("er-coap-transaction: OSTimer cannot be started\n");
		  }

      } else {
    	t->retrans_interval <<= 1;  /* double */
        PRINTF("Doubled (%u) interval\n", t->retrans_counter);
      }

      t = NULL;
    } else {
      /* timed out */
      PRINTF("Timeout\n");
//      printf("coap drop %d\n",t->mid);
      restful_response_handler callback = t->callback;
      void *callback_data = t->callback_data;

      /* handle observers */
      coap_remove_observer_by_client(&t->addr, t->port);

      coap_clear_transaction(t);

      if(callback) {
        callback(callback_data, NULL);
      }
    }
  } else {
	restful_response_handler callback = t->callback;
	void *callback_data = t->callback_data;
    coap_clear_transaction(t);

    /// TODO must check with the Contiki what happens, because the code is being stucked at the coap_sending_block at the semaphore
    // Despite of the NON messages being responded, there are no need to wait a ACK
    if(callback) {
    	callback(callback_data, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
coap_clear_transaction(coap_transaction_t *t)
{
  if(t) {
//    printf("Freeing transaction %u: %p\n", t->mid, t);

    // Stop and delete the timer
    OSTimerStop(t->retrans_timer, TRUE);
    list_remove(transactions_list, t);
    memb_free(&transactions_memb, t);
  }
}
coap_transaction_t *
coap_get_transaction_by_mid(uint16_t mid)
{
  coap_transaction_t *t = NULL;

  for(t = (coap_transaction_t *)list_head(transactions_list); t; t = t->next) {
    if(t->mid == mid) {
      PRINTF("Found transaction for MID %u: %p\n", t->mid, t);
      return t;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
ostick_t
coap_check_transactions()
{
  coap_transaction_t *t = NULL;

  for(t = (coap_transaction_t *)list_head(transactions_list); t; t = t->next) {
	if(t->retrans_timer->state == TIMER_STOPPED){
      ++(t->retrans_counter);
//      printf("Retransmitting %u (%n)\n", t->mid, t->retrans_counter);
      coap_send_transaction(t);

      // Restart the timer if the max retransmission isn't reached
      if(t->retrans_counter < COAP_MAX_RETRANSMIT){
    	  return t->retrans_interval;
      }
      return 0; // Do not repeat the timer
    }
  }
  return 0; // Do not repeat the timer
}
/*---------------------------------------------------------------------------*/
