
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


/* MCU and OS includes */
#include "tasks_unet_original.h"

#include "BRTOS.h"
#include <string.h>

/* Config. files */
//#include "BRTOSConfig.h"
#include "AppConfig.h"
#include "NetConfig.h"
#include "BoardConfig.h"

/* Function prototypes */
#include "unet_api.h"
#include "transport.h"
#include "app.h"
#include "unet_app.h"


#if DEBUG_PRINTF && WINNT
//#include "stdio.h"
//#define PRINTF(...) printf(__VA_ARGS__); fflush(stdout);
#elif DEBUG_PRINTF
//#include <stdio.h>
//#define PRINTF(...) printf(__VA_ARGS__); fflush(stdout);
#else
#define PRINTF(...)
#endif


/*************************************************/
/* Task: keep watchdog happy and system time     */
/*************************************************/
void System_Time(void *param)
{
   /* task setup */
   INT16U i = 0;

   (void)param;
   OSResetTime();
   
#if (WATCHDOG == 1)
#endif
  
   /* task main loop */
   for (;;)
   {

	   #if (WATCHDOG == 1)
	   /* Feed WDT counter */
		#if PROCESSOR==COLDFIRE_V1
			   __RESET_WATCHDOG();
		#endif
	   #endif

      DelayTask(25); /* wait 10 ticks -> 10 ms */

      #if RADIO_DRIVER_WATCHDOG == 1
           //Radio_Count_states();
      #endif
      
      i++;
      if (i >= 40)
      {
        OSUpdateUptime();
        i = 0;
      }
   }
}

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR) && 1
void pisca_led_net(void *param)
{
	(void)param;
	unet_transport_t client;
	addr64_t dest_addr64 = {.u8 = {0x47,0x42,0x00,0x00,0x00,0x00,0x12,0x34}};
	uint8_t packet[8];
	uint8_t response[32];
	APP_PACKET *app_payload;

	app_payload = (APP_PACKET*)packet;

	DelayTask(5000);

	// !!!! D�vida,
	client.src_port = 222;
	client.dst_port = 221;
	client.dest_address = &dest_addr64;
	unet_connect(&client);

	app_payload->APP_Profile = GENERAL_PROFILE;
	app_payload->APP_Command = GENERAL_ONOFF;

	for(;;)
	{
		// Envia mensagem para o client
		app_payload->APP_Command_Attribute = TRUE;
		unet_send(&client, packet, 3,30);
		if (unet_recv(&client,response,1000) >= 0)
		{
			PRINTF_APP(1,"Packet received from (port/address):  %d/",client.sender_port); PRINTF_APP_ADDR64(1,&(client.sender_address));
			PRINTF_APP(1,"Packet Content: %s\n\r",response);
			DelayTask(5000);
		}else{
			DelayTask(10000);
		}
		app_payload->APP_Command_Attribute = FALSE;
		unet_send(&client, packet, 3,30);
		if (unet_recv(&client,response,1000) >= 0)
		{
			PRINTF_APP(1,"Packet received from (port/address):  %d/",client.sender_port); PRINTF_APP_ADDR64(1,&(client.sender_address));
			PRINTF_APP(1,"Packet Content: %s\n\r",response);
			DelayTask(5000);
		}else{
			DelayTask(10000);
		}
	}
}
#endif


void UNET_App_1_Decode(void *param)
{
   (void)param;
   unet_transport_t server;
   uint8_t packet[8];
   char *response_packet = "Pacote recebido com sucesso!";
   APP_PACKET *app_payload;

   app_payload = (APP_PACKET*)packet;

#if defined BRTOS_PLATFORM
#if BRTOS_PLATFORM == FRDM_KL25Z
	// Enables the port clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	GPIOPinConfigure(GPIO_PB18_PB18);

	// Enables Drive Strength
	GPIOPadConfigSet (GPIOB_BASE, GPIO_PIN_18, PORT_TYPE_DSE_HIGH);

	// Set port for LED to output
	GPIOPinSet(GPIOB_BASE, GPIO_PIN_18);
	xGPIODirModeSet(GPIOB_BASE, GPIO_PIN_18, xGPIO_DIR_MODE_OUT);
#endif
#endif

	server.src_port = 221;
	server.dst_port = 222;
	// Escuta porta 221
	unet_listen(&server);

   /* task main loop */
   for (;;)
   {
       /* Wait event from APP layer, with or without timeout */
	   (void)unet_recv(&server,packet,0);
	   PRINTF_APP(1,"Packet received from (port/address):  %d/",server.sender_port);
	   PRINTF_APP_ADDR64(1,&(server.sender_address));
       switch(app_payload->APP_Profile)
       {
        	case GENERAL_PROFILE:
        		Decode_General_Profile(app_payload);
        		break;

        	default:
        		break;
       }
       if (&(server.sender_address) != NULL){
    	   if (memcmp(&(server.sender_address),node_addr64_get(),2) == 0){
    		   server.dest_address = &(server.sender_address);
    		   server.dst_port = server.sender_port;
    		   // Fazer v�rias tentativas, pois pode estar respondendo
    		   // um comando do terminal e o pacote down estar ocupado
	    	   unet_send(&server, (uint8_t *)response_packet, 29,30);
    	   }
       }
   }
}

#if (UNET_DEVICE_TYPE == ROUTER)
#if (ROUTER_TYPE == ROUTER1)
void pisca_led_net(void *param)
{
	(void)param;
	addr64_t dest_addr64 = {.u8 = {0x47,0x42,0x00,0x00,0x00,0x00,0x12,0x34}};
	for(;;)
	{
		// Envia mensagem para o coordenador
		NetGeneralONOFF(TRUE, &dest_addr64);
#if (CONTIKI_MAC_ENABLE == 1)
		DelayTask(3010);
#else
		DelayTask(1000);
#endif
		NetGeneralONOFF(FALSE, &dest_addr64);
#if (CONTIKI_MAC_ENABLE == 1)
		DelayTask(3010);
#else
		DelayTask(1000);
#endif
	}
}
#endif

#if (ROUTER_TYPE == ROUTER2)
void make_path(void *param)
{
	(void)param;
	for(;;)
	{
		// Envia mensagem para o coordenador
		NetGeneralCreateUpPath();
		DelayTask(5000);
	}
}
#endif
#endif


#if 0
void UNET_App_1_Decode(void *param)
{
#if (defined BOOTLOADER_ENABLE) && (BOOTLOADER_ENABLE==1)
   INT8U  ret = 0;
#endif

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
   char buffer[8]; (void) buffer;
#endif
   (void)param;

	#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
    // Init serial driver
	//Init_UART0();
	#endif

   /* task main loop */
   for (;;)
   {

      /* Wait event from APP layer, with or without timeout */
#if (defined BOOTLOADER_ENABLE) && (BOOTLOADER_ENABLE==1)
	      ret = OSSemPend(SIGNAL_APP1, SIGNAL_TIMEOUT);
#else
	      (void)OSSemPend(SIGNAL_APP1, 0);
#endif

      #if (defined BOOTLOADER_ENABLE) && (BOOTLOADER_ENABLE==1)
      if(ret != TIMEOUT){
      #endif

       acquireRadio();

       switch(app_packet.APP_Profile)
       {
        case GENERAL_PROFILE:
		  #if (UNET_DEVICE_TYPE == PAN_COORDINATOR) && 0
          (void)UARTPutString(UART0_BASE, "Pacote do perfil geral recebido do n� ");
          (void)UARTPutString(UART0_BASE, PrintDecimal(nwk_packet.NWK_Source, buffer));\
          (void)UARTPutString(UART0_BASE, "\n\r");
		  #endif
          Decode_General_Profile();
          break;

        case LIGHTING_PROFILE:
		  #if (UNET_DEVICE_TYPE == PAN_COORDINATOR) && 0
          (void)UARTPutString(UART0_BASE, "Pacote do perfil lighting recebido!\n\r");
		  #endif
          //Decode_Lighting_Profile();
          break;       

        #if (defined BOOTLOADER_ENABLE) && (BOOTLOADER_ENABLE==1)
        case BULK_DATA_PROFILE:
          WBootloader_Handler();
          break;
        #endif

        default:
		  #if (UNET_DEVICE_TYPE == PAN_COORDINATOR) && 0
          (void)UARTPutString(UART0_BASE, "Pacote de dados gen�rico recebido!\n\r");
		  #endif
          break;
       }
       releaseRadio();

      #if (defined BOOTLOADER_ENABLE) && (BOOTLOADER_ENABLE==1)
      }else{
        WBootloader_Handler_Timeout();
      }
      #endif

   }
}

#endif

void pisca_led(void *param)
{
	CHAR8 *test_pointer = "Teste";
	CHAR8 data_test[6];
	int status = 0;
	int i = 0;

	(void)param;

	// Inicializa o m�dulo de mem�ria FLASH
	//InitFlash();

	// L� 5 bytes da mem�ria FLASH
	//ReadFromFlash(0x1F040, data_test, 5);

	data_test[5]=0;
	status = strcmp(test_pointer, data_test);

	// Verifica se a FLASH j� havia sido escrita
	if (status)
	{
		// Escreve 5 bytes na mem�ria FLASH
		//WriteToFlash(test_pointer, 0x1F040, 5);
	}

	for(;;)
	{
		for(i=0;i<6;i++)
		{
			data_test[i] = 0;
		}

		//ReadFromFlash(0x1F040, data_test, 5);

		status = strcmp(test_pointer, data_test);

		// S� pisca o outro LED se a FLASH leu o valor correto
		if (!status)
		{
		}
		DelayTask(200);

		if (!status)
		{
		}
		DelayTask(200);
	}
}

void led_activity(void *param){
	int flag = 0;
	(void)param;
	
	while(1)
	{
		DelayTask(100);
		if (flag)
		{
			flag = 0;
		}else{
			flag = 1;
		}
		#if WINNT
			PRINTF("Tick Count: %u\r\n", (uint32_t)OSGetTickCount());
			fflush(stdout);
		#endif
	}
}

void led_activity2(void *param){
	int flag = 0;
	(void)param;

	while(1)
	{
		DelayTask(100);
		if (flag)
		{
			flag = 0;
		}else{
			flag = 1;
		}
		#if WINNT
			PRINTF("Run LED task 2\r\n");
			fflush(stdout);
		#endif
	}
}

/** \defgroup app_terminal	Terminal de Comandos
 *  @{
 * Tarefa para processamento de comandos recebidos por terminal.
 */
#include "terminal.h"

static void terminal_init(void)
{
	#if defined BRTOS_PLATFORM
	#if (BRTOS_PLATFORM == FRDM_KL25Z)
	#elif (BRTOS_PLATFORM == BRTOS_COLDUINO)
		(void) cdc_init(TERM_BUFSIZE); /* Initialize the USB CDC Application */
		printf_install_putchar((void*)putchar_usb);
	#endif
	#endif
}

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
void Terminal_Task(void *p)
{
	uint8_t data = '\0';
	(void)p;
	terminal_init();

	while(1)
	{
		#if (BRTOS_PLATFORM == FRDM_KL25Z)
		UARTGetChar(UART0_BASE,&data,0);
		UARTPutChar(UART0_BASE,data);
		#endif
		if (terminal_input(data)) terminal_process();
	}
}
#else
//char *teste = "Comando executado com sucesso!\n\r";
#if 1

unet_transport_t *server_putchar = NULL;
char term_buffer[MAX_APP_PAYLOAD_SIZE];
static int term_buffer_size = 0;
char unet_putchar(char c)
{

	term_buffer[term_buffer_size++%MAX_APP_PAYLOAD_SIZE] = c;
	if(term_buffer_size == MAX_APP_PAYLOAD_SIZE || c == ETX)
	{
		if(server_putchar != NULL)
		{
			while(unet_send(server_putchar, term_buffer, term_buffer_size, 21) < 0)
			{
				OSDelayTask(250);
			}
		}
		term_buffer_size = 0;
		memset(term_buffer, 0x00, sizeof(term_buffer));
	}
	return c;
}
#endif

void Terminal_Task(void *p)
{
	unet_transport_t server;
	uint8_t		payload_rx[16];
	//uint8_t   	payload_tx[512];
	uint8_t		*cmd;
	uint8_t		*ret;
	int 		idx = 0;
	int 		size = 0;
	(void)p;

	server.src_port = 23;
	server.dst_port = 22;
	// Escuta porta 23
	unet_listen(&server);

	/* */
	server_putchar = &server;

	while(1)
	{
		(void)unet_recv(&server,payload_rx,0);
	    server.dest_address = &(server.sender_address);
	    server.dst_port = server.sender_port;
		payload_rx[server.payload_size] = '\n';
		cmd = payload_rx;
		while(*cmd)
		{
			if(terminal_input(*cmd++) == 1)
			{
				*cmd = '\0';

#if (BRTOS_PLATFORM == BOARD_ROTEADORCFV1)
				printf_install_putchar(unet_putchar);
#endif
				ret = (uint8_t*)terminal_process();

#if BRTOS_PLATFORM == BOARD_ROTEADORCFV1
				if(term_buffer_size != 0) unet_putchar(ETX); /* Send of text char */
				printf_install_putchar(NULL);
				break;
#endif
				uint8_t	*packet = ret;
			    size = 0;
			    idx = 0;
			    while(*packet){
			    	packet++;
			    	size++;
			    }
			    // Envia o fim de string
			    size++;

			    do{
			    	if (size <= MAX_APP_PAYLOAD_SIZE)
			    	{
			    		if(unet_send(&server, &ret[idx], size,21) == -1) break;
			    		size = 0;
			    	}else
			    	{
			    		if(unet_send(&server, &ret[idx], MAX_APP_PAYLOAD_SIZE,21) == -1) break;
			    		size -= MAX_APP_PAYLOAD_SIZE;
			    		idx += MAX_APP_PAYLOAD_SIZE;
			    	}
			    }while(size > 0);
			}
		}
	}
}
#endif
