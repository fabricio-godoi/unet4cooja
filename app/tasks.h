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

#ifndef TASKS_H_
#define TASKS_H_

#include "BRTOSConfig.h"
#include "benchmark.h"

#if TASK_WITH_PARAMETERS == 1
void System_Time(void *parameters);
void Task_Serial(void *parameters);
#else
void System_Time(void);
void Task_Serial(void);
#endif

#endif
