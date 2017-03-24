/*
 * timer_control.c
 *
 *  Created on: 5 de mai de 2016
 *      Author: gustavo
 */
#include "trickle.h"
#include "BRTOS.h"
#include "NetConfig.h"
#include "ieee802154.h"
#include "pcg_basic.h"

#define MAX_SCALING_INTERVAL	8
volatile uint16_t trickle_seed = 0;


#if 0
unsigned short random_rand(void){
	// escrever c�digo que use a semente para gerar um n�mero aleat�rio
	return trickle_seed;
}
#endif

// a semente pode ser o endere�o 16 bits do n� + 1 (por causa do coordenador)
void form_seed(uint16_t seed){
	int i = 0;
	while(i < 16 || (seed == 0x0000 || seed == 0x8003)) {
		// o valor que aplica-se o ou na linha abaixo deve ser aleat�rio
		// n�o usar o i
		seed = (seed << 1) | i;
		seed <<= 1;
		i++;
	}
	trickle_seed = seed;
}

void open_trickle(trickle_t *timer, ostick_t interval, ostick_t max_interval){
	addr16_t node_addr16 = {.u8 = {MAC16_INIT_VALUE}};
	timer->interval = interval;
	timer->init_interval = interval;
	timer->max_interval = max_interval;
	timer->interval_scaling = 0;
	timer->complete_interval = 0;

	// Se diferente do estado inicial
	if (pcg32_state() == 0x853c49e6748fea9bULL){
		if (trickle_seed == 0) form_seed((uint16_t)((node_addr16.u8[0]<<1) | node_addr16.u8[1]));	// must be the node address 16
		pcg32_srandom((uint64_t)trickle_seed, (uint64_t)((node_addr16.u8[0]<<1) | node_addr16.u8[1]));
	}
}

void run_trickle(trickle_t *timer){
	ostick_t temp;
	// Peguei esse c�digo do contiki, mas me parece que o intervalo cresce
	// r�pido demais, talvez a melhor abordagem seja intervalo de escala
	// fixo em 1
	if (timer->interval < (timer->max_interval>>1)){
		timer->interval = timer->interval << timer->interval_scaling;
	}else{
		timer->interval = timer->max_interval;
	}

	if (timer->interval_scaling < MAX_SCALING_INTERVAL){
		// Ao inv�s de deixar crescer, limitei em um o fator de escala
		// sen�o o intervalo cresce muito r�pido
		timer->interval_scaling++;
		if (timer->interval_scaling > 1) timer->interval_scaling = 1;
	}

	temp = (timer->interval >> 1) + ((ostick_t)pcg32_boundedrand(timer->max_interval) % (timer->interval >> 1));
	timer->t = temp + timer->complete_interval;

	// Calcula o tempo complementar para o pr�ximo c�lculo do trickle
	timer->complete_interval = timer->interval - temp;
}

void reset_trickle(trickle_t *timer){
	timer->interval = timer->init_interval;
	timer->complete_interval = 0;
}

uint8_t is_trickle_reseted(trickle_t *timer){
	if (timer->interval == timer->init_interval){
		return TRUE;
	}else{
		return FALSE;
	}
}
