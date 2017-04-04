/*
 * \file:	stdio.h
 * \author:	Fabricio Negrisolo de Godoi
 * \date:	02-04-2017
 * \brief:	Functions to output formated strings
 */

#ifndef STDIO_H_
#define STDIO_H_

int sprintf(char *out, const char *format, ...);
int printf(const char *format, ...);
//const char* itoa (unsigned long i);

#define PRINTF(...) //printf(__VA_ARGS__)

////////////////////////////////////////////////
////////////////////////////////////////////////
///      Serial module definitions           ///
////////////////////////////////////////////////
////////////////////////////////////////////////
#define CR             13         //  ASCII code for carry return
#define LF             10         //  ASCII code for line feed



#endif /* STDIO_H_ */
