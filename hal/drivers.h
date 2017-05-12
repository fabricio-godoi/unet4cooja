/**
* \file drivers.h
* \brief Microcontroller drivers; defines and function prototypes.
*
*
**/
#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "BRTOS.h"

void drivers_init();

/** Serial functions */
uint8_t getchar(uint8_t *c, ostick_t time_wait);
uint8_t gets(uint8_t *string, ostick_t time_wait);
int putchar(int c);
int puts(const char *s);

#endif // DRIVERS_H_
