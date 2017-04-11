/**
* \file drivers.h
* \brief Microcontroller drivers defines and function prototypes.
*
*
**/
#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "BRTOS.h"
#include "mcu.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include <string.h>

void drivers_init();

int putchar(int c);
int puts(const char *s);

#endif // DRIVERS_H_
