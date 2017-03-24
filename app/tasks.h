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

#if TASK_WITH_PARAMETERS == 1
void System_Time(void *parameters);
void Task_Serial(void *parameters);
void pisca_led_net(void *param);
void UNET_App_1_Decode(void *param);
void pisca_led(void *param);
void Terminal_Task(void *p);
#else
void System_Time(void);
void Task_Serial(void);
void pisca_led_net(void);
void UNET_App_1_Decode(void);
void pisca_led(void);
void Terminal_Task(void);
#endif
