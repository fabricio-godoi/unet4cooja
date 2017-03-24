/**
* \file drivers.h
* \brief Microcontroller drivers defines and function prototypes.
*
*
**/

#include "BRTOS.h"
#include "mcu.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include <string.h>

void drivers_init();

#ifndef putchar
extern int putchar(int c);
#endif

#ifndef puts
int puts(const char *s);
#endif

