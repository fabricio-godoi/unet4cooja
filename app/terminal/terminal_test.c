/*
 * terminal_test.c
 */

#include "terminal.h"

#define DEBUG_PRINTF 1
#if DEBUG_PRINTF && WINNT
//#include <stdio.h>
//#define PRINTF(...) printf(__VA_ARGS__); fflush(stdout);
#elif DEBUG_PRINTF
#include "../../drivers/stdio.h"
#define PRINTF(...) printf(__VA_ARGS__);
#else
#define PRINTF(...)
#endif

void terminal_test_cmd(char *cmd)
{
	while(*cmd)
	{
		if(terminal_input(*cmd++) == 1)
		{
			terminal_process();
		}
	}

}

void terminal_test(void)
{
	terminal_test_cmd("cmd arg1 arg2 arg3\n");
	terminal_test_cmd("help\n");
	terminal_test_cmd("top\n");
	terminal_test_cmd("ver\n");
	terminal_test_cmd("ver\r");
	terminal_test_cmd("ver\r\n");

	terminal_test_cmd("netaddr\n");
	terminal_test_cmd("netst\n");
	terminal_test_cmd("netnt\n");
	terminal_test_cmd("netrt\n");
	terminal_test_cmd("netsd 1\n");

#if 0
	terminal_test_cmd("nettx abcdef\n");
#endif

	terminal_test_cmd("netrx\n");
	terminal_test_cmd("netpl 10\n");
	terminal_test_cmd("netch 15\n");
	terminal_test_cmd("netch 26\n");



	PRINTF("all commands executed!\r\n");
}





