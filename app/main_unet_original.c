/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */
#include <stdint.h>

/* MCU and OS includes */
#include "BRTOS.h"

/* Config. files */
#include "AppConfig.h"
#include "NetConfig.h"
#include "BoardConfig.h"
#include "tasks_unet_original.h"        /* for tasks prototypes */
#include "unet_api.h"   /* for UNET network functions */

BRTOS_TH TH_SYSTEM;
BRTOS_TH TH_NET_APP1;
BRTOS_TH TH_NET_APP2;
BRTOS_TH TH_TERMINAL;

#define RUN_TEST 0
#if RUN_TEST
	extern void task_run_tests(void*);
	extern void terminal_test(void);
#endif


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main_app(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{

  #if RUN_TEST
	terminal_test();
	task_run_tests(NULL);
  #endif

  UserEnterCritical();

  // Initialize BRTOS
  BRTOS_Init();

  #if (PROCESSOR == ARM_Cortex_M0)
  // Iniciar UART para utilizar com printf
  Init_UART0(115200,128);
  #endif

  ////////////////////////////////////////////////////////

  if(InstallTask(&System_Time,"System Time",System_Time_StackSize,SystemTaskPriority, NULL, &TH_SYSTEM) != OK)
  {
    while(1){};
  };

#if(NETWORK_ENABLE == 1)
  UNET_Init();      /* Install uNET tasks: Radio, Link, Ack Up/Down, Router Up/Down */
#endif

#if (PROCESSOR == ARM_Cortex_M0)
#if (UNET_DEVICE_TYPE == PAN_COORDINATOR) && 1
  if(InstallTask(&pisca_led_net,"Blink LED Example",UNET_App_StackSize,APP2_Priority, NULL, &TH_NET_APP2) != OK)
  {
    while(1){};
  };
#else
  if(InstallTask(&UNET_App_1_Decode,"Decode app 1 profiles",UNET_App_StackSize,APP1_Priority, NULL, &TH_NET_APP1) != OK)
  {
    while(1){};
  };
#endif
#endif

  #if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
  if(InstallTask(&Terminal_Task,"Terminal Task",Terminal_StackSize,Terminal_Priority, NULL, &TH_TERMINAL) != OK)
  {
    while(1){};
  };
  #else
  if(InstallTask(&Terminal_Task,"Terminal Task",Terminal_StackSize,APP3_Priority, NULL, &TH_TERMINAL) != OK)
  {
    while(1){};
  };
  #endif

  #if(NETWORK_ENABLE == 1) && 0
  if(InstallTask(&UNET_App_1_Decode,"Decode app 1 profiles",UNET_App_StackSize,APP1_Priority, NULL, &TH_NET_APP1) != OK)
  {
    while(1){};
  };

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
  if(InstallTask(&pisca_led_net,"Blink LED Example",UNET_App_StackSize,APP2_Priority, NULL, &TH_NET_APP2) != OK)
  {
    while(1){};
  };
#endif

#if (UNET_DEVICE_TYPE == ROUTER)
#if (ROUTER_TYPE == ROUTER1)
  if(InstallTask(&pisca_led_net,"Blink LED Example",UNET_App_StackSize,APP2_Priority, NULL, &TH_NET_APP2) != OK)
  {
    while(1){};
  };
#endif

#if (ROUTER_TYPE == ROUTER2)
  if(InstallTask(&make_path,"Make up path",UNET_App_StackSize,APP2_Priority, NULL, &TH_NET_APP2) != OK)
  {
    while(1){};
  };
#endif
#endif
#endif

#if 0
  if(InstallTask(&led_activity,"Blink LED for activity",UNET_App_StackSize,20, NULL, NULL) != OK)
  {
    while(1){};
  };
#endif

#if 0
  if(InstallTask(&led_activity2,"Blink LED for activity 2",UNET_App_StackSize,21, NULL, NULL) != OK)
  {
    while(1){};
  };
#endif


#if 0
  if(InstallTask(&task_run_tests,"Tests",UNET_App_StackSize,2, NULL, NULL) != OK)
  {
    while(1){};
  };
#endif

  // Start Task Scheduler
  if(BRTOSStart() != OK)
  {
    for(;;){};
  };  

  return 0;

}

/* END main */
/*!
** @}
*/
