/** System Includes **/
#include "tasks.h"
#include "BRTOS.h"
#include "debug_stack.h"
#include "leds.h"

/** Configuration Files **/
#include "NetConfig.h"
#include "BoardConfig.h"

/** Function Prototypes **/
#include "unet_api.h"
#include "transport.h"
#include "unet_app.h"


/** Public/Private Variables **/

extern BRTOS_Queue *Serial;
#if TASK_WITH_PARAMETERS == 1
void System_Time(void *parameters)
#else
void System_Time(void)
#endif
{
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
void Task_Serial(void *parameters)
#else
void Task_Serial(void)
#endif
{
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


#define BENCHMARK_MSG_MAX_SIZE		90    // TODO change to payload max length
#define BENCHMARK_START_DELAY		20000 // value in ms


#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)

#if (TASK_WITH_PARAMETERS == 1)
void unet_benchmark(void *param){
	(void)param;
#else
void unet_benchmark(void){
#endif

	/// Client will be woken when the message arrive on defined source port
	unet_transport_t client;
	client.src_port = 222;
	client.dst_port = 221;
	unet_connect(&client);

	uint8_t message[BENCHMARK_MSG_MAX_SIZE];

	// Wait network stabilization
	DelayTask(BENCHMARK_START_DELAY);
	// Reset all statistics and start gathering information
	NODESTAT_RESET();
	for(;;)
	{


		/**
		 *  1 - to start, all nodes must be connected at the network (parent set)
		 *  2 - on start, all statistics must be clear
		 *  3 - on end, all nodes must be notified, and the server must
		 *      wait the last message arrive (if occur without error)
		 *  4 - all network communication should stop when finishing the tests
		 *      avoiding control messages errors and so on
		 *  5 - a test must be performed to check the network stabilization time
		 *      and messages lost, when all nodes started
		 */


		// Wake only packet received
		if (unet_recv(&client,message,0) >= 0){
			printf("tasks:;%s;addr: ",message);
			print_addr64(&client.sender_address);
		}
//		DelayTask(200);
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
	/// Set coordenator address
	addr64_t tasks_dest_addr64 = {.u8 = {PANID_INIT_VALUE,0x00,0x00,0x00,0x00,0x00,COORDINATOR_ID}};
	unet_transport_t server;

	server.src_port = 221;
	server.dst_port = 222;
	server.dest_address = &tasks_dest_addr64;
	unet_listen(&server);

	unsigned char message[BENCHMARK_MSG_MAX_SIZE];
	unsigned char size;
	unsigned int  number = 1;

	// Wait network stabilization
	DelayTask(BENCHMARK_START_DELAY+2000); // starts 2 seconds after server
	while(node_data_get(NODE_PARENTINDEX) == NO_PARENT) DelayTask(5000);
	for(;;)
	{

		// Envia mensagem para o coordenador
		size = (char) sprintf((char*)message,"Hello %d;from %d", number, node_id);
//		printf("> %s\n",message);
		if(++number > 999) number = 1;

		unet_send(&server,message,size,0);

		DelayTask(1000);
	}
}
#endif
