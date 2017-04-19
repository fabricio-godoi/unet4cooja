/** System Includes **/
#include "tasks.h"
#include "BRTOS.h"
#include "debug_stack.h"
#include "leds.h"
#include "uart.h"

/** Configuration Files **/
#include "NetConfig.h"
#include "BoardConfig.h"

/** Function Prototypes **/
#include "unet_api.h"
#include "transport.h"
#include "unet_app.h"


/** Public/Private Variables **/

//extern BRTOS_Queue *Serial; // this should come from drivers
#if TASK_WITH_PARAMETERS == 1
void System_Time(void *parameters){
	(void)parameters;
#else
void System_Time(void){
#endif

	OSResetTime();
	OSResetDate();
	for (;;)
	{
		DelayTask(1000); // 1s
		OSUpdateUptime();
	}
}

#if 0
#include "OSInfo.h"
char big_buffer[20];

#if TASK_WITH_PARAMETERS == 1
void Task_Serial(void *parameters){
	(void) parameters;
#else
void Task_Serial(void){
#endif

	/* task setup */
	INT8U pedido = 0;

	// task main loop
	PRINTF("Started: task_serial\n");
	for (;;)
	{
		//	   PRINTF("Inside: task_serial\n");

		if(!OSQueuePend(Serial, &pedido, 0))
		{
			switch(pedido)
			{
#if (COMPUTES_CPU_LOAD == 1)
			case '1':
				acquireUART();
				OSCPULoad(big_buffer);
				PRINTF(big_buffer);
				releaseUART();
				break;
#endif
			case '2':
				acquireUART();
				OSUptimeInfo(big_buffer);
				PRINTF(big_buffer);
				releaseUART();
				break;
			case '3':
				acquireUART();
				PRINTF((CHAR8*)version);
				PRINTF("\n\r");
				releaseUART();
				break;
			case '4':
				acquireUART();
				OSAvailableMemory(big_buffer);
				PRINTF(big_buffer);
				releaseUART();
				break;
			case '5':
				acquireUART();
				OSTaskList(big_buffer);
				PRINTF(big_buffer);
				releaseUART();
				break;
#if (OSTRACE == 1)
			case '7':
				acquireUART();
				Send_OSTrace();
				PRINTF("\n\r");
				releaseUART();
				break;
#endif
			default:
				break;
			}
		}
	}
}
#endif


// TODO set benchmark to a specific module file
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
 *  7 - Set a fixed message size to be sent, use random time to send(?)
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


/**
 * Configure the max app payload length, this value is dependent of
 * the network layer, so check what is the maximum network payload length
 * before set this.
 * unet deafult: 35 bytes -> 128 - 35 = 93
 */
#define BENCHMARK_MSG_MAX_SIZE		90 //(spare for some possible overflows)
/**
 *  START_DELAY: value intended to wait network stabilization
 *  END_DELAY:   value intended to wait network buffer goes empty
 */
#define BENCHMARK_START_DELAY		 15000 // value in ms
#define BENCHMARK_END_DELAY			120000 // 2 seconds

#define BENCHMARK_MESSAGE "DEFAULT BENCHMARK PAYLOAD\0"
#define BENCHMARK_MESSAGE_LENGTH    26


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
	uint16_t msg_number;
	uint8_t  message[BENCHMARK_MESSAGE_LENGTH];
}Benchmark_Packet_Type;
#define BM_PACKET_SIZE (BENCHMARK_MESSAGE_LENGTH + 2)

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
	/// TODO check why rtable_up is getting addresses not stipulated (20:0E on 12 motes network)
	/// TODO1 could not replicate this behavior...
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
	// Task semaphore, wake on unet or uart command
	BRTOS_Sem *bm_sem;
	assert(OSSemCreate(0,&bm_sem) == ALLOC_EVENT_OK);
	UNET_Set_App_Callback(bm_sem);
	uart_callback(bm_sem);

	// Enable/Disable application communication
	uint8_t en_comm = 0; // default: disabled

	Benchmark_Control_Type bm_ctrl;
	Benchmark_Packet_Type bm_packet;

	/// Client will be woken when the message arrive on defined source port
	unet_transport_t client;
	client.src_port = Benchmark_Port;
	client.dst_port = Benchmark_Port;
	unet_connect(&client);


	// Wait notification from observer to start
//	while(!bm_ctrl.c.start) getchar(&bm_ctrl.ctrl, 0);
//	BM_PRINTF("server: Command 0x%02X received!\n",bm_ctrl.ctrl);
//
//	// Reset all statistics and start gathering information
//	NODESTAT_RESET();
//	en_comm = TRUE;

	/**
	 * NOTE:
	 *  Observer (simulator) will wait for every node to set the parent,
	 *  when it's all setup, the observer will start every mote individually.
	 */
	for(;;)
	{
		// Network or Serial reception
		if(!OSSemPend(bm_sem,0)){

			BM_PRINTF("Semaphore!\n");
			// Check for control messages
			if (getchar(&bm_ctrl.ctrl, NO_TIMEOUT) == READ_BUFFER_OK){
				BM_PRINTF("server: Command 0x%02X received!\n",bm_ctrl.ctrl);
				if(bm_ctrl.c.start) { en_comm = TRUE;     }
				if(bm_ctrl.c.stop)  { en_comm = FALSE;    }
				if(bm_ctrl.c.stats) { NODESTAT_ENABLE();  }
				else 				{ NODESTAT_DISABLE(); }
				if(bm_ctrl.c.reset) { NODESTAT_RESET();   }
			}

			// Wake on packet receive, or to check if there is some control message
			if (unet_recv(&client,(uint8_t *)&bm_packet,NO_TIMEOUT) == OK){
				// Do nothing, just clear the buffer
				BM_PRINTF("server: %d; %s; from: ",bm_packet.msg_number, bm_packet.message);
				BM_PRINTF_ADDR64(&client.sender_address);
			}
		}
	}
}
#endif
#if (UNET_DEVICE_TYPE == ROUTER)

#if (TASK_WITH_PARAMETERS == 1)
void unet_benchmark(void *param){
	(void)param;
#else
void unet_benchmark(void){
#endif
	// Enable/Disable application communication
	uint8_t en_comm = 0; // default: disabled

	// Packet to be sent
	Benchmark_Control_Type bm_ctrl;
	Benchmark_Packet_Type  bm_packet = { 0, {BENCHMARK_MESSAGE} };
//	bm_packet.msg_number = 0;
//	bm_packet.message = (uint8_t*)&BENCHMARK_MESSAGE;


	/// Set coordinator address
	addr64_t tasks_dest_addr64 = {.u8 = {PANID_INIT_VALUE,0x00,0x00,0x00,0x00,0x00,0x00}};
	unet_transport_t server;

	server.src_port = Benchmark_Port;
	server.dst_port = Benchmark_Port;
	server.dest_address = &tasks_dest_addr64;
	unet_listen(&server);

	// Wait network stabilization, when the parent is known
//	DelayTask(BENCHMARK_START_DELAY+2000); // starts 2 seconds after server
	DelayTask(1000); // wait unet network initialize
	while(node_data_get(NODE_PARENTINDEX) == NO_PARENT) DelayTask(500);
	// Notify the observer that is all ready to start
	// Wait observe response to actually initiate the process
	printf("BMCC_START\n");
	for(;;)
	{

		// Wake only the task on control received or timeout to send the message
		// TODO add random (?) .... yes, contiki use it as well...
//		if (unet_recv(&server,&bm_ctrl.ctrl,1000) >= 0){
		if (getchar(&bm_ctrl.ctrl, 1000) == READ_BUFFER_OK){
			BM_PRINTF("client: Command 0x%02X received!\n",bm_ctrl.ctrl);
			if(bm_ctrl.c.start) { en_comm = TRUE;     }
			if(bm_ctrl.c.stop)  { en_comm = FALSE;    }
			if(bm_ctrl.c.stats) { NODESTAT_ENABLE();  }
			else 				{ NODESTAT_DISABLE(); }
			if(bm_ctrl.c.reset) { NODESTAT_RESET();   }
		}

		// Send message to coordinator
		if(en_comm){
			BM_PRINTF("client: msg %d\n",bm_packet.msg_number);
			unet_send(&server,(uint8_t*)&bm_packet,BM_PACKET_SIZE,0);
			bm_packet.msg_number++;
		}
	}
}
#endif
