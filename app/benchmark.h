/*
 * benchmark.h
 *
 *  Created on: Apr 20, 2017
 *      Author: user
 */

#ifndef BENCHMARK_H_
#define BENCHMARK_H_


/**
 * Configure how much packets the each node can receive in one second
 * in the application layer.
 * Theoretical value for 802.15.4:
 *   - Up to 250 kbps;
 *   - Packet size 128 bytes (1024 bits);
 *   - So it could receive 250 packets per second.
 *
 * Note: every network protocol has control messages, collision avoidance delay,
 *       time minimum between retries, and so on, so this value will never reach
 *       theoretical values.
 */
#define BENCHMARK_MAX_PACKETS_SECOND 1

/**
 * Define the longest path between clients and server
 * This value is determined by number of hops.
 *
 * This is used to manage the delay between packets as follow:
 * Time = number_of_clients * BENCHMARK_LONGEST_PATH * 150 + 30
 * Time: period between transmission
 * 150:  average time per hop
 * 30:   minimum delay (1 hop path)
 *
 * e.g. (root) <- (1) <- (2) <- (3)
 * BENCHMARK_LONGEST_PATH is 3
 */
#define BENCHMARK_LONGEST_PATH	12


/**
 * Configure the max app payload length, this value is dependent of
 * the network layer, so check what is the maximum network payload length
 * before set this.
 * unet deafult: 35 bytes -> 128 - 35 = 93
 */
#define BENCHMARK_MSG_MAX_SIZE		90 //(spare for some possible overflows)

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

typedef union{
	uint8_t ctrl;
	struct{
		uint8_t start:1;		//<! Start communication
		uint8_t stop:1;			//<! Stop communication
		uint8_t stats:1;		//<! Enable/Disable statistics
		uint8_t reset:1;		//<! Reset the statistics
		uint8_t shutdown:1;		//<! Disable any kind of network (not implmentend)
		uint8_t get:1;			//<! Get parameter (not implemented)
		uint8_t set:1;			//<! Set parameter (not implemented)
		uint8_t unused:1;
	}c;
}Benchmark_Control_Type;

typedef struct{
	uint32_t tick;
	uint16_t msg_number;
	uint8_t  message[BENCHMARK_MESSAGE_LENGTH];
}Benchmark_Packet_Type;
#define BM_PACKET_SIZE (4 + 2 + BENCHMARK_MESSAGE_LENGTH)

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
