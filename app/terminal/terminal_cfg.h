/*
 * terminal_cfg.h
 *
 */

#ifndef APP_TERMINAL_CFG_H_
#define APP_TERMINAL_CFG_H_

/************* TERMINAL CONFIG *************************/
#define TERM_INPUT_BUFSIZE 		32

/* Supported commands */
/* Must be listed in alphabetical order !!!! */

/*  ------ NAME ------- HELP --- */
#define COMMAND_TABLE(ENTRY) \
ENTRY(help,"Help Command")     \
ENTRY(netaddr, "Show net role, addr16, addr64") \
ENTRY(netch,"Set/get RX channel - 11 to 26")\
ENTRY(netdbv,"Set/get debug verbose")\
ENTRY(netnt,"Print neighbor table") \
ENTRY(netpl,"Set/get TX power - 0 (higher) to 31 (lower)")\
ENTRY(netrt,"Print route table") \
ENTRY(netrx,"Start/stop rx listening")\
ENTRY(netsd,"Set a destination to tx")\
ENTRY(netst, "Show net stats") \
ENTRY(nettst,"Network test") \
ENTRY(nettx,"Tx a packet") \
ENTRY(runst,"Running stats")	\
ENTRY(top,"System info")    \
ENTRY(ver,"System version")

#define HELP_DESCRIPTION         0

#if WINNT
#include <stdio.h>
#define TERM_PRINT(...) printf(__VA_ARGS__); fflush(stdout);
#else
#include "../../lib/stdio.h"
#define TERM_PRINT(...)	printf(__VA_ARGS__)
#endif


/*******************************************************/

#endif /* APP_TERMINAL_CFG_H_ */
