/*
 * @file:	stdio.c
 * @author:	Fabricio Negrisolo de Godoi
 * @date:	19-02-2017
 * @brief:	Function to input/output text, alike stdio.
 * @source:	http://forum.43oh.com/topic/1289-tiny-printf-c-version/#entry10652
 */

#include "stdio.h"

#include "drivers.h"
#include <stdarg.h>

//** Check if the default output function is putchar
#ifndef putc
#define putc(x) putchar(x)
#endif

//** Configuration
#define PAD_RIGHT 1
#define PAD_ZERO 2

//** Error check
#ifndef putchar
#ifndef putc
#error "To use this module, must specify putchar or putc function!"
#endif
#endif

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


#define DIGITS 10
static const char* itoa (unsigned long i){
	// max value of 16bits = 0 to 65535
	// long is 32bits = 4.294.967.295 (10 digits)

	/* Room for INT_DIGITS digits, - and '\0' */
	  static char buf[DIGITS + 2];
	  char *p = buf + DIGITS + 1;	/* points to terminating '\0' */
	  if (i >= 0) {
	    do {
	      *--p = '0' + (i % 10);
	      i /= 10;
	    } while (i != 0);
	    return p;
	  }
	  else {			/* i < 0 */
	    do {
	      *--p = '0' - (i % 10);
	      i /= 10;
	    } while (i != 0);
	    *--p = '-';
	  }
	  return p;
}


static void xtoa(unsigned long x, const unsigned long *dp)
{
    char c;
    unsigned long d;
    if(x) {
        while(x < *dp) ++dp;
        do {
            d = *dp++;
            c = '0';
            while(x >= d) ++c, x -= d;
            putc(c);
        } while(!(d & 1));
    } else
        putc('0');
}

static void puth(unsigned n, char format)
{
    static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
    static const char HEX[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
    if(format == 'X') putc(HEX[n & 15]);
    else putc(hex[n & 15]);
}

void printf(char *format, ...)
{
    char c;
    int i;
    long n;
    register int pad, width;

    va_list a;
    va_start(a, format);
    while(c = *format++) {
        if(c == '%') {
        	pad &= ~PAD_ZERO;
        	width = 0;
            // Clear pad
        	while (*format == '0') { // This indicates how many zeros must appear at least
				++format;
				pad |= PAD_ZERO;
			}
        	// Clear width
			for ( ; *format >= '0' && *format <= '9'; ++format) { // This check for the width of zeros nedded
				width *= 10;
				width += *format - '0';
			}
            switch(c = *format++) {
				case 'x':
				case 'X':
					i = va_arg(a, int);
					if(width > 0){
						while(width-- > 0) puth(i >> (width*4), c);
					}
					else{
						do{ // base 16
							puth(i%16, c);
							i /= 16;
						}while(i>0);
					}
					break;
                case 's':                       // String
                    puts(va_arg(a, char*));
                    break;
                case 'c':                       // Char
                    putc(va_arg(a, char));
                    break;
                case 'd':
                    i = va_arg(a, int);
                    if(i < 0) i = -i, putc('-');
                    xtoa((unsigned)i, dv + 5);
                    break;
                case 'i':                       // 16 bit Integer
                case 'u':                       // 16 bit Unsigned
                    i = va_arg(a, int);
                    if(c == 'i' && i < 0) i = -i, putc('-');
                    xtoa((unsigned)i, dv + 5);
                    break;
                case 'l':                       // 32 bit Long
                case 'n':                       // 32 bit uNsigned loNg
                    n = va_arg(a, long);
                    if(c == 'l' &&  n < 0) n = -n, putc('-');
                    xtoa((unsigned long)n, dv);
                    break;
//                case 'x':                       // 16 bit heXadecimal
//                    i = va_arg(a, int);
//                    puth(i >> 12);
//                    puth(i >> 8);
//                    puth(i >> 4);
//                    puth(i);
//                    break;
                case 0: return;
                default: goto bad_fmt;
            }
        } else
bad_fmt:    putc(c);
    }
    va_end(a);
}



///*** BRTOS \/ *****/


static void printchar(char **str, int c)
{

	if (str) {
		**str = (char)c;
		++(*str);
	}
	else
	{
		(void)putchar(c);
	}
}




static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = (unsigned int)i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = (unsigned int)-i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (unsigned int)u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = (char)(t + '0');
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}





/**** BRTOS \/ *******/


static int print( char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}


int sprintf_lib(char *out, const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return print( &out, format, args );
}


int snprintf_lib( char *buf, unsigned int count, const char *format, ... )
{
        va_list args;

        ( void ) count;

        va_start( args, format );
        return print( &buf, format, args );
}

int printf_lib(const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return print( 0, format, args );
}

