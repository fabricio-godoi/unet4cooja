/***********************************************************************************
@file   app.h
@brief  functions to send and receive data in/from the network
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
**********************************************************************************/

#ifndef APP_H
#define APP_H

#include "BRTOS.h"
#include "unet_api.h"
#include "AppConfig.h"

#if 0
INT8U NetSimpleMeasureSE(INT8U MeasureType, INT16U Value16, INT32U Value32);
INT8U NetMultiMeasureSE(SE_STRUCT *se);
#endif

typedef union
{
	uint8_t   u8[4];
	uint16_t  u16[2];
	uint32_t  u32;
}_uint32_t;

/* app packet struct */
typedef struct _APP_PACKET
{
    // APP Packet
    uint8_t       APP_Profile;       /*  Perfil de Aplicação
                                     ex: perfil p/ iluminação
                                     = LIGHTING_PROFILE */
    uint8_t       APP_Command;       /* Comando do perfil a ser executado */
    uint8_t       APP_Command_Attribute;
                                     /* Atributo do comando do perfil a ser executado
                                      ex: comando = ON/OFF, atributo = ON */
    uint8_t       APP_Payload[96];
} APP_PACKET;

void Decode_General_Profile(APP_PACKET *app);
INT8U NetGeneralInfo(INT8U MeasureType, INT8U Value8,INT16U Value16, INT16U destiny);
INT8U NetLightingProfile(INT8U Command, INT8U Parameter, INT8U Value8, INT16U Value16);

INT8U NetDebugPacket(INT8U Command, INT8U Parameter, INT8U Value8, INT16U Value16, INT16U destiny);

void SniferRep(INT16U atributte, INT16U destiny);
INT8U UNET_TX_ToBaseSation(INT8U Command, INT8U Attribute, _uint32_t * ptr_data, INT8U nbr_bytes);

uint8_t NetGeneralONOFF(uint8_t state, addr64_t *destiny);
INT8U NetGeneralCreateUpPath(void);

void Process_Debug_Packet(void); 


/* Return codes */
typedef enum {         
  NO_FAIL,              
  TASK_FAIL_INIT,
  NO_LOAD, 
  GENERIC_FAIL   
}FAIL_T;

void ReportFailure_Get(FAIL_T *failure_kind); 
void ReportFailure_Set(FAIL_T failure_kind);

enum ParamNames
{
    LUX_THRESHOLD_MIN,
    LUX_THRESHOLD_MAX,
    PARENT_THRESHOLD,
    PARENT_THRESHOLD_MIN,
    REPORT_PERIOD_1000MS,
    REPORT_JITTER_100MS
};

extern INT8U    Config_LUX_THRESHOLD_MIN;
extern INT8U    Config_LUX_THRESHOLD_MAX;
extern INT8U    Config_PARENT_THRESHOLD;
extern INT8U    Config_PARENT_THRESHOLD_MIN;
extern INT8U    Config_REPORT_PERIOD_1000MS;
extern INT8U    Config_REPORT_JITTER_100MS;

void GeneralResponse(void);
#endif
