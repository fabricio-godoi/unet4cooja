/*
 * \file node.c
 *
 */

#include <string.h>
#include "node.h"

static addr64_t  	 Addr64;
static addr64_t  	 PANId64;
static node_params_t nodedata;

/*--------------------------------------------------------------------------------------------*/
void node_data_init(void)
{
	memset(&nodedata, 0x00, sizeof(node_params_t));
}
/*--------------------------------------------------------------------------------------------*/
uint16_t node_data_get_16b(node_param_opt_t opt)
{
	uint16_t val;
	val = node_data_get(opt) << 8;
	val += node_data_get(opt+1);
	return val;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_data_set_16b(node_param_opt_t opt, uint16_t val)
{
	REQUIRE_FOREVER((opt+1) < NODE_PARAM_MAX);
	val = HTONS(val);
	nodedata.node_param[opt] = (uint8_t)(val >> 8);
	nodedata.node_param[opt+1] = (uint8_t) val;
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_data_get(node_param_opt_t opt)
{
	return nodedata.node_param[opt];
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_data_set(node_param_opt_t opt, uint8_t val)
{
	REQUIRE_FOREVER(opt < NODE_PARAM_MAX);
	nodedata.node_param[opt] = val;
	// Ao atualizar o macaddr16 e panid16 do nó, altera também no radio
	if (opt >=NODE_ADDR16H && opt <=NODE_PANID16L){
		 UNET_RADIO.set(opt,val);
	}
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t* node_pan_id64_get(void)
{
	return (uint8_t*)&PANId64;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_pan_id64_set(uint8_t *val)
{
	REQUIRE_FOREVER(val != NULL);
	memcpy(&PANId64,val,8);
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
void node_pan_id64_init(void)
{
	memset(&PANId64, 0xFF, sizeof(PANId64));
}
/*--------------------------------------------------------------------------------------------*/
uint8_t* node_addr64_get(void)
{
	return (uint8_t*)&Addr64.u8;
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_addr64_set(uint8_t *val)
{
	REQUIRE_FOREVER(val != NULL);
	memcpy(&Addr64,val,8);
	return 0;
}
/*--------------------------------------------------------------------------------------------*/
void node_addr64_init(void)
{
	memset(&Addr64, 0xFF, sizeof(Addr64));
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_seq_num_get_next(void)
{

	if(++nodedata.node_param[NODE_SEQ_NUM] == 0)
	{
		nodedata.node_param[NODE_SEQ_NUM] = 1;
	}
	return nodedata.node_param[NODE_SEQ_NUM];
}
/*--------------------------------------------------------------------------------------------*/
uint8_t node_seq_num_get(void)
{
	return nodedata.node_param[NODE_SEQ_NUM];
}
/*--------------------------------------------------------------------------------------------*/
void node_addr64_print(uint8_t * addr)
{
	REQUIRE_FOREVER(addr != NULL);
	uint8_t i = 0;
	for (i=0;i<7;i++)
	{
		#if DEBUG_PRINTF == 1
		PRINTF("%02X:", *addr++);
		#else
		#include "terminal_cfg.h"
		TERM_PRINT("%02X:", *addr++);
		#endif
	}
	#if DEBUG_PRINTF == 1
	PRINTF("%02X:", *addr++);
	#else
	TERM_PRINT("%02X", *addr++);
	#endif
}
/*--------------------------------------------------------------------------------------------*/

