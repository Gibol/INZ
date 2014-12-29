/* 	Oprogramowanie urzadzenia ADA-2 wykonanego na potrzeby projektu inzynierskiego
		"Projekt stanowiska laboratoryjnego do badania kodowania PCM 
		Pawel 'Gibol' Gibaszek
		Politechnika Wroclawska 2014 */


/* Includes ------------------------------------------------------------------*/ 
#include "main.h"

/* 1kHz Sine tables */
uint16_t SIN_44K[44] = {
	2048,2339,2624,2898,3154,3388,3595,3770,
3910,4012,4074,4095,4074,4012,3910,3770,
3595,3388,3154,2898,2624,2339,2048,1756,
1471,1197,941,707,500,325,185,83,
21,0,21,83,185,325,500,707,
941,1197,1471,1756 };

uint16_t SIN_32K[32] = {
	2048,2447,2831,3185,3495,3750,3939,4056,
4095,4056,3939,3750,3495,3185,2831,2447,
2048,1648,1264,910,600,345,156,39,
0,39,156,345,600,910,1264,1648};

/* System variables */
__IO uint16_t uhADCxConvertedValue[2] = {0};
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

/* Sample buffer for USB transmission */
static volatile uint8_t sampleBuffer[SAMPLE_BUFFER_SIZE];
static volatile uint16_t dataCounter = 0;

/* Configuration variables initialisation */
static SignalSource sigSource = TestSignal1;
static SignalOutput sigOutput = AnalogOutput1;
static SampligFrequency samFrequency = F44_1KHZ;
static FrequencyGroup freqGroup = F_GROUP1;
static DeviceStatus devStatus = Idle;
static WordLenght wordLen = Word12Bits;

int main(void)
{
  USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc, 
            &USBD_CDC_cb, 
            &USR_cb);
  
	if (SysTick_Config(SystemCoreClock / 1000))
  { 
    //Capture error
    while (1);
  }
	
	LED_Init();
	ADC_Config();
	ADC_SoftwareStartConv(ADC1);
	DAC_Config();
	TIM_Init();
	
	while (1)
  {
	
		/* Send collected samples when ready, and configured */
		if(dataCounter == SAMPLE_BUFFER_SIZE && devStatus == Busy)
		{
			uint16_t mod;
			int i,j;
			uint8_t frame[56];
			frame[0] = 100;
			frame[1] = 89;
			frame[55] = 101;
			
			for(j = 0; j < SAMPLE_BUFFER_SIZE/50; j++)
			{
				frame[2] = j;
				mod=0;
				for(i = 0; i < 50; i++)
				{
					frame[i+3] = sampleBuffer[(50*j) + i];
					mod += sampleBuffer[(50*j) + i];
				}
				
				frame[54] = (uint8_t)(mod & 0xFF);
				frame[53] = (uint8_t)(mod >> 8);
					LED_Toggle(Yellow);
				VCP_DataTx(frame, 56);
					LED_Toggle(Yellow);
			}
			dataCounter = 0;
		}
	}
} 


static void TIM_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  /* TIM6 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  /* Enable TIM6 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler =  N_DEFAULT - 1;
  TIM_TimeBaseStructure.TIM_Period = 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE); 
  /* TIM6 enable update interrupt */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
}

void TIM_ChangeN(int N)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM6, DISABLE); 
	
	TIM_TimeBaseStructure.TIM_Prescaler =  N - 1;
  TIM_TimeBaseStructure.TIM_Period = 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE); 
  /* TIM6 enable update interrupt */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	
}


void TIM6_DAC_IRQHandler(void)
{	
	uint16_t val;
  static uint8_t waveGenCnt = 0;
	
	if(sigSource == AnalogInput1)
	{
		val = uhADCxConvertedValue[0];
	}
	else if(sigSource == AnalogInput2)
	{
		val = uhADCxConvertedValue[1];
	}
	else if(sigSource == TestSignal1)
	{
		
		if(freqGroup == F_GROUP1)
		{
			val = SIN_44K[waveGenCnt];
			if(samFrequency == F44_1KHZ)
			{
				
				waveGenCnt++;	
			}
			else if(samFrequency == F22_05KHZ)
			{
				
				waveGenCnt+=2;
			}
			else if(samFrequency == F11_025KHZ)
			{
				
				waveGenCnt+=4;
			}
		
		if(waveGenCnt >= 44) waveGenCnt = 0;
		}
		else if(freqGroup == F_GROUP2)
		{
			val = SIN_32K[waveGenCnt];
			if(samFrequency == F32KHZ)
			{
				
				waveGenCnt++;	
			}
			else if(samFrequency == F16KHZ)
			{
				
				waveGenCnt+=2;
			}
			else if(samFrequency == F8KHZ)
			{
				
				waveGenCnt+=4;
			}
			if(waveGenCnt >= 32) waveGenCnt = 0;
		}
		
		
		
	}
	
  if(sigOutput == AnalogOutput1)
	{
		DAC_SetChannel1Data(DAC_Align_12b_R, val);
	}
	else if(sigOutput == AnalogOutput2)
	{
		DAC_SetChannel2Data(DAC_Align_12b_R, val);
	}
	
	if(dataCounter < SAMPLE_BUFFER_SIZE)
	{
		sampleBuffer[dataCounter+1] = (uint8_t) val ;
		sampleBuffer[dataCounter] = ((uint8_t)(val >> 8)&0xFF) ;
		dataCounter++;
	}		
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);	
}

static void DAC_Config(void)
{
	GPIO_InitTypeDef      GPIO_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;
	
	/* Enable DAC Clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	 
	/* GPIO Configuration */
	GPIO_InitStructure.GPIO_Pin = DAC1_GPIO_PIN | DAC2_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(ANALOG_GPIO_PORT , &GPIO_InitStructure);	
	
	/* DAC Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	
	/* Enable DAC Channel1 */
  DAC_Cmd(DAC_Channel_1, ENABLE);
	/* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);
	
}

static void ADC_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Enable ADCx, DMA and GPIO clocks ****************************************/ 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(ANALOG_GPIO_CLK, ENABLE);  
  RCC_APB2PeriphClockCmd(ADC1_CLK | ADC2_CLK, ENABLE);

  /* DMA2 Stream0 channel2 configuration **************************************/
  DMA_InitStructure.DMA_Channel = ADC_DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC_CCR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uhADCxConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA_STREAMx, &DMA_InitStructure);
  DMA_Cmd(DMA_STREAMx, ENABLE);

  /* Configure ADC pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = ADC1_GPIO_PIN | ADC2_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(ANALOG_GPIO_PORT, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Init(ADC2, &ADC_InitStructure);

  /* ADC regular channel configuration **************************************/
  ADC_RegularChannelConfig(ADC1, ADC1_CHANNEL, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC2, ADC2_CHANNEL, 1, ADC_SampleTime_3Cycles);

 /* Enable DMA request after last transfer (Multi-ADC mode) */
  ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);

  /* Enable ADC DMA */
  ADC_DMACmd(ADC1, ENABLE);
	
  /* Enable ADC */
  ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);
}

void USB_Debug(uint8_t text[], uint8_t len)
{
	const uint8_t crlfnul[3] = "\r\n\0";
	VCP_DataTx((uint8_t*)text, len);
	VCP_DataTx((uint8_t*)crlfnul, 3);
}

void get_dec_str (uint8_t* str, uint8_t len, uint32_t val)
{
  uint8_t i;
  for(i=0; i<len; i++)
  {
    str[len-1-i] = (uint8_t) ((val % 10UL) + '0');
    val/=10;
  }

}

void LED_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(LED_CLOCK, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = LED_GREEN_Pin | LED_YELLOW_Pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LED_GPIO, &GPIO_InitStructure);
	
	LED_Off(Green);
	LED_Off(Yellow);
	
}

void LED_On(LED led)
{
	uint16_t pin;
	if(led == Green)
	{
		pin = LED_GREEN_Pin;
	}
	else if(led == Yellow)
	{
		pin = LED_YELLOW_Pin;
	}
	else
	{
		return;
	}
	
	GPIO_SetBits(LED_GPIO, pin);
}
	
void LED_Off(LED led)
{
		uint16_t pin;
	if(led == Green)
	{
		pin = LED_GREEN_Pin;
	}
	else if(led == Yellow)
	{
		pin = LED_YELLOW_Pin;
	}
	else
	{
		return;
	}
	
	GPIO_ResetBits(LED_GPIO, pin);
}

void LED_Toggle(LED led)
{
			uint16_t pin;
	if(led == Green)
	{
		pin = LED_GREEN_Pin;
	}
	else if(led == Yellow)
	{
		pin = LED_YELLOW_Pin;
	}
	else
	{
		return;
	}
	
	if(GPIO_ReadOutputDataBit(LED_GPIO, pin))
	{
		GPIO_ResetBits(LED_GPIO, pin);
	}
	else
	{
		GPIO_SetBits(LED_GPIO, pin);
	}
}

#ifdef USE_FULL_ASSERT
/**
* @brief  assert_failed
*         Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  File: pointer to the source file name
* @param  Line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {}
}
#endif
