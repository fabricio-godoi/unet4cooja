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
 *      An abstraction layer for RESTful Web services (Erbium).
 *      Inspired by RESTful Contiki by Dogan Yazar.
 *      This code is and adaptation from Contiki source code to BRTOS.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 *      Fabricio N. de Godoi <fabricio.n.godoi@gmail.com>
 */

#include <string.h>
#include <stdio.h>
#include "rest-engine.h"

#define REST_DEBUG 0
#if REST_DEBUG
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
LIST(restful_services);
LIST(restful_periodic_services);
ostick_t rest_engine_time_evt_handler(void);
/*---------------------------------------------------------------------------*/
/*- REST Engine API ---------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * \brief Initializes and starts the REST Engine process
 *
 * This function must be called by server processes before any resources are
 * registered through rest_activate_resource().
 */
void
rest_init_engine(void)
{
  list_init(restful_services);

  REST.set_service_callback(rest_invoke_restful_service);

  /* Start the RESTful server implementation. */
  REST.init();

  /*Start REST engine process */
  rest_engine_process();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Makes a resource available under the given URI path
 * \param resource A pointer to a resource implementation
 * \param path The URI path string for this resource
 *
 * The resource implementation must be imported first using the
 * extern keyword. The build system takes care of compiling every
 * *.c file in the ./resources/ sub-directory (see example Makefile).
 */
void
rest_activate_resource(resource_t *resource, char *path)
{
  resource->url = path;
  list_add(restful_services, resource);

  PRINTF("Activating: %s\n", resource->url);

  /* Only add periodic resources with a periodic_handler and a period > 0. */
  if((resource->flags & IS_PERIODIC) && resource->periodic->periodic_handler
     && resource->periodic->period) {
    PRINTF("Periodic resource: %p (%s)\n", resource->periodic,
           resource->periodic->resource->url);
    OSTimerSet(&resource->periodic->periodic_timer, rest_engine_time_evt_handler, 0);
    list_add(restful_periodic_services, resource->periodic);
  }
}
/*---------------------------------------------------------------------------*/
/*- Internal API ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
list_t
rest_get_resources(void)
{
  return restful_services;
}
/*---------------------------------------------------------------------------*/
int
rest_invoke_restful_service(void *request, void *response, uint8_t *buffer,
                            uint16_t buffer_size, int32_t *offset)
{
  uint8_t found = 0;
  uint8_t allowed = 1;

  resource_t *resource = NULL;
  const char *url = NULL;

  for(resource = (resource_t *)list_head(restful_services);
      resource; resource = resource->next) {

    /* if the web service handles that kind of requests and urls matches */
    if((REST.get_url(request, &url) == strlen(resource->url)
        || (REST.get_url(request, &url) > strlen(resource->url)
            && (resource->flags & HAS_SUB_RESOURCES)))
       && strncmp(resource->url, url, strlen(resource->url)) == 0) {
      found = 1;
      rest_resource_flags_t method = REST.get_method_type(request);

      PRINTF("/%s, method %u, resource->flags %u\n", resource->url,
             (uint16_t)method, resource->flags);
      if(resource == NULL) printf("resource is null\n");

      if((method & METHOD_GET) && resource->get_handler != NULL) {
        /* call handler function */
        resource->get_handler(request, response, buffer, buffer_size, offset);
      } else if((method & METHOD_POST) && resource->post_handler != NULL) {
        /* call handler function */
        resource->post_handler(request, response, buffer, buffer_size,
                               offset);
      } else if((method & METHOD_PUT) && resource->put_handler != NULL) {
        /* call handler function */
        resource->put_handler(request, response, buffer, buffer_size, offset);
      } else if((method & METHOD_DELETE) && resource->delete_handler != NULL) {
        /* call handler function */
        resource->delete_handler(request, response, buffer, buffer_size,
                                 offset);
      } else {
        allowed = 0;
        REST.set_response_status(response, REST.status.METHOD_NOT_ALLOWED);
      }
      break;
    }
  }
  if(!found) {
    REST.set_response_status(response, REST.status.NOT_FOUND);
  } else if(allowed) {
    /* final handler for special flags */
    if(resource->flags & IS_OBSERVABLE) {
      REST.subscription_handler(resource, request, response);
    }
  }

  return found & allowed;
}
/*-----------------------------------------------------------------------------------*/

ostick_t rest_engine_time_evt_handler(void) {
	periodic_resource_t *periodic_resource = NULL;

	printf("rest time evt handler\n");
	for (periodic_resource = (periodic_resource_t *) list_head(restful_periodic_services);
			periodic_resource;
			periodic_resource =	periodic_resource->next) {
		if (periodic_resource->period
			&& (periodic_resource->periodic_timer->state == TIMER_STOPPED)) {

			PRINTF("Periodic: etimer expired for /%s (period: %lu)\n",
					periodic_resource->resource->url, periodic_resource->period);

			/* Call the periodic_handler function, which was checked during adding to list. */
			(periodic_resource->periodic_handler)();

			// Reset the timer with the new value
			return (ostick_t)periodic_resource->period;
		}
	}

	// Some error probably occurred, so stop the timer;
	// this only should get here if:
	// 1- The timer that timeouted was modified before done the loop above
	// 2- The period has been set to zero, in that case is no longer periodic
	return 0; // Do not repeat
}

void rest_engine_process(void)
{
  /* initialize the PERIODIC_RESOURCE timers, which will be handled by this process. */
  periodic_resource_t *periodic_resource = NULL;

  for(periodic_resource = (periodic_resource_t *)list_head(restful_periodic_services);
      periodic_resource;
      periodic_resource = periodic_resource->next)
  {
    if(periodic_resource->periodic_handler && periodic_resource->period) {
      PRINTF("Periodic: Set timer for /%s to %lu\n",
             periodic_resource->resource->url, periodic_resource->period);
	  OSTimerStart(periodic_resource->periodic_timer,  periodic_resource->period);
    }
  }
}


/*---------------------------------------------------------------------------*/
