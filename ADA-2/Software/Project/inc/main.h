#ifndef MAIN_H
#define MAIN_H

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include <stdio.h>

/* LED SECTION */
#define LED_GPIO 				GPIOD
#define LED_GREEN_Pin		GPIO_Pin_12
#define LED_YELLOW_Pin	GPIO_Pin_11
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
/* EO ADC SECTION */

/* DAC SECTION */



/* EO DAC SECTION */

/* TIMER SECTION */
#define N_44_1KHZ 952
#define N_32KHZ 1312
#define N_22_05KHZ 1904
#define N_16KHZ 2625
#define N_11_025KHZ 3809
#define N_8KHZ 5250
#define N_DEFAULT N_22_05KHZ
static void TIM_Init(void);
static void TIM_ChangeN(int N);
void TIM7_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
/* EO TIMER SECTION */


// Function prototypes
void get_dec_str (uint8_t* str, uint8_t len, uint32_t val);


// Exported variables
//extern volatile uint32_t system_time_counter;

#endif