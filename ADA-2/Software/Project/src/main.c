/* 	Oprogramowanie urzadzenia ADA-2 wykonanego na potrzeby projektu inzynierskiego
		"Projekt stanowiska laboratoryjnego do badania kodowania PCM 
		Pawel 'Gibol' Gibaszek
		Politechnika Wroclawska 2014 */


/* Includes ------------------------------------------------------------------*/ 
#include "main.h"

__IO uint16_t uhADCxConvertedValue[2] = {0};
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

static volatile uint8_t sampleBuffer[2000];
static volatile uint16_t dataCounter = 0;


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
	if(dataCounter == 1000)
	{
		uint16_t mod;
		int i,j;
		uint8_t frame[56];
		frame[0] = 100;
		frame[1] = 89;
		frame[55] = 101;
		
		for(j = 0; j < 2000/50; j++)
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
			VCP_DataTx(frame, 56);
		}
		dataCounter = 0;
	}
	}
} 


static void TIM_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  /* TIM7 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  /* Enable TIM7 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler =  42000 - 1;
  TIM_TimeBaseStructure.TIM_Period = 50 - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
  /* TIM7 enable counter */
  //TIM_Cmd(TIM7, ENABLE);
  /* TIM7 enable update interrupt */
  //TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);	
	
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

void TIM7_IRQHandler(void)
{

	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
}

void TIM6_DAC_IRQHandler(void)
{	
	uint16_t val1 = uhADCxConvertedValue[0], val2 = uhADCxConvertedValue[1];
  DAC_SetDualChannelData(DAC_Align_12b_R, val2, val1);
	if(dataCounter < 1000)
	{
		sampleBuffer[dataCounter] = (uint8_t) val1;
		sampleBuffer[dataCounter] = (uint8_t)(val1 >> 8);
		sampleBuffer[1000+dataCounter] = (uint8_t)val2 ;
		sampleBuffer[1000+dataCounter] = (uint8_t)(val2 >> 8);
		dataCounter+=2;
	}		
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);	
}

static void DAC_Config(void)
{
	GPIO_InitTypeDef      GPIO_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	 
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
