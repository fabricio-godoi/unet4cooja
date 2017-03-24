/** System Includes **/
#include "tasks.h"
#include "BRTOS.h"
#include "debug_stack.h"
#include "leds.h"

/** Configuration Files **/
#include "AppConfig.h"
#include "NetConfig.h"
#include "BoardConfig.h"

/** Function Prototypes **/
#include "unet_api.h"
#include "transport.h"
#include "app.h"
#include "unet_app.h"


/** Public/Private Variables **/

extern BRTOS_Queue *Serial;
#if TASK_WITH_PARAMETERS == 1
void System_Time(void *parameters)
#else
void System_Time(void)
#endif
{   
	PRINTF("Started System_Time task!\n");
	OSResetTime();
	OSResetDate();

	INT16U i = 0;
	// task main loop
	for (;;)
	{
		DelayTask(10); // 10ms
		i++;
		if(i>=100){
			OSUpdateUptime();
			i=0;
		}
	}
}


#include "OSInfo.h"
char big_buffer[20];

#if TASK_WITH_PARAMETERS == 1
void Task_Serial(void *parameters)
#else
void Task_Serial(void)
#endif
{
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




#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
#if (TASK_WITH_PARAMETERS == 1)
void pisca_led_net(void *param){
	(void)param;
#else
void pisca_led_net(void){
#endif
	unet_transport_t client;
//	addr64_t dest_addr64 = {.u8 = {0x47,0x42,0x00,0x00,0x00,0x00,0x12,0x34}};
	addr64_t dest_addr64 = {.u8 = {0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x01}}; // set client as destination
//	addr64_t dest_addr64 = {.u8 = {0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x02}}; // set client as destination
	uint8_t packet[8];
	uint8_t message[32];
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

	unsigned int mac, pan;
	unsigned long mac64;
	unsigned char value;
	for(;;)
	{

//		OSEnterCritical();
//		// PAN
//		UNET_RADIO.get(PANID16H,&value);
//		pan = value<<8;
//		UNET_RADIO.get(PANID16L,&value);
//		pan |= value;
//		PRINTF("tasks: pan %04X\n",pan);
//		// ADDR64
//		PRINTF("tasks: addr64 ");
//		UNET_RADIO.get(MACADDR64_7,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_6,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_5,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_4,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_3,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_2,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_1,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_0,&value);
//		PRINTF("%02X\n",value);
//		// MAC
//		UNET_RADIO.get(MACADDR16H,&value);
//		mac = value<<8;
//		UNET_RADIO.get(MACADDR16L,&value);
//		mac |= value;
//		PRINTF("tasks: mac %04X\n",mac);
//
//		OSExitCritical();

		/// Turn LED on

		// Envia mensagem para o client
//		app_payload->APP_Command_Attribute = TRUE;
//		unet_send(&client, packet, 3,30);
//		PRINTF("tasks: PAN sending packet\n");
		if (unet_recv(&client,message,0) >= 0)
		{
			PRINTF("tasks: PAN rcv packet\n");
			PRINTF_APP(1,"tasks: Packet received from (port/address):0xff  %d/",client.sender_port); PRINTF_APP_ADDR64(1,&(client.sender_address));
			PRINTF_APP(1,"tasks: Packet Content: %s\n\r",message);
			DelayTask(5000);
		}else{
			DelayTask(10000);
		}

		/// Turn LED off


//		app_payload->APP_Command_Attribute = FALSE;
//		unet_send(&client, packet, 3,30);
//		PRINTF("tasks: PAN sending packet\n");
//		if (unet_recv(&client,message,1000) >= 0)
//		{
//			PRINTF("tasks: PAN rcv packet\n");
//			PRINTF_APP(1,"Packet received from (port/address):  %d/",client.sender_port); PRINTF_APP_ADDR64(1,&(client.sender_address));
//			PRINTF_APP(1,"Packet Content: %s\n\r",message);
//			DelayTask(5000);
//		}else{
//			DelayTask(10000);
//		}
	}
}
#endif


#if (TASK_WITH_PARAMETERS == 1)
void UNET_App_1_Decode(void *param){
   (void)param;
#else
void UNET_App_1_Decode(void){
#endif
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
#if (TASK_WITH_PARAMETERS == 1)
void pisca_led_net(void *param){
	(void)param;
#else
void pisca_led_net(void){
#endif
	 uint8_t packet[8];
////	addr64_t dest_addr64 = {.u8 = {0x47,0x42,0x00,0x00,0x00,0x00,0x12,0x34}};
		addr64_t dest_addr64 = {.u8 = {0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00}};
//		addr64_t dest_addr64 = {.u8 = {0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x01}};

	unet_transport_t server;

	server.src_port = 221;
	server.dst_port = 222;
	server.dest_address = &dest_addr64;
//	unet_listen(&server);
//	char x;
//	if(node_id == 2) x = 3;
//	else if(node_id == 3)	x=2;
//	else x=1;
//	addr64_t dest_addr64 = {.u8 = {0xff,0xff,0x00,0x00,0x00,0x00,0x00,x}};

	unsigned char string[] = "teste\0";
	unsigned int mac, pan;
	unsigned long mac64;
	unsigned char value;
	for(;;)
	{

//		OSEnterCritical();
//		// PAN
//		UNET_RADIO.get(PANID16H,&value);
//		pan = value<<8;
//		UNET_RADIO.get(PANID16L,&value);
//		pan |= value;
//		PRINTF("tasks: pan %04X\n",pan);
//		// ADDR64
//		PRINTF("tasks: addr64 ");
//		UNET_RADIO.get(MACADDR64_7,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_6,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_5,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_4,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_3,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_2,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_1,&value);
//		PRINTF("%02X ",value);
//		UNET_RADIO.get(MACADDR64_0,&value);
//		PRINTF("%02X\n",value);
//		// MAC
//		UNET_RADIO.get(MACADDR16H,&value);
//		mac = value<<8;
//		UNET_RADIO.get(MACADDR16L,&value);
//		mac |= value;
//		PRINTF("tasks: mac %04X\n",mac);
//
//		OSExitCritical();


		// Envia mensagem para o coordenador
		unet_send(&server,string,6,0);

//	   (void)unet_recv(&server,packet,0);
//	   PRINTF("tasks: recv\n");
//	   PRINTF_APP(1,"Packet received from (port/address):  %d/",server.sender_port);
//	   PRINTF_APP_ADDR64(1,&(server.sender_address));


//		PRINTF("tasks: sending ON\n");
//		NetGeneralONOFF(TRUE, &dest_addr64);
//		DelayTask(1000);
////		PRINTF("tasks: sending OFF\n");
//		NetGeneralONOFF(FALSE, &dest_addr64);
		DelayTask(1000);
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
#if (TASK_WITH_PARAMETERS == 1)
void UNET_App_1_Decode(void *param){
	   (void)param;
#else
void UNET_App_1_Decode(void){
#endif
#if (defined BOOTLOADER_ENABLE) && (BOOTLOADER_ENABLE==1)
   INT8U  ret = 0;
#endif

#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
   char buffer[8]; (void) buffer;
#endif

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
#if (TASK_WITH_PARAMETERS == 1)
void Terminal_Task(void *p){
	(void)p;
#else
void Terminal_Task(void){
#endif

	uint8_t data = '\0';
	terminal_init();

	while(1)
	{
//		#if (BRTOS_PLATFORM == FRDM_KL25Z)
//		UARTGetChar(UART0_BASE,&data,0);
//		UARTPutChar(UART0_BASE,data);
//		#endif
		data=getchar();
		putchar(data);
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

#if (TASK_WITH_PARAMETERS == 1)
void Terminal_Task(void *p){
	(void)p;
#else
void Terminal_Task(void){
#endif
	unet_transport_t server;
	uint8_t		payload_rx[16];
	//uint8_t   	payload_tx[512];
	uint8_t		*cmd;
	uint8_t		*ret;
	int 		idx = 0;
	int 		size = 0;

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

#ifdef BRTOS_PLATFORM
#if (BRTOS_PLATFORM == BOARD_ROTEADORCFV1)
				printf_install_putchar(unet_putchar);
#endif
#endif
				ret = (uint8_t*)terminal_process();

#ifdef BRTOS_PLATFORM
#if BRTOS_PLATFORM == BOARD_ROTEADORCFV1
				if(term_buffer_size != 0) unet_putchar(ETX); /* Send of text char */
				printf_install_putchar(NULL);
				break;
#endif
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
