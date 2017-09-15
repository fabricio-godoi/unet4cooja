/**********************************************************************************
@file   unet_api.h
@brief  public interface to the UNET network
@authors: Gustavo Weber Denardin
          Carlos Henrique Barriquello

Copyright (c) <2009-2013> <Universidade Federal de Santa Maria>

  * Software License Agreement
  *
  * The Software is owned by the authors, and is protected under 
  * applicable copyright laws. All rights are reserved.
  *
  * The above copyright notice shall be included in
  * all copies or substantial portions of the Software.
  *  
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE. 
*********************************************************************************/

#ifndef UNET_API_H
#define UNET_API_H

/* This file is a public interface to the UNET network */
/* Any application should reference only this file in order to use the UNET API */

#include "NetConfig.h"
#include "radio.h"
#include "packet.h"
#include "node.h"
#include "ieee802154.h"
#include "link.h"
#include "unet_app.h"

#include "string.h"

#define UNET_VERSION    "Network Ver. 1.3.0"

/* Message code returns */ 
#define SEND_OK              (INT8U)0x00
#define SEND_ERROR           (INT8U)0x01

#define PACKET_OVERHEAD_SIZE  (UNET_PHY_HEADER_SIZE+UNET_MAC_HEADER_SIZE+UNET_LLC_HEADER_SIZE+UNET_NWK_HEADER_SIZE+UNET_TRANSP_HEADER_SIZE+UNET_MAC_FOOTER_SIZE+3)
#define MAX_APP_PAYLOAD_SIZE  (MAX_PACKET_SIZE - PACKET_OVERHEAD_SIZE)

/* External functions */
void UNET_Init(void);
void UNET_APP(void);
void UNET_Set_App_Callback(BRTOS_Sem *callback);

/* UNET tasks */
#if TASK_WITH_PARAMETERS == 1
void UNET_Radio_Task(void *param);
void UNET_Link_Packet_Task(void *param);
void UNET_NWK(void *param);
#else
void UNET_Radio_Task(void);
void UNET_Link_Packet_Task(void);
void UNET_NWK(void);
#endif


/* Functions used to send data in the network */
INT8U NetSimpledata(INT8U direction, INT8U destino, INT8U *p_data);
void  NetSimpleMeasure(INT8U direction, INT8U MeasureType, INT16U Value16, INT32U Value32);

/* Functions used to receive and decode data from the network */
void  Decode_Lighting_Profile(void);
void  Decode_SmartEnergy_Profile(void);
uint8_t Decode_Data_Profile(void);

typedef uint16_t netst_cnt_t;
struct netstat_t
{
	/// Application
	netst_cnt_t apptxed;      // application txed packets
	netst_cnt_t apprxed;      // application rxed packets

	/// Transport
	netst_cnt_t netapptx;	  // packets storage at network layer, ready to be sent
	netst_cnt_t netapprx;	  // packets received at network layer, ready to be used to application

	/// Network
	netst_cnt_t duplink;      // packets duplicates at link layer

	netst_cnt_t routed;       // routed packets
	netst_cnt_t dupnet;       // packets duplicates at net layer
	netst_cnt_t routdrop;     // packets dropped by routing buffer overflow

	/// Data Link
	netst_cnt_t txed;         // successfully transmitted packets
	netst_cnt_t rxed;         // received packets
	netst_cnt_t txfailed;     // transmission failures
	netst_cnt_t txnacked;	  // transmission not acked
	netst_cnt_t txmaxretries; // transmission max retries reached
	netst_cnt_t chkerr;       // checksum error

	netst_cnt_t overbuf;      // packets dropped by RX buffer overflow
	netst_cnt_t dropped;      // packets dropped by hops limit, route not available
	netst_cnt_t corrupted;    // application packet corrupted, does not have the size informed

	/// Physical
	netst_cnt_t radiotx;	  // packets sent in the radio
	netst_cnt_t radiorx;	  // packets recv in the radio
	netst_cnt_t radiocol;     // radio transmissions collisions
	netst_cnt_t radionack;    // radio transmission not acknowledged


	/// Removed because contiki doesn't use it
//	netst_cnt_t radioresets;  // radio reset

//	netst_cnt_t rxedbytes;    // rxed bytes
//	netst_cnt_t txedbytes;    // txed bytes
//	netst_cnt_t rxbps;        // rx throughput
//	netst_cnt_t txbps;        // tx throughput
};

#define NODESTAT_ENABLE()   do{ extern char UNET_NodeStat_Ctrl; UNET_NodeStat_Ctrl=1;} while(0);
#define NODESTAT_DISABLE()	do{ extern char UNET_NodeStat_Ctrl; UNET_NodeStat_Ctrl=0;} while(0);
#define NODESTAT_RESET() 	do{ extern struct netstat_t UNET_NodeStat; memset(&UNET_NodeStat, 0x00, sizeof(UNET_NodeStat));} while (0);
#define NODESTAT_CHECK(x)   if((x) == (netst_cnt_t)(-1)) NODESTAT_RESET();
// Default is enabled
#define NODESTAT_UPDATE(x)  do {                                                \
                                extern struct netstat_t UNET_NodeStat;          \
                                extern char UNET_NodeStat_Ctrl;                 \
								if(UNET_NodeStat_Ctrl){                         \
                                    NODESTAT_CHECK(++UNET_NodeStat.x)           \
								}                                               \
							}while(0);

#define NODESTAT_ADD(x,y)	do {                                                \
								extern struct netstat_t UNET_NodeStat;          \
								extern char UNET_NodeStat_Ctrl;                 \
								if(UNET_NodeStat_Ctrl){                         \
									UNET_NodeStat.x += y;                       \
								}                                               \
							}while(0);

void* GetUNET_Statistics(uint8_t* tamanho);


// Apps signals
#ifdef SIGNAL_APP1
extern BRTOS_Sem    *(SIGNAL_APP1);
#endif


#endif
