/*
 * assert.h
 *
 *  Created on: Mar 7, 2017
 *      Author: user
 */

#ifndef ASSERT_H_
#define ASSERT_H_


// Recommended to enable/disable DEBUG at other file, or compile directive (same as VERBOSE)
#ifdef DEBUG
#define ASSERT_DEBUG DEBUG
#else
#define ASSERT_DEBUG 1 /// Default is enable, but if is deploy version, you should disable it
#endif

#if ASSERT_DEBUG == 1
#include "stdio.h"
#endif

/* For debugging code */
/* Simple assert, if condition 'x' isn't met, error is throw */
#define assert(x)   		if(!(x)){ printf("Error at %s:%d\n",__FILE__,__LINE__); while(1); }
/* If condition 'x' isn't met, error is throw with status 's' */
#define asserte(x,s) 	if(!(x)){ printf("Error %d at %s:%d\n",s,__FILE__,__LINE__); while(1); }
/* Try to assert x with y, if not equal, return message with x value */
// TODO improve this, cannot pass x to printf, since will execute some function again, change call method?
//#define asserte(x,y)		if(!(x)){ PRINTF("Got %d at %s:%d, expected %d\n",x,__FILE__,__LINE__,y); while(1); }

#endif /* ASSERT_H_ */
