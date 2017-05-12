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
 *   Revision: 0.01       , 0.02
 *   Date:     19/02/2017 , 05/04/2017
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
#if(NETWORK_ENABLE == 1)
	UNET_Init(); /* Install uNET tasks: Radio, Link, Ack Up/Down, Router Up/Down */
#endif

	/** System Tasks Installation **/
#if (TASK_WITH_PARAMETERS == 0)
	//// Common tasks between server and client
	assert(InstallTask(&System_Time, "System Time", System_Time_StackSize, SystemTaskPriority, &TH_SYSTEM) == OK);
	//// App tasks
	assert(InstallTask(&unet_benchmark, "Benchmark", UNET_Benchmark_StackSize, UNET_Benchmark_Priority, &TH_UNET_BM) == OK);
#else
	//// Common tasks between server and client
	assert(InstallTask(&System_Time, "System Time", System_Time_StackSize, SystemTaskPriority, NULL, &TH_SYSTEM) == OK);
	//// App tasks
	assert(InstallTask(&unet_benchmark, "Benchmark", UNET_Benchmark_StackSize, UNET_Benchmark_Priority, NULL, &TH_UNET_BM) == OK);
#endif


#ifdef COOJA_H_
	// Notify that the system is setup
	PRINTF("Node %d, addr: %02X:%02X:00:00:00:00:%02X:%02X\n",node_id,PANID_INIT_VALUE,MAC16_INIT_VALUE);
#endif

	/** Inicia OS **/
	BRTOSStart();

	/** Should never gets here */
	return 1;

}
