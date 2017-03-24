
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


void System_Time(void *param);
void pisca_led(void *param);
void pisca_led_net(void *param);
void UNET_App_1_Decode(void *param);
void make_path(void *param);
void led_activity(void *param);
void led_activity2(void *param);
void task_run_tests(void* param);

void Terminal_Task(void *);

void exec(void *);
void exec2(void *);
void exec3(void *);
