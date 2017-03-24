/*
 * \file packet.c
 *
 */

#include "packet.h"
#include "BoardConfig.h"

/*--------------------------------------------------------------------------------------------*/
uint8_t packet_info_get(packet_t *pkt, packet_info_t opt)
{
	return pkt->info[opt];
}
/*--------------------------------------------------------------------------------------------*/
uint8_t packet_info_set(packet_t *pkt, packet_info_t opt, uint8_t val)
{
	REQUIRE_FOREVER(opt < PKTINFO_MAX_OPT);
	pkt->info[opt] = val;
	return 0;
}

/*--------------------------------------------------------------------------------------------*/
#define PACKET_PRINT_ENABLE   1
void packet_print(uint8_t *pkt, uint8_t len)
{
#if PACKET_PRINT_ENABLE
	while(len > 0)
	{
		len--;
		PRINTF("%02X ", *pkt++);
	}
	PRINTF("\r\n");

#else
	(void)pkt; (void)len;
#endif
}
/*--------------------------------------------------------------------------------------------*/
uint8_t packet_acquire_down(void)
{
	OS_SR_SAVE_VAR
	extern packet_t packet_down;
	/* todo : use a mutex with timeout */
	OSEnterCritical();
	if(packet_down.state != PACKET_IDLE)
	{
		OSExitCritical();
		return PACKET_ACCESS_DENIED;
	}else
	{

		packet_down.state = PACKET_START_ROUTE;
		OSExitCritical();
		return PACKET_ACCESS_ALLOWED;
	}

}
/*--------------------------------------------------------------------------------------------*/
void packet_release_down(void)
{
	OS_SR_SAVE_VAR
	extern packet_t packet_down;
	/* todo : use a mutex */
	OSEnterCritical();
	packet_down.state = PACKET_IDLE;
	OSExitCritical();
}
/*--------------------------------------------------------------------------------------------*/
uint8_t packet_acquire_up(void)
{
	OS_SR_SAVE_VAR
	extern packet_t packet_up;
	/* todo : use a mutex with timeout */
	OSEnterCritical();
	if(packet_up.state != PACKET_IDLE)
	{
		OSExitCritical();
		return PACKET_ACCESS_DENIED;
	}else
	{
		packet_up.state = PACKET_START_ROUTE;
		OSExitCritical();

		PRINTF_ROUTER(1,"PACKET OWNED BY TASK %s \r\n", ContextTask[currentTask].TaskName);
		return PACKET_ACCESS_ALLOWED;
	}

}
/*--------------------------------------------------------------------------------------------*/
void packet_release_up(void)
{
	OS_SR_SAVE_VAR
	extern packet_t packet_up;
	/* todo : use a mutex */
	OSEnterCritical();
	packet_up.state = PACKET_IDLE;
	OSExitCritical();
}
/*--------------------------------------------------------------------------------------------*/
packet_state_t packet_state_down(void)
{
	extern packet_t packet_down;
	return packet_down.state;
}
/*--------------------------------------------------------------------------------------------*/
packet_state_t packet_state_up(void)
{
	extern packet_t packet_up;
	return packet_up.state;
}
/*--------------------------------------------------------------------------------------------*/
