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
 *   Revision: 0.01
 *   Date:     08/01/2018
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
#include "NetConfig.h"
#include "BoardConfig.h"
#include "unet_api.h"   /* for UNET network functions */

/* Cooja */
#include "node_id.h"
#include "cooja.h"

/* CoAP Application Base Tetst */
#include "benchmark-coap.h"
#include "rest-engine.h"


#ifdef _SOMNIUM__
#define COMPILER "Somnium"
#else
#define COMPILER "Standard"
#endif

#define asmv(arg) __asm__ __volatile__(arg)


/** Tasks Handlers */
BRTOS_TH TH_SYSTEM;
BRTOS_TH TH_UNET_BM;

/**
 Main function
 */
volatile unsigned char status;
int main(void) {

	///////////////////////////////////////////////////
	// Start BRTOS tasks and configurations
	BRTOSInit();

	///////////////////////////////////////////////////
	// Start all mcu drives (leds, uart, spi, etc.)
	drivers_init();

	///////////////////////////////////////////////////
	// Start Cooja simulator dependencies
#ifdef COOJA_H_
	cooja_initialize();
#endif

#if DEBUGDCO_ENABLE==1
	debugdco_init();
#endif

#if 0
	char *compiler = COMPILER;
	PRINTF("Compiler: %s\n",compiler);
#endif


	/** uNet **/
//	PRITNF("Installing network...\n");
#if(NETWORK_ENABLE == 1)
	UNET_Init(); /* Install uNET tasks: Radio, Link, Ack Up/Down, Router Up/Down */
#endif

	/** System Tasks Installation **/
//	PRINTF("Installing system time...\n");
	assert(InstallTask(&System_Time, "System Time", System_Time_StackSize, SystemTaskPriority, &TH_SYSTEM) == OK);
//	PRINTF("Installing system timers and stacks...\n");

	// Timers for CoAP Application
#define TIMERS_STACKSIZE 1024
#define TIMER_PRIORITY   14
	OSTimerInit(TIMERS_STACKSIZE, TIMER_PRIORITY); // TODO validate stacksize and priority
//	PRINTF("System timers initialized\n");



//	PRINTF("Installing CoAP...\n");
	// TODO add CoAP tasks install
//	rest_init_engine(); // Initialize rest engine // this is only needed in the server
	/// TODO godoi check if this is needed only in the server side
//	PRINTF("CoAP initialized\n");

//	PRINTF("Installing benchmark...\n");
	/** System Tasks Installation **/
#if (TASK_WITH_PARAMETERS == 0)
	//// App tasks
	assert(InstallTask(&unet_benchmark, "Benchmark CoAP", UNET_Benchmark_StackSize, UNET_Benchmark_Priority, &TH_UNET_BM) == OK);
#else
	//// App tasks
	assert(InstallTask(&unet_benchmark, "Benchmark CoAP", UNET_Benchmark_StackSize, UNET_Benchmark_Priority, NULL, &TH_UNET_BM) == OK);
#endif


#ifdef COOJA_H_
	// Notify that the system is setup
	PRINTF("Node %d, addr: %02X:%02X:00:00:00:00:%02X:%02X\n",node_id,PANID_INIT_VALUE,MAC16_INIT_VALUE);
#endif

	/** Inicia OS **/
	printf("Starting BRTOS\n");
	BRTOSStart();

	/** Should never gets here */
	return 1;

}
