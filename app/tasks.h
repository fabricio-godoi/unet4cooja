/**
* \file tasks.h
* \brief Task function prototypes.
*
*
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
*                                              OS Tasks
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*********************************************************************************************************/

#include "BRTOSConfig.h"

#define System_Time_StackSize      192
#define UNET_Benchmark_StackSize   736

#define Benchmark_Port		222

#if TASK_WITH_PARAMETERS == 1
void System_Time(void *parameters);
void Task_Serial(void *parameters);
void unet_benchmark(void *param);
#else
void System_Time(void);
void Task_Serial(void);
void unet_benchmark(void);
#endif
