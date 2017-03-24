/**
 * \file cooja.c
 * \author Fabricio Negrisolo de Godoi
 * \brief File intended to initiate all Cooja simulator requisites
 */

#include "cooja.h"

/* Cooja emulator automatically update it at upload */
unsigned int node_id = 0;

void cooja_initialize(void){
	node_id = 0;
}
