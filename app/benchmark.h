/*
 * benchmark.h
 *
 *  Created on: Apr 20, 2017
 *      Author: user
 */

#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <stdint.h>

/**
 * Configure the max app payload length, this value is dependent of
 * the network layer, so check what is the maximum network payload length
 * before set this.
 * unet deafult: 35 bytes -> 128 - 35 = 93
 * contiki ipv6 default: 40 bytes -> 127 - 40 = 87
 */
#define BENCHMARK_MSG_MAX_SIZE		80 //(spare for some possible overflows)

/**
 * Message that will be sent over network
 */
#define BENCHMARK_MESSAGE "DEFAULT BENCHMARK PAYLOAD\0"
#define BENCHMARK_MESSAGE_LENGTH    26

/**
 * Message that the client will sent to the controller
 * to inform that the parent has been set
 */
#define BENCHMARK_CLIENT_CONNECTED "BMCC_START\n"

/**
 * Message to inform that the client has sent
 * all the messages
 */
#define BENCHMARK_CLIENT_DONE	"BMCD_DONE\n"

/// Benchmark controls commands
enum{
	BMCC_NONE = 0,
	BMCC_START,
	BMCC_END,
	BMCC_RESET,
	BMCC_GET,
	BMCC_SET,

	// Do not remove, must be the last parameter
	BMCC_NUM_COMMANDS
};

/**
 * This is a control message structure for benchmark
 * start - will start the communication
 * stop  - even if the start is set, this will force it all to stop
 * stats - if set, the statistics will be enabled
 * reset - clear all statistics
 * shutdown - not implemented
 * get   - return the current control
 * set   - this is used to set the number of motes in the network and
 *         to configure the interval needed between transmission (MSB+LSB)
 *         (after control is expected 3 bytes - one for the number of motes
 *                                              two for the interval is ms)
 */
typedef union{
	uint8_t ctrl;
	struct{
		uint8_t start:1;		//<! Start communication
		uint8_t stop:1;			//<! Stop communication
		uint8_t stats:1;		//<! Enable/Disable statistics
		uint8_t reset:1;		//<! Reset the statistics
		uint8_t shutdown:1;		//<! Disable any kind of network (not implmentend)
		uint8_t get:1;			//<! Get parameter
		uint8_t set:1;			//<! Set parameter (num of motes and interval)
		uint8_t reserved:1;		//<! Reserved to ensure the usability of JavaScript
	}c;
}Benchmark_Control_Type;


// INFO setup to be with 32 bytes of data
typedef struct{
	uint16_t unused;
	uint16_t from;
	uint16_t msg_number;
	uint8_t  message[BENCHMARK_MESSAGE_LENGTH];
}Benchmark_Packet_Type;
#define BM_PACKET_SIZE (4 + 2 + BENCHMARK_MESSAGE_LENGTH)


#if BM_PACKET_SIZE > BENCHMARK_MSG_MAX_SIZE
#error "BM_PACK_SIZE is larger than permitted in a single packet!"
#endif

/**
 * Task parameters
 */
#define System_Time_StackSize      192
#define UNET_Benchmark_StackSize   736
#define Benchmark_Port		222

#if TASK_WITH_PARAMETERS == 1
void unet_benchmark(void *param);
#else
void unet_benchmark(void);
#endif



#endif /* BENCHMARK_H_ */
