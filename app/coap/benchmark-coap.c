/*
 * \file benchmark.c
 * \brief Network benchmark control
 * \author Fabricio Negrisolo de Godoi
 * \date 08-01-2018
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
#include "benchmark-coap.h"

/** CoAP */
#include "er-coap-engine.h"
#include "res-benchmark.h"

/**
 *  Benchmark specifications:
 *  1 - to start, all nodes must be connected at the network (parent set)
 *  2 - on start, all statistics must be clear
 *  3 - on end, all nodes must be notified, and the server must
 *      wait the last message arrive (if occur without error)
 *  4 - the statistics must be paused when the tests are finished
 *  5 - it's used a fixed message with predefined length
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
static uint8_t bm_data[16]; // used to read serial
static uint16_t bm_pkts_recv[255]; // individual packets received count per client
#if (BM_CHK_TXRX_TIME_EN == TRUE)
static uint32_t stick = 0; // start tick from serial command
#endif
static void benchmark_parse_control(uint8_t bm_data[]){
	bm_ctrl.ctrl = bm_data[0];
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
		bm_num_of_nodes = bm_data[1];
		bm_num_of_packets = bm_data[2];
		bm_interval = (uint16_t)(bm_data[3]<<7) + bm_data[4]; // TODO need to get the last 2bits lost in communication method by JS
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

/*---------------------------------------------------------------------------*/
/**
 * CoAP configuration
 */

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t res_benchmark;

/*---------------------------------------------------------------------------*/

#if (TASK_WITH_PARAMETERS == 1)
void unet_benchmark(void *param){
	(void)param;
#else
void unet_benchmark(void){
#endif

	// Packet that will be received
	Benchmark_Packet_Type bm_packet = { 0, 0, 0, "\0" };
	coap_packet_t *coap_data;
	static uint16_t message_table[128];
	memset(message_table, 0, 128*sizeof(uint16_t)); // Clear block
#if (BM_CHK_TXRX_TIME_EN == TRUE)
	uint32_t tick;
#endif

	// Task semaphore, wake on coap or uart command
	BRTOS_Sem *bm_sem;
	assert(OSSemCreate(0,&bm_sem) == ALLOC_EVENT_OK);
	uart_callback(bm_sem);
	res_benchmark_set_callback(bm_sem);

	/**
	 * CoAP configuration
	 */

	/* Initialize the REST engine. */
	rest_init_engine();

	/* Install benchmark resource */
	rest_activate_resource(&res_benchmark, "");

	// Notify the observer with some information
	printf("server: brtos unet\n");

	// Wait observe response to actually initiate the process
	gets(bm_data, 0);
	benchmark_parse_control(bm_data);
	assert(bm_num_of_nodes != 0);
	assert(bm_interval != 0);

	/**
	 * NOTE:
	 *  Observer (simulator) will wait for every node to set the parent,
	 *  when it's all setup, the observer will start every mote individually.
	 */
	uint16_t i;// from;
	for(;;)
	{
		// Network or Serial reception
		if(OSSemPend(bm_sem, 0) == OK){

			// Check for control messages
			if (gets(bm_data, NO_TIMEOUT) > 0){
				benchmark_parse_control(bm_data);
				if(bm_ctrl.c.reset){
					for(i=0;i<=bm_num_of_nodes;i++){
						bm_pkts_recv[i] = 0;
						message_table[i] = 0;
					}
				}
			}
			// Wake on packet receive
			else if(res_benchmark_is_pend()){
#if (BM_CHK_TXRX_TIME_EN == TRUE)
				tick = (uint32_t) OSGetTickCount() - stick;
#endif
				res_benchmark_post();

				// Does not check if it's duplicated, just inform that has packet arrived
				NODESTAT_UPDATE(apprxed);

				// Get the arrived data
				coap_data = res_benchmark_get_data();
				memcpy(&bm_packet,(char *)coap_data->payload, BM_PACKET_SIZE);


				// TODO test a little more and delete it
//				if((coap_get_transaction_by_mid(coap_data->mid)->addr.u8[sizeof(addr64_t) - 1]) != (bm_packet.from))
//					printf("debug: addr diff: tran %d bm %d\n",(coap_get_transaction_by_mid(coap_data->mid)->addr.u8[sizeof(addr64_t) - 1]), bm_packet.from);
//				from = bm_packet.from;

				BM_PRINTF("[from %d] msg: [%d] %s\n",bm_packet.from,bm_packet.msg_number,bm_packet.message);
				if(bm_packet.msg_number > message_table[bm_packet.from]){
					BM_PRINTF("benchmark: msg: %u; table: %u; %s; from: %d\n",
							bm_packet.msg_number,
							message_table[bm_packet.from],
							bm_packet.message, bm_packet.from);

					message_table[bm_packet.from] = bm_packet.msg_number;
					// This ensure the number of packets arrived
					bm_pkts_recv[bm_packet.from]++;

					if(strcmp((char*)bm_packet.message, (char*)BENCHMARK_MESSAGE) != 0)	NODESTAT_UPDATE(corrupted);
				}
			}
		}
	}
}
#endif //(UNET_DEVICE_TYPE == PAN_COORDINATOR)

//#define UNET_DEVICE_TYPE ROUTER // TODO remove this latter, just for debug
#if (UNET_DEVICE_TYPE == ROUTER)

/*---------------------------------------------------------------------------*/
/**
 * CoAP configurations
 */
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

static coap_packet_t request[1]; /* This way the packet can be treated as pointer as usual. */

/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 5
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_urls[NUMBER_OF_URLS] =
{ ".well-known/core", "/actuators/toggle", "battery/", "error/in//path" , ""};


/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void client_chunk_handler(void *response)
{
	// Do nothing for now
	(void) response;
//  const uint8_t *chunk;
//  int len = coap_get_payload(response, &chunk);

  // Do nothing, since nothing is returned

//  printf("|%.*s", len, (char *)chunk);
}
/*---------------------------------------------------------------------------*/



/** Values in ms */
#define SEND_INTERVAL	1000
#define SEND_INTERVAL_MIN_DELAY 200

volatile ostick_t wait_time;

#if (TASK_WITH_PARAMETERS == 1)
void unet_benchmark(void *param){
	(void)param;
#else
void unet_benchmark(void){
#endif

	random_init(node_id);

	// Packet to be sent
	Benchmark_Packet_Type bm_packet = { 0, node_id, 1, {BENCHMARK_MESSAGE} };

	/// Set coordinator address
	addr64_t tasks_dest_addr64 = {.u8 = {PANID_INIT_VALUE,0x00,0x00,0x00,0x00,0x00,0x00}};
	unet_transport_t server;

	server.src_port = COAP_SERVER_PORT;
	server.dst_port = COAP_SERVER_PORT;
	server.dest_address = &tasks_dest_addr64;

	/**
	 * Configure CoAP
	 */
	/* receives all CoAP messages */
	coap_init_engine();

	// Wait observe response to actually initiate the process
	gets(bm_data, 0);
	benchmark_parse_control(bm_data);
	assert(bm_num_of_nodes != 0);
	assert(bm_interval != 0);

	//	ostick_t send_interval = bm_num_of_nodes*(1000/BENCHMARK_MAX_PACKETS_SECOND);
	ostick_t send_time; // Time to send
//	ostick_t wait_time; // Minimum time to wait, complement send_time
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
		if(gets(bm_data, wait_time) > 0){
			benchmark_parse_control(bm_data);
			if(bm_ctrl.c.start){
				bm_packet.from = node_id;
				bm_packet.msg_number = 1;
			}
			if(bm_ctrl.c.stop) wait_time = 0;
			if(bm_ctrl.c.set){
				send_time = (ostick_t)(bm_interval/bm_num_of_nodes) * (bm_num_of_nodes - node_id); // start close from server;
			}

			// Delay the start of messages
			if(!bm_ctrl.c.stop && bm_ctrl.c.start && wait_time != send_time){
				wait_time = send_time;
				goto wait_serial;
			}
		}

		// Send message to coordinator
		if(bm_en_comm){

			ticked = (ostick_t) OSGetTickCount();

			// Refresh the source of message
			bm_packet.from = node_id;

			// Send the message
			/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
			coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
			coap_set_header_uri_path(request, service_urls[4]);
			coap_set_payload(request, &bm_packet, BM_PACKET_SIZE);

			NODESTAT_UPDATE(apptxed);
			coap_blocking_request(&server,
							      request,
								  client_chunk_handler);

			tick = (ostick_t) OSGetTickCount();
#if (BM_CHK_TXRX_TIME_EN == TRUE)
			BM_PRINTF("client: msg [%d] sent delay was %n\n",bm_packet.msg_number,(uint32_t)(tick-ticked));
#else
			BM_PRINTF("client: msg [%d] sent\n",bm_packet.msg_number);
#endif

			bm_packet.msg_number++;
			if(bm_packet.msg_number > bm_num_of_packets){
				wait_time = 0; bm_en_comm = FALSE;
				printf(BENCHMARK_CLIENT_DONE);
			}
			else{
				wait_time = (ostick_t)bm_interval -  (ostick_t)(tick - ticked);
//				printf("client: next delay %n\n",wait_time);
				if(wait_time == 0 || wait_time > bm_interval) wait_time = NO_TIMEOUT;
//				wait_time = NO_TIMEOUT; // No delay between runs
			}
		}

	}
}
#endif
