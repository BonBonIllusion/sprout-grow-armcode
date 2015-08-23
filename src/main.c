/**
  ******************************************************************************
  * @file    Project/STM32F4_EVB_Demo/main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-March-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "lwip/tcp.h"
#include "lcd_log.h"
#include "dht11.h"

/** @addtogroup STM32F4xx_StdPeriph_Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS 	10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
static __IO uint32_t uwTimingDelay;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	RCC_ClocksTypeDef RCC_Clocks;

	/* SysTick end of count event each 10ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
	/* Initialize the timer for dht11 */
	tim_init(TIM2);
	/* Initialize the SRAM ****************************************************/
	PSRAM_Init();
	/* Initialize the LCD *****************************************************/
	LCD_Init();
	LCD_LOG_Init();
  LCD_LOG_SetHeader((uint8_t*)" Ethernet test");
	LCD_LOG_SetFooter ((uint8_t*)"     localtime: ");
	/* Add your application code here */
	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();
	/* Initilaize the LwIP stack */
	LwIP_Init();
	/* Main Loop */
	DNS_Init();
	while (1)
	{
		/* check if any packet received */
		if (ETH_CheckFrameReceived())
		{
			/* process received ethernet packet */
			LwIP_Pkt_Handle();
		}
		/* handle periodic timers for LwIP */
		LwIP_Periodic_Handle(LocalTime);
	}
}



/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
	uwTimingDelay = nTime;

	while (uwTimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
	if (uwTimingDelay != 0x00)
	{
		uwTimingDelay--;
	}
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
	LocalTime += SYSTEMTICK_PERIOD_MS;
	//LCD_LOG_PrintTime(LocalTime);
}

/*--------------------------------
       Callbacks implementation:
           the callbacks prototypes are defined in the stm32f4_evb_audio_codec.h file
           and their implementation should be done in the user coed if they are needed.
           Below some examples of callback implementations.
                                     --------------------------------------------------------*/

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
