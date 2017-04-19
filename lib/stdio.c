/*
 * \file:	stdio.c
 * \author:	Fabricio Negrisolo de Godoi
 * \date:	02-04-2017
 * \brief:	Functions to output formated strings
 */

#include "stdio.h"

#include <stdarg.h>

//** Configuration
#define PAD_RIGHT 1
#define PAD_ZERO 2

/** \brief dv is the number of digits to convert to string */
static const unsigned long dv[] = {
//  4294967296      // 32 bit unsigned max
    1000000000,     // +0
     100000000,     // +1
      10000000,     // +2
       1000000,     // +3
        100000,     // +4
//       65535      // 16 bit unsigned max
         10000,     // +5
          1000,     // +6
           100,     // +7
            10,     // +8
             1,     // +9
};

/**
 * \brief put one character into defined output or standard output (e.g. UART)
 * \param out it's a pointer to a pointer (e.g. string, buffer, memory region)
 * \param c it's the character that will be put in the output
 * \return 1 for defined output, otherwise return the standard output function
 */
static short putc(char **out, int c)
{
	if(out) {
		**out = (char)c;
		++(*out);
	}
	else {
		return putchar(c);
	}
	return 1;
}

/**
 * \brief put one string into defined output or standard output (e.g. UART)
 * \param out it's a pointer to a pointer (e.g. string, buffer, memory region)
 * \param c it's the string that will be put in the output
 * \return the number of characters put in the output
 */
static short putstring(char **out, const char *s){
	short width = 0;
	if(out){
		while(*s) {
			**out = (char)*s++;
			++(*out);
			width++;
		}
	}
	else{
		while(*s) {
			(void)putchar((char)*s++);
			width++;
		}
	}
	return width;
}

/**
 * \brief transform numbers to strings
 * \param out it's a pointer to a pointer (e.g. string, buffer, memory region)
 * \param x is the value that will be converted
 * \param dp is the size in character that x will be transformed
 * \return the number of characters put in the output
 */
static short xtoa(char **out, unsigned long x, const unsigned long *dp)
{
	short size=0;
    char c;
    unsigned long d;
    if(x) {
        while(x < *dp) ++dp;
        do {
            d = *dp++;
            c = '0';
            while(x >= d) ++c, x -= d;
            size += putc(out, c);
        } while(!(d & 1));
    } else{
        size += putc(out, '0');
    }
    return size;
}

/**
 * \brief Transform n into hexadecimal equivalent given a format
 * \param out it's a pointer to a pointer (e.g. string, buffer, memory region)
 * \param n is a value size of byte
 * \param format is 'x' for lower case and 'X' for upper case
 * \return the number of characters put in the output
 */
static short puth(char **out, unsigned n, char format)
{
    static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
    static const char HEX[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
    if(format == 'X') putc(out, HEX[n & 15]);
    else putc(out, hex[n & 15]);
    return 1;
}

/**
 * \brief Transform a string in a given format with a list of arguments and put it in a predefined output.
 * 		  Formats current supported x,X,d,i,u,c,s,l,n.
 * \param out it's a pointer to a pointer (e.g. string, buffer, memory region)
 * \param format it's the format that the string will be formated (e.g. ("Hello %s","world"))
 * \param a it's the arguments list got from va_start
 * \return the number of character put in the output
 */
static int print(char **out, const char *format, va_list a)
{
	register short size=0;
    char c;
    int i;
    long n;
    register int pad, width;

    while((c = *format++)) {
        if(c == '%') {
        	pad &= ~PAD_ZERO;
        	width = 0;
            // Clear pad
        	while (*format == '0') { // This indicates how many zeros must appear at least
				++format;
				pad |= PAD_ZERO;
			}
        	// Clear width
			for ( ; *format >= '0' && *format <= '9'; ++format) { // This check for the width of zeros needed
				width *= 10;
				width += *format - '0';
			}
            switch(c = *format++) {
				case 'x':
				case 'X':
					i = va_arg(a, int);
					if(width > 0){
						while(width-- > 0) size += puth(out, i >> (width*4), c);
					}
					else{
						do{ // base 16
							size += puth(out, i%16, c);
							i /= 16;
						}while(i>0);
					}
					break;
                case 's':                       // String
                    size += putstring(out, (char*)va_arg(a, char*));
                    break;
                case 'c':                       // Char
                    size += putc(out, (char)va_arg(a, int));
                    break;
                case 'd':
                    i = va_arg(a, int);
                    if(i < 0) i = -i, size += putc(out, '-');
                    size += xtoa(out, (unsigned)i, dv + 5);
                    break;
                case 'i':                       // 16 bit Integer
                case 'u':                       // 16 bit Unsigned
                    i = va_arg(a, int);
                    if(c == 'i' && i < 0) i = -i, size += putc(out, '-');
                    size += xtoa(out, (unsigned)i, dv + 5);
                    break;
                case 'l':                       // 32 bit Long
                case 'n':                       // 32 bit uNsigned loNg
                    n = va_arg(a, long);
                    if(c == 'l' &&  n < 0) n = -n, size += putc(out, '-');
                    size += xtoa(out, (unsigned long)n, dv);
                    break;
                case 0: goto end;
                default: goto bad_fmt;
            }
        } else{
bad_fmt:    size+=putc(out, c);
        }
    }
end:
	size+=putc(out,'\0');
    va_end(a);
    return size;
}

/**
 * \brief Transform a string in a given format with a list of arguments and put it in the standard output.
 * 		  Formats current supported x,X,d,i,u,c,s,l,n.
 * \param format it's the format that the string will be formated (e.g. ("Hello %s","world"))
 * \param ... it's the arguments that will be parsed be va_start
 * \return the number of character put in the output
 */
int printf(const char *format, ...)
{
	va_list a;

	va_start(a, format);
	return print((void*)0, format, a);
}


/**
 * \brief Transform a string in a given format with a list of arguments and put it in a predefined output.
 * 		  Formats current supported x,X,d,i,u,c,s,l,n.
 * \param out it's a pointer to a pointer (e.g. string, buffer, memory region)
 * \param format it's the format that the string will be formated (e.g. ("Hello %s","world"))
 * \param ... it's the arguments that will be parsed be va_start
 * \return the number of character put in the output
 */
int sprintf(char *out, const char *format, ...)
{
	va_list a;

	va_start(a, format);
	return print(&out, format, a);
}
