/**
 * \file main.c
 * \brief main functions to test BRTOS with uNet under Cooja simulator
 **/
/*********************************************************************************************************
 *                                               BRTOS
 *                                Brazilian Real-Time Operating System
 *                            Acronymous of Basic Real-Time Operating System
 *
 *
 *                                  Open Source RTOS under MIT License
 *
 *
 *
 *                                           BRTOS for Cooja
 *
 *
 *   Authors:  Fabricio Negrisolo de Godoi
 *   Revision: 0.01       ,
 *   Date:     19/02/2017 ,
 *
 *********************************************************************************************************/
/* Library */
#include <signal.h>
#include <stdint.h>

/* MCU */
#include "drivers.h"
#include "debugdco.h"

/* BRTOS */
#include "BRTOS.h"
#include "tasks.h"

/* uNet */
#include "AppConfig.h"
#include "NetConfig.h"
#include "BoardConfig.h"
#include "unet_api.h"   /* for UNET network functions */

/* Cooja */
#include "node_id.h"
#include "cooja.h"

/** init CPU function */
/** fun��o de in�cio da CPU */
void mcu_init(void);

/** declara um ponteiro do evento sem�foro */
/** semaphore event pointer */
BRTOS_Sem *SEMTESTE;

/** Declara evento da serial */
BRTOS_Queue *Serial;
#ifdef _SOMNIUM__
#define COMPILER "Somnium"
#else
#define COMPILER "Standard"
#endif

#define asmv(arg) __asm__ __volatile__(arg)

/*---------------------------------------------------------------------------*/
void *w_memcpy(void *out, const void *in, size_t n) {
	uint8_t *src, *dest;
	src = (uint8_t *) in;
	dest = (uint8_t *) out;
	while (n-- > 0) {
		*dest++ = *src++;
	}
	return out;
}

void *w_memset(void *out, int value, size_t n) {
	uint8_t *dest;
	dest = (uint8_t *) out;
	while (n-- > 0) {
		*dest++ = value & 0xff;
	}
	return out;
}

/* TODO create a specific module to it (merge with assert?) */

extern const char *__msg_table[] = { "OK", "NO_MEMORY", "STACK_SIZE_TOO_SMALL",
		"END_OF_AVAILABLE_PRIORITIES", "BUSY_PRIORITY", "INVALID_TIME",
		"TIMEOUT", "CANNOT_ASSIGN_IDLE_TASK_PRIO", "NOT_VALID_TASK",
		"NO_TASK_DELAY", "END_OF_AVAILABLE_TCB", "EXIT_BY_NO_ENTRY_AVAILABLE",
		"TASK_WAITING_EVENT", "CANNOT_UNINSTALL_IDLE_TASK",
		"EXIT_BY_NO_RESOURCE_AVAILABLE" };

void __error_message(unsigned char _error_code) {
	switch (_error_code) {
	case OK :
		PRINTF("OK")
		;
		break;
	case NO_MEMORY :
		PRINTF("NO_MEMORY")
		;
		break;
	case STACK_SIZE_TOO_SMALL :
		PRINTF("STACK_SIZE_TOO_SMALL")
		;
		break;
	case END_OF_AVAILABLE_PRIORITIES :
		PRINTF("END_OF_AVAILABLE_PRIORITIES")
		;
		break;
	case BUSY_PRIORITY :
		PRINTF("BUSY_PRIORITY")
		;
		break;
	case INVALID_TIME :
		PRINTF("INVALID_TIME")
		;
		break;
	case TIMEOUT :
		PRINTF("TIMEOUT")
		;
		break;
	case CANNOT_ASSIGN_IDLE_TASK_PRIO :
		PRINTF("CANNOT_ASSIGN_IDLE_TASK_PRIO")
		;
		break;
	case NOT_VALID_TASK :
		PRINTF("NOT_VALID_TASK")
		;
		break;
	case NO_TASK_DELAY :
		PRINTF("NO_TASK_DELAY")
		;
		break;
	case END_OF_AVAILABLE_TCB :
		PRINTF("END_OF_AVAILABLE_TCB")
		;
		break;
	case EXIT_BY_NO_ENTRY_AVAILABLE :
		PRINTF("EXIT_BY_NO_ENTRY_AVAILABLE")
		;
		break;
	case TASK_WAITING_EVENT :
		PRINTF("TASK_WAITING_EVENT")
		;
		break;
	case CANNOT_UNINSTALL_IDLE_TASK :
		PRINTF("CANNOT_UNINSTALL_IDLE_TASK")
		;
		break;
	case EXIT_BY_NO_RESOURCE_AVAILABLE :
		PRINTF("EXIT_BY_NO_RESOURCE_AVAILABLE")
		;
		break;
	default:
		PRINTF("Error code not found!")
		;
		break;
	}
}

/********************/
/** Tasks Handlers **/
BRTOS_TH TH_SYSTEM;
BRTOS_TH TH_NET_APP1;
BRTOS_TH TH_NET_APP2;
BRTOS_TH TH_TERMINAL;
/********************/

#define RUN_TEST 0
#if RUN_TEST
extern void task_run_tests(void*);
extern void terminal_test(void);
#endif

/**
 Main function
 */
volatile unsigned char status;
int main(void) {

#if RUN_TEST
	terminal_test();
	task_run_tests(NULL);
#endif

	///////////////////////////////////////////////////
	// Start BRTOS tasks and configurations
	BRTOSInit();

	///////////////////////////////////////////////////
	// Start all mcu drives (leds, uart, spi, etc.)
	drivers_init();

	///////////////////////////////////////////////////
	// Start Cooja simulator dependencies
#ifdef _COOJA_H_
	cooja_initialize();
#endif

#if DEBUGDCO_ENABLE==1
	debugdco_init();
#endif

#if 0
	char *compiler = COMPILER;
	PRINTF("Compiler: %s\n",compiler);
#endif

	/** Cria evento de sem�foro - contador inicial 0 */
	/** Semaphore event - counter 0 */
	assert(OSSemCreate(0, &SEMTESTE) == ALLOC_EVENT_OK);
	/** Fila da porta serial - 16 bytes */
	assert(OSQueueCreate(32, &Serial) == ALLOC_EVENT_OK);


	/** uNet **/
#if(NETWORK_ENABLE == 1)
	UNET_Init(); /* Install uNET tasks: Radio, Link, Ack Up/Down, Router Up/Down */
#endif

	/** System Tasks Installation **/

	//// Common tasks between server and client
	assert(InstallTask(&System_Time, "System Time", System_Time_StackSize, SystemTaskPriority, &TH_SYSTEM) == OK);
//	assert(InstallTask(&Task_Serial,"Serial Handler",256,2,NULL) == OK);
	assert(InstallTask(&Terminal_Task, "Terminal Task", Terminal_StackSize,	Terminal_Priority, &TH_TERMINAL) == OK); //APP3_Priority

	//// App tasks
	assert(InstallTask(&pisca_led_net, "Blink LED Example", UNET_App_StackSize,	APP2_Priority, &TH_NET_APP2) == OK);

#if(NETWORK_ENABLE == 1) && 0
#if (TASK_WITH_PARAMETERS == 1)
	assert(InstallTask(&UNET_App_1_Decode,"Decode app 1 profiles",UNET_App_StackSize,APP1_Priority, NULL, &TH_NET_APP1) == OK);
#else
	assert(InstallTask(&UNET_App_1_Decode,"Decode app 1 profiles",UNET_App_StackSize,APP1_Priority, &TH_NET_APP1) == OK);
#endif
#endif


	// uNet PAN tester
#if 0
	extern void task_run_tests(void);
	assert(InstallTask(&task_run_tests,"Tests",UNET_App_StackSize,2, NULL) == OK);
#endif

	/** Inicia OS **/
	BRTOSStart();

	/** nunca chega aqui */
	/** never gets here */
	return 1;

}

