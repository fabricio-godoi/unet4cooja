#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

//#include "radio_null.h"
#include "cc2520.h"

/******************************************************************/
/* ADC channels for voltage, current, temperature, light          */
/* Relay output pin                                               */
#if PROCESSOR == COLDFIRE_V1
#define   PIN_RELAY           PTED_PTED2
#define   CHAN_LIGHT          (INT8U)18
#define   CHAN_VIN            (INT8U)1
#define   CHAN_VLOAD          (INT8U)0
#define   CHAN_ILOAD          (INT8U)19
#define   CORE_TEMP           (INT8U)26
#endif
/*********************************************************/

/*********************************************************/
/* EEPROM Config */
#if PROCESSOR == COLDFIRE_V1
#define EEPROM_ADDRESS        0x0001FC00
#endif
/*********************************************************/

/*********************************************************/
/* DBG Config */
#if PROCESSOR == COLDFIRE_V1
#define BDM_ENABLE            1
#define TEST_PIN              0
#define BDM_DEBUG_OUT         PTAD_PTAD4
#endif
/*********************************************************/

/*************************************************/
/** Radio configuration - used by radio driver   */ 
// #define MAC64_MEM_ADDRESS    0x00001FF0

// Power levels
#define RFTX_0dB    0x00
#define RFTX_m10dB  0x40
#define RFTX_m20dB  0x80
#define RFTX_m30dB  0xC0
#define RFTX_m36dB  0xF8

#define INCLUDE_PRINT		0
#define PRINT_PING_INFO()

/****************************************************/
#endif
