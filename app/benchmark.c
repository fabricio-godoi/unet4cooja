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
#define BM_DEBUG
#ifdef BM_DEBUG
#define BM_PRINTF(...) PRINTF(__VA_ARGS__)
#define BM_PRINTF_ADDR64(x)	print_addr64(x)
#else
#define BM_PRINTF(...)
#define BM_PRINTF_ADDR64(x)
#endif


// Enable/Disable application communication
static uint8_t bm_en_comm = 0; // default: disabled
static uint8_t bm_num_of_nodes = 0;
static Benchmark_Control_Type bm_ctrl;
static uint8_t data[16]; // used to read serial
static uint16_t bm_pkts_recv[255]; // individual packets received count per client

static void benchmark_parse_control(void){
	BM_PRINTF("benchmark: Command 0x%02X received!\n",bm_ctrl.ctrl);
	if(bm_ctrl.c.start) { bm_en_comm = TRUE; OSCountReset(); }
	if(bm_ctrl.c.stop)  { bm_en_comm = FALSE; }
	if(bm_ctrl.c.stats) { NODESTAT_ENABLE();  }
	else 				{ NODESTAT_DISABLE(); }
	if(bm_ctrl.c.reset) { NODESTAT_RESET();   }
	if(bm_ctrl.c.get)   { printf("%02X\n",bm_ctrl.ctrl); }
//	if(bm_ctrl.c.set){
//		if(getchar(&bm_num_of_nodes,0) == READ_BUFFER_OK){
//			printf("Number of motes: %u\n",bm_num_of_nodes);
//		}
//	}
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
	ostick_t ticked, tick;

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
	printf("server: brtos unet %u\n",BENCHMARK_MAX_PACKETS_SECOND);


	// Wait observe response to actually initiate the process
	gets(data, 0);
	bm_ctrl.ctrl = data[0];
	benchmark_parse_control();
	if(bm_ctrl.c.set){
		bm_num_of_nodes = data[1];
		BM_PRINTF("Number of motes: %u\n",bm_num_of_nodes);
	}
	assert(bm_num_of_nodes != 0);


	/**
	 * NOTE:
	 *  Observer (simulator) will wait for every node to set the parent,
	 *  when it's all setup, the observer will start every mote individually.
	 */
	for(;;)
	{
		// Network or Serial reception
		if(OSSemPend(bm_sem, 0) == OK){

			// Check for control messages
			if (gets(data, NO_TIMEOUT) > 0){
				bm_ctrl.ctrl = data[0];
				benchmark_parse_control();
			}

			// Wake on packet receive, or to check if there is some control message
			if (unet_recv(&client,(uint8_t *)&bm_packet, NO_TIMEOUT) == OK){
				tick = OSGetTickCount();
				// Make a individual count for each client
				bm_pkts_recv[client.sender_address.u8[7]]++;
				NODESTAT_UPDATE(apprxed);
				BM_PRINTF("tick: %n\n",(uint32_t)tick);
				BM_PRINTF("server: %n; %d; %s; from: %d\n",
						((uint32_t)tick-bm_packet.tick), bm_packet.msg_number,
						bm_packet.message,client.sender_address.u8[7]);
//				BM_PRINTF_ADDR64(&client.sender_address);
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
	bm_ctrl.ctrl = data[0];
	benchmark_parse_control();
	if(bm_ctrl.c.set){
		bm_num_of_nodes = data[1];
		BM_PRINTF("Number of motes: %u\n",bm_num_of_nodes);
	}
	assert(bm_num_of_nodes != 0);
	//	ostick_t send_interval = bm_num_of_nodes*(1000/BENCHMARK_MAX_PACKETS_SECOND);
	ostick_t send_interval = (ostick_t)(bm_num_of_nodes) * (BENCHMARK_LONGEST_PATH*150) + 30;
	ostick_t send_time; // Time to send
	ostick_t wait_time; // Minimum time to wait, complement send_time
	ostick_t ticked, tick;


	//	send_time = random_get()%send_interval;
	//	send_time = send_interval * node_id / bm_num_of_nodes;
	send_time = (ostick_t)(bm_num_of_nodes - node_id) * (BENCHMARK_LONGEST_PATH*150) + 30; // bigger ids first - 3 seconds for each mote
	wait_time = 0;

	// Wait network stabilization, when the parent is known
	DelayTask(500); // wait unet network initialize
	while(node_data_get(NODE_PARENTINDEX) == NO_PARENT) DelayTask(500);
	// Notify the observer that is all ready to start
	printf(BENCHMARK_CLIENT_CONNECTED);

	for(;;)
	{
		// Wake only the task on control received or timeout to send the message
		if(gets(data, wait_time) > 0){
			bm_ctrl.ctrl = data[0];
			benchmark_parse_control();
			if(bm_ctrl.c.start){
				// Delay the start of messages
				if(gets(data, send_time) > 0){
					bm_ctrl.ctrl = data[0];
					benchmark_parse_control();
				}
//				BM_PRINTF("benchmark: started\n");
			}
			if(bm_ctrl.c.stop) wait_time = 0;
		}

		// Send message to coordinator
		if(bm_en_comm){
			ticked = (ostick_t) OSGetTickCount();
			bm_packet.tick = (uint32_t) ticked;
			unet_send(&server,(uint8_t*)&bm_packet, BM_PACKET_SIZE, 0);
			tick = (ostick_t) OSGetTickCount();
			BM_PRINTF("client: msg %d - %n\n",bm_packet.msg_number,(uint32_t)ticked);

			NODESTAT_UPDATE(apptxed);
			bm_packet.msg_number++;
			wait_time =  (ostick_t)send_interval -  (ostick_t)(tick - ticked);
			if(wait_time <= 0 || wait_time > send_interval) wait_time = NO_TIMEOUT;
		}

	}
}
#endif


