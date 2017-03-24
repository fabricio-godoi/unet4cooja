/*
 * @file:	stdio.h
 * @author:	Fabricio Negrisolo de Godoi
 * @date:	19-02-2017
 * @brief:	Function to input/output text
 *
 */

#ifndef _STDIO_H_
#define _STDIO_H_

/* Configurable to use the default input/output */
#ifdef printf
#undef printf
#endif
//const char* itoa (unsigned long i);




int sprintf_lib(char *out, const char *format, ...);
int snprintf_lib( char *buf, unsigned int count, const char *format, ... );
int printf_lib(const char *format, ...);

void printf(char *format, ...);
#define PRINTF(...) printf(__VA_ARGS__)

////////////////////////////////////////////////
////////////////////////////////////////////////
///      Serial module definitions           ///
////////////////////////////////////////////////
////////////////////////////////////////////////
#define CR             13         //  ASCII code for carry return
#define LF             10         //  ASCII code for line feed



#endif /* _IOCLASS_H_ */
