#ifndef MAIN_H
#define MAIN_H

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "g711.h"
#include "analog_companding.h"
#include <stdio.h>


/* CONFIG SECTION */
    typedef enum { F8KHZ, F11_025KHZ, F16KHZ, F22_05KHZ, F32KHZ, F44_1KHZ}                                          SampligFrequency;
    typedef enum { None, ADigital, MuDigital, AAnalog, MuAnalog, Approx13seg }                                      CompressionType;
    typedef enum { Word4bits=4, Word6bits=6, Word8bits=8, Word10bits = 10, Word12bits=12 }                          WordLenght;
    typedef enum { AnalogInput1, AnalogInput2, TestSignal1, TestSignal2 }                                           SignalSource;
    typedef enum { AnalogOutput1, AnalogOutput2}                                                                    SignalOutput;
    typedef enum { Bit0, Bit1, Bit2, Bit3, Bit4, Bit5, Bit6, Bit7, Bit8, Bit9, Bit10, Bit11, BitRandom, BitNone }   BitError;
    typedef enum { Start = 255, Stop = 0 }                                                                          StartStopCommand;
		typedef enum { Idle, Busy, Configured }                                                                         DeviceStatus;
		typedef enum { F_GROUP1, F_GROUP2 }																																							FrequencyGrpup;
		
    typedef struct {
        SampligFrequency samplingFrequency;
        WordLenght wordLenght;
        CompressionType compressionType;
        SignalSource signalSource;
        SignalOutput signalOutput;
        BitError bitError;
				float analogCompressionParam;
    } ADASettings;
		

		
#define SAMPLE_BUFFER_SIZE 500 //must devide by 50

/* EO CONFIG SECTION */

/* LED SECTION */
#define LED_GPIO 				GPIOD
#define LED_GREEN_Pin		GPIO_Pin_11
#define LED_YELLOW_Pin	GPIO_Pin_12
#define LED_CLOCK				RCC_AHB1Periph_GPIOD
typedef enum {Green, Yellow} LED;
void LED_Init(void);
void LED_On(LED led);
void LED_Off(LED led);
void LED_Toggle(LED led);
/* EO LED SECTION */

/* USB DEBUG SECTION */
void USB_Debug(uint8_t text[], uint8_t len);
/* EO USB DEBUG SECTION */

/* ANALOG SECTION */
#define ADC1_CHANNEL            ADC_Channel_7
#define ADC2_CHANNEL 						ADC_Channel_6
#define ADC1_CLK                RCC_APB2Periph_ADC1
#define ADC2_CLK								RCC_APB2Periph_ADC2
#define ANALOG_GPIO_CLK    			RCC_AHB1Periph_GPIOA
#define ADC1_GPIO_PIN           GPIO_Pin_7
#define ADC2_GPIO_PIN           GPIO_Pin_6
#define ANALOG_GPIO_PORT        GPIOA
#define ADC_DMA_CHANNEL         DMA_Channel_0
#define DMA_STREAMx             DMA2_Stream0
#define ADC_CCR_ADDRESS    			((uint32_t)0x40012308)
#define DAC1_GPIO_PIN GPIO_Pin_5
#define DAC2_GPIO_PIN GPIO_Pin_4
#define DAC_CLK RCC_APB1Periph_DAC
static void DAC_Config(void);
static void ADC_Config(void);
/* EO ANALOG SECTION */


/* TIMER SECTION */
#define N_DEFAULT F44_1KHZ
static void TIM_Init(void);
static void TIM_ChangeN(int N);
void TIM6_DAC_IRQHandler(void);
/* EO TIMER SECTION */

/* SIGNAL PROCESSING SECTION */
void compressSample(int16_t *sample);
void decompressSample(int16_t *sample);
int16_t getTestSignalSample(void);
void simulateError(int16_t *sample);
/* EO SIGNAL PROCESSING SECTION */

// Function prototypes
void get_dec_str (uint8_t* str, uint8_t len, uint32_t val);


#endif
