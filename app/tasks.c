/** System Includes **/
#include "tasks.h"
#include "BRTOS.h"
#include "debug_stack.h"
#include "leds.h"



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

