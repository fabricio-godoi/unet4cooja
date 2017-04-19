/**
 * \file cooja.c
 * \author Fabricio Negrisolo de Godoi
 * \brief File intended to initiate all Cooja simulator requisites
 */

#ifndef COOJA_H_
#define COOJA_H_

#include "node_id.h"


/**
 * This enables Cooja annotation, e.g. arrows to parents
 * NOTE: the coordinator ID must be set to zero to this fully work
 */
#define COOJA_ANNOTATION



void cooja_initialize(void);

#endif /* COOJA_COOJA_H_ */
