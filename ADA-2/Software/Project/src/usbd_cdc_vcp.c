/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_vcp.h"
#include "usb_conf.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LINE_CODING linecoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };



/* These are external variables imported from CDC core to be used for IN 
   transfer management. */
extern uint8_t  APP_Rx_Buffer []; /* Write CDC received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */
	
extern ADASettings currentSettings;
extern uint8_t configurationChangedFlag;	
extern DeviceStatus devStatus;
extern uint8_t deviceConfiguredFlag;
	
#define  CONFIG_FRAME_SIZE  		((uint8_t) 13)
#define  DISCOVERY_FRAME_SIZE 	((uint8_t) 5)
#define  CONVERSION_FRAME_SIZE  ((uint8_t) 6)
#define  START_BYTE	 						((uint8_t) 100)
#define  STOP_BYTE	 						((uint8_t) 101)
#define  DISCOVERY_FRAME	 			((uint8_t) 44)
#define  CONFIG_FRAME	 					((uint8_t) 55)
#define  CONVERSION_FRAME			  ((uint8_t) 66)
#define  LONGEST_FRAME_SIZE			CONFIG_FRAME_SIZE
static uint8_t currentIndex = 0;	

/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init     (void);
static uint16_t VCP_DeInit   (void);
static uint16_t VCP_Ctrl     (uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_COMConfig(uint8_t Conf);

CDC_IF_Prop_TypeDef VCP_fops = 
{
  VCP_Init,
  VCP_DeInit,
  VCP_Ctrl,
  VCP_DataTx,
  VCP_DataRx
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
  
  return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{

  return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl (uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{ 
  switch (Cmd)
  {
  case SEND_ENCAPSULATED_COMMAND:
    /* Not  needed for this driver */
    break;

  case GET_ENCAPSULATED_RESPONSE:
    /* Not  needed for this driver */
    break;

  case SET_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case GET_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case CLEAR_COMM_FEATURE:
    /* Not  needed for this driver */
    break;

  case SET_LINE_CODING:
    linecoding.bitrate = (uint32_t)(Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
    linecoding.format = Buf[4];
    linecoding.paritytype = Buf[5];
    linecoding.datatype = Buf[6];
    /* Set the new configuration */
    VCP_COMConfig(OTHER_CONFIG);
    break;

  case GET_LINE_CODING:
    Buf[0] = (uint8_t)(linecoding.bitrate);
    Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
    Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
    Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
    Buf[4] = linecoding.format;
    Buf[5] = linecoding.paritytype;
    Buf[6] = linecoding.datatype; 
    break;

  case SET_CONTROL_LINE_STATE:
    /* Not  needed for this driver */
    break;

  case SEND_BREAK:
    /* Not  needed for this driver */
    break;    
    
  default:
    break;
  }

  return USBD_OK;
}

/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in 
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
uint16_t VCP_DataTx (uint8_t* Buf, uint32_t Len)
{
	uint32_t tx_counter = 0;
	
	
	while(tx_counter < Len){
		APP_Rx_Buffer[APP_Rx_ptr_in] = *(Buf+tx_counter);
		
		APP_Rx_ptr_in++;
		
		/* To avoid buffer overflow */
		if(APP_Rx_ptr_in == APP_RX_DATA_SIZE)
		{
			APP_Rx_ptr_in = 0;
		}
		
		tx_counter++;
	}

  return USBD_OK;
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
uint16_t VCP_DataRx (uint8_t* Buf, uint32_t Len)
{
	
  uint32_t i;
	
	/* too much data in buffer - transmission error */
	if(Len > LONGEST_FRAME_SIZE) return USBD_OK;
	
	/* invalid data - transmission error */
	if(currentIndex == 0 && Buf[0] != START_BYTE)
	{
		currentIndex = 0;
		return USBD_OK;
	}
	
	/* check for discovery frame */
	if(Buf[0] == START_BYTE && Buf[1] == DISCOVERY_FRAME 
		&& Buf[DISCOVERY_FRAME_SIZE -3] == 0 && Buf[DISCOVERY_FRAME_SIZE -2] == 0 && Buf[DISCOVERY_FRAME_SIZE -1] == STOP_BYTE)
	{
		VCP_DataTx(Buf, 5);
		return USBD_OK;
	}
	
	/* check for conversion start stop command frame */
	if(Buf[0] == START_BYTE && Buf[1] == CONVERSION_FRAME && Buf[CONVERSION_FRAME_SIZE -1] == STOP_BYTE)
	{
		if(Buf[2] == Buf[4])
		{
			if(Buf[2] == Stop) 
			{
				devStatus = Idle;
				TIM_Cmd(TIM6, DISABLE); 
			}
			else if(Buf[2] == Start && deviceConfiguredFlag == 1) 
			{
				devStatus = Busy;
				TIM_Cmd(TIM6, ENABLE); 
			}
		}
	}
	
	/* check for config frame */
	if(Buf[0] == START_BYTE && Buf[1] == CONFIG_FRAME && Buf[CONFIG_FRAME_SIZE -1] == STOP_BYTE)
	{
				uint16_t mod = 0, mod2 = 0;
				for(i = 0; i < 8; i++)
				{
					mod += Buf[2 + i];
				}
				
				mod2 = (Buf[CONFIG_FRAME_SIZE -3] << 8);
				mod2 |= Buf[CONFIG_FRAME_SIZE -2];

				if(mod == mod2)
				{
					/* save settings */
					currentSettings.samplingFrequency = (SampligFrequency) 	Buf[2];
     			currentSettings.compressionType = 	(CompressionType) 	Buf[3];
					currentSettings.wordLenght =  			(WordLenght) 				Buf[4];
					currentSettings.signalSource = 			(SignalSource) 			Buf[5];
					currentSettings.signalOutput = 			(SignalOutput) 			Buf[6];
					currentSettings.bitError = 					(BitError) 					Buf[7];
					double temp = Buf[8];
					temp *= 10;
					temp += Buf[9];
					currentSettings.analogCompressionParam = temp/10.0f;
					configurationChangedFlag = 1;
					
					/* create and send response */
					Buf[2] = 0;
					Buf[3] = 0;
					Buf[4] = STOP_BYTE;
					VCP_DataTx(Buf, 5);
				}	
				return USBD_OK;
	}
  
  return USBD_OK;
}

/**
  * @brief  VCP_COMConfig
  *         Configure the COM Port with default values or values received from host.
  * @param  Conf: can be DEFAULT_CONFIG to set the default configuration or OTHER_CONFIG
  *         to set a configuration received from the host.
  * @retval None.
  */
static uint16_t VCP_COMConfig(uint8_t Conf)
{

  return USBD_OK;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
