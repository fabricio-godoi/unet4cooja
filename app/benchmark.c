/*
 * \file benchmark.c
 * \brief Network benchmark control
 * \author Fabricio Negrisolo de Godoi
 * \date 20-04-2017
 *
 */

/** System Includes **/
#include "tasks.h"
#include "BRTOS.h"
#include "uart.h"
#include "random.h"

/** Configuration Files **/
#include "NetConfig.h"
#include "BoardConfig.h"

/** Function Prototypes **/
#include "unet_api.h"
#include "transport.h"
#include "unet_app.h"

/**
 *  Benchmark specifications:
 *  1 - to start, all nodes must be connected at the network (parent set)
 *  2 - on start, all statistics must be clear
 *  3 - on end, all nodes must be notified, and the server must
 *      wait the last message arrive (if occur without error)
 *  4 - all network communication should stop when finishing the tests
 *      avoiding control messages errors and so on
 *  5 - a test must be performed to check the network stabilization time
 *      and messages lost, when all nodes started
 *  6 - Max neighborhood per mote must be checked in each simulation;
 *      RTABLE_UP_ENTRIES_MAX_NUM need to be configured as well;
 *  7 - Set a fixed message size to be sent, use random time to send
 *
 *
 *
 *
 *  TODO:
 *   - change the simulation time by every node has to
 *   send at least 'n' (100) messages through network;
 *   - rearrange the send speed, the network must send
 *   packets at the speed of the coordinator, e.g.
 *   if the coordinator can receive a packet every 100ms
 *   all network must generate just a packet at this time
 */

/** Define Benchmark debug messages */
//#define BM_DEBUG
#ifdef BM_DEBUG
#define BM_PRINTF(...) PRINTF(__VA_ARGS__)
#define BM_PRINTF_ADDR64(x)	print_addr64(x)
#else
#define BM_PRINTF(...)
#define BM_PRINTF_ADDR64(x)
#endif

// This enable/disable the counter to check the time
// between sending and receiving a message in tick counts
#define BM_CHK_TXRX_TIME_EN		FALSE

// Enable/Disable application communication
static uint8_t bm_en_comm = 0; // default: disabled
static uint8_t bm_num_of_nodes = 0;
static uint8_t bm_num_of_packets = 0;
static uint16_t bm_interval = 0;
static Benchmark_Control_Type bm_ctrl;
static uint8_t data[16]; // used to read serial
static uint16_t bm_pkts_recv[255]; // individual packets received count per client
static uint32_t stick = 0; // start tick from serial command
static void benchmark_parse_control(uint8_t data[]){
	bm_ctrl.ctrl = data[0];
	BM_PRINTF("benchmark: Command 0x%02X received!\n",bm_ctrl.ctrl);
	if(bm_ctrl.c.start) {
		bm_en_comm = TRUE;
#if (BM_CHK_TXRX_TIME_EN == TRUE)
		// Synchronization method between all nodes,
		// can only be accomplished by the simulator,
		// since it can send the same message to all
		// motes at once.
		stick = (uint32_t) OSGetTickCount();
#endif
//#if (UNET_DEVICE_TYPE == ROUTER)
//		OSDelayTask(10); // Wait some time to server startup
//#endif
	}
	if(bm_ctrl.c.stop)  { bm_en_comm = FALSE; }
	if(bm_ctrl.c.stats) { NODESTAT_ENABLE();  }
	else 				{ NODESTAT_DISABLE(); }
	if(bm_ctrl.c.reset) { NODESTAT_RESET();   }
	if(bm_ctrl.c.get)   { printf("%02X\n",bm_ctrl.ctrl); }
	if(bm_ctrl.c.set){
		bm_num_of_nodes = data[1];
		bm_num_of_packets = data[2];
		bm_interval = (uint16_t)(data[3]<<7) + data[4];
		BM_PRINTF("benchmark: Nodes: %d; Interval %d\n",bm_num_of_nodes,bm_interval);
//		if(getchar(&bm_num_of_nodes,0) == READ_BUFFER_OK){
//			printf("Number of motes: %u\n",bm_num_of_nodes);
//		}
	}
}

#if 0
/**
 * \brief Check if the addr is at the same PAN ID
 * \param addr 8 bytes address
 * \return 0 if isn't the same PAN ID, 1 otherwise
 */
static char isSamePan(uint8_t *addr){
	uint8_t* my_addr = node_addr64_get();
	if(addr[0] != my_addr[0] || addr[1] != my_addr[1] ) return 0;
	return 1;
}

#include "unet_router.h"
/**
 * \brief Send message to all nodes at the same PAN ID
 * \param message is the byte array that will be send
 * \param length is the length of the message
 */
static void notify_all (uint8_t *message, int length){
	unet_routing_table_up_t *table = unet_rtable_up_get();
	unet_transport_t destination;
	destination.src_port = Benchmark_Port;
	destination.dst_port = Benchmark_Port;

	char i;
	for(i=0; i < RTABLE_UP_ENTRIES_MAX_NUM && isSamePan((uint8_t *)&table[i].dest_addr64.u8); i++){
		destination.dest_address = (addr64_t *)&table[i].dest_addr64;
		unet_send(&destination,message,length,0);
	}
}
#endif

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
#if (TASK_WITH_PARAMETERS == 1)
void unet_benchmark(void *param){
	(void)param;
#else
void unet_benchmark(void){
#endif

	// Packet that will be received
	Benchmark_Packet_Type bm_packet = { 0, 0, "\0" };
#if (BM_CHK_TXRX_TIME_EN == TRUE)
	uint32_t tick;
#endif

	// Task semaphore, wake on unet or uart command
	BRTOS_Sem *bm_sem;
	assert(OSSemCreate(0,&bm_sem) == ALLOC_EVENT_OK);
	UNET_Set_App_Callback(bm_sem);
	uart_callback(bm_sem);

	/// Client will be woken when the message arrive on defined source port
	unet_transport_t client;
	client.src_port = Benchmark_Port;
	client.dst_port = Benchmark_Port;
	unet_connect(&client);

	// Notify the observer with some information
	printf("server: brtos unet\n");


	// Wait observe response to actually initiate the process
	gets(data, 0);
	benchmark_parse_control(data);
	assert(bm_num_of_nodes != 0);
	assert(bm_interval != 0);

	/**
	 * NOTE:
	 *  Observer (simulator) will wait for every node to set the parent,
	 *  when it's all setup, the observer will start every mote individually.
	 */
	uint16_t i;
	for(;;)
	{
		// Network or Serial reception
		if(OSSemPend(bm_sem, 0) == OK){

			// Check for control messages
			if (gets(data, NO_TIMEOUT) > 0){
//				bm_ctrl.ctrl = data[0];
				benchmark_parse_control(data);
				if(bm_ctrl.c.reset){
					for(i=0;i<=bm_num_of_nodes;i++) bm_pkts_recv[i] = 0;
				}
			}

			// Wake on packet receive, or to check if there is some control message
			if (unet_recv(&client,(uint8_t *)&bm_packet, NO_TIMEOUT) == OK){
#if (BM_CHK_TXRX_TIME_EN == TRUE)
				tick = (uint32_t) OSGetTickCount() - stick;
#endif
				// Make a individual count for each client
				bm_pkts_recv[client.sender_address.u8[7]]++;
				NODESTAT_UPDATE(apprxed);

#if (BM_CHK_TXRX_TIME_EN == TRUE)
				BM_PRINTF("server: delay to receive was %n; msg [%d] %s; from: %d\n",
						((uint32_t)tick-bm_packet.tick), bm_packet.msg_number,
						bm_packet.message,client.sender_address.u8[7]);
#else
				BM_PRINTF("server: msg [%d] recv from: %d\n",bm_packet.msg_number,client.sender_address.u8[7]);

				if(strcmp(bm_packet.message, BENCHMARK_MESSAGE) !=0 )	NODESTAT_UPDATE(corrupted);
#endif
			}
		}
	}
}
#endif

#if (UNET_DEVICE_TYPE == ROUTER)

/** Values in ms */
#define SEND_INTERVAL	1000
#define SEND_INTERVAL_MIN_DELAY 200

#if (TASK_WITH_PARAMETERS == 1)
void unet_benchmark(void *param){
	(void)param;
#else
void unet_benchmark(void){
#endif

	random_init(node_id);

	// Packet to be sent
	Benchmark_Packet_Type bm_packet = { 0, 1, {BENCHMARK_MESSAGE} };
//	bm_packet.msg_number = 0;
//	bm_packet.message = (uint8_t*)&BENCHMARK_MESSAGE;


	/// Set coordinator address
	addr64_t tasks_dest_addr64 = {.u8 = {PANID_INIT_VALUE,0x00,0x00,0x00,0x00,0x00,0x00}};
	unet_transport_t server;

	server.src_port = Benchmark_Port;
	server.dst_port = Benchmark_Port;
	server.dest_address = &tasks_dest_addr64;
	unet_listen(&server);


	// Wait observe response to actually initiate the process
	gets(data, 0);
//	bm_ctrl.ctrl = data[0];
	benchmark_parse_control(data);
	assert(bm_num_of_nodes != 0);
	assert(bm_interval != 0);

	//	ostick_t send_interval = bm_num_of_nodes*(1000/BENCHMARK_MAX_PACKETS_SECOND);
	ostick_t send_time; // Time to send
	ostick_t wait_time; // Minimum time to wait, complement send_time
	ostick_t ticked, tick;


	//	send_time = random_get()%send_interval;
	//	send_time = send_interval * node_id / bm_num_of_nodes;
	send_time = (ostick_t)(bm_interval/bm_num_of_nodes) * (bm_num_of_nodes - node_id); // start close from server
	if((ostick_t)send_time == 0) send_time = (ostick_t) NO_TIMEOUT;
	wait_time = 0; // Wait until start message arrives

	// Wait network stabilization, when the parent is known
	DelayTask(500); // wait unet network initialize
	while(node_data_get(NODE_PARENTINDEX) == NO_PARENT) DelayTask(500);
	// Notify the observer that is all ready to start
	printf(BENCHMARK_CLIENT_CONNECTED);

	// Counter for number of messages sent
	for(;;)
	{
		// Wake only the task on control received or timeout to send the message
		wait_serial:
		if(gets(data, wait_time) > 0){
//			bm_ctrl.ctrl = data[0];
			benchmark_parse_control(data);
			if(bm_ctrl.c.start){
				bm_packet.tick = 0;
				bm_packet.msg_number = 1;
			}
			if(bm_ctrl.c.stop) wait_time = 0;
			if(bm_ctrl.c.set){
				send_time = (ostick_t)(bm_interval/bm_num_of_nodes) * (bm_num_of_nodes - node_id); // start close from server;
			}


//			printf("send_time = %n\n",(uint16_t)send_time);
//			printf("wait_time = %n\n",(uint16_t)wait_time);
			// Delay the start of messages
			if(!bm_ctrl.c.stop && bm_ctrl.c.start && wait_time != send_time){
				wait_time = send_time;
				goto wait_serial;
			}
		}

		// Send message to coordinator
		if(bm_en_comm){
			ticked = (ostick_t) OSGetTickCount();

#if (BM_CHK_TXRX_TIME_EN == TRUE)
			bm_packet.tick = (uint32_t) ticked - stick;
#endif
			// Send the message
			unet_send(&server,(uint8_t*)&bm_packet, BM_PACKET_SIZE, 0);

			tick = (ostick_t) OSGetTickCount();

#if (BM_CHK_TXRX_TIME_EN == TRUE)
			BM_PRINTF("client: msg [%d] sent delay was %n\n",bm_packet.msg_number,(uint32_t)ticked-tick);
#else
			BM_PRINTF("client: msg [%d] sent\n",bm_packet.msg_number);
#endif

			NODESTAT_UPDATE(apptxed);
			bm_packet.msg_number++;
			if(bm_packet.msg_number > bm_num_of_packets){
				wait_time = 0; bm_en_comm = FALSE;
				printf(BENCHMARK_CLIENT_DONE);
			}
			else{
				wait_time =  (ostick_t)bm_interval -  (ostick_t)(tick - ticked);
//				printf("client: next delay %n\n",wait_time);
				if(wait_time == 0 || wait_time > bm_interval) wait_time = NO_TIMEOUT;
			}
		}

	}
}
#endif
