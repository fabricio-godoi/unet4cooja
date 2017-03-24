/*
 * \file node.h
 *
 */

#ifndef UNET_NODE_H_
#define UNET_NODE_H_

#include "ieee802154.h"
#include "radio.h"


typedef enum
{
	NODE_CHANNEL = 0,
	NODE_DISTANCE = 1,
	NODE_ADDR16 = 2,
	NODE_ADDR16H = 2,
	NODE_ADDR16L = 3,
	NODE_PANID16 = 4,
	NODE_PANID16H = 4,
	NODE_PANID16L = 5,
	NODE_PARENTINDEX = 6,
	NODE_PARENTFAILURES = 7,
	NODE_NEIGHBOR_COUNT = 8,
	NODE_SEQ_NUM = 9,
	NODE_PARAM_MAX
}node_param_opt_t;

typedef union
{
	struct node_params
	{
		uint8_t channel;
		uint8_t distance;
		uint8_t addr16h;
		uint8_t addr16l;
		uint8_t panid16h;
		uint8_t panid16l;
		uint8_t parent_idx;
		uint8_t parent_successive_failures;
		uint8_t nb_cnt;
		uint8_t seq_num;
	}st_node_param;
	uint8_t node_param[NODE_PARAM_MAX];
}node_params_t;

/** node functions */
uint8_t  node_data_get(node_param_opt_t opt);
uint8_t  node_data_set(node_param_opt_t opt, uint8_t val);
void     node_data_init(void);
uint8_t* node_pan_id64_get(void);
uint8_t  node_pan_id64_set(uint8_t *val);
void     node_pan_id64_init(void);
uint8_t* node_addr64_get(void);
uint8_t  node_addr64_set(uint8_t *val);
void     node_addr64_init(void);
uint16_t node_data_get16b(node_param_opt_t opt);
uint8_t  node_data_set16b(node_param_opt_t opt, uint16_t val);
uint8_t  node_seq_num_get(void);
uint8_t  node_seq_num_set(void);
uint8_t  node_seq_num_get_next(void);
void     node_addr64_print(uint8_t * addr);


#endif /* UNET_NODE_H_ */
