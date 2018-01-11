/*
 * res-benchmark.h
 *
 *  Created on: Jan 8, 2018
 *      Author: user
 */

#ifndef APP_COAP_RESOURCES_RES_BENCHMARK_H_
#define APP_COAP_RESOURCES_RES_BENCHMARK_H_

/** Set a semaphore to be called back */
void res_benchmark_set_callback(BRTOS_Sem *c);

uint8_t res_benchmark_is_pend();
void res_benchmark_post(void);

/** Get the data from the resource */
void* res_benchmark_get_data();

#endif /* APP_COAP_RESOURCES_RES_BENCHMARK_H_ */
