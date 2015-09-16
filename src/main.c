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
#include "stdlib.h"
#include "S_W_L.h"


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
char schedule_string[50];
int schedule_got = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */



void RTC_Config()
{
	RTC_InitTypeDef RTC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);    /* Enable the PWR clock */
	PWR_BackupAccessCmd(ENABLE);                          /* Allow access to RTC */

	RCC_LSICmd(ENABLE);                                   /* Enable the LSI OSC */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);   /* Wait till LSI is ready */  
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);               /* Select the RTC Clock Source */

	RCC_RTCCLKCmd(ENABLE);                                /* Enable the RTC Clock */
	RTC_WaitForSynchro();                  /* Wait for RTC APB registers synchronisation */

	/* Configure the RTC data register and RTC prescaler */
	RTC_InitStructure.RTC_AsynchPrediv = 127;
	RTC_InitStructure.RTC_SynchPrediv = 249;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);
}


void Time_Date_Setting(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec)
{
	
	RTC_TimeTypeDef RTC_TimeStruct_main;
	RTC_DateTypeDef RTC_DateStruct_main;
	
	
	RTC_TimeStruct_main.RTC_Hours = hour;
	RTC_TimeStruct_main.RTC_Minutes = min;
	RTC_TimeStruct_main.RTC_Seconds = sec;
	
	RTC_DateStruct_main.RTC_Year = year;
	RTC_DateStruct_main.RTC_Month = mon;
	RTC_DateStruct_main.RTC_Date = day;
	
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct_main);
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct_main);
	
}


//if I start the soaking period at 11 O'clock and the total soaking period is 5 hours
//what I coded would not do well, because I can't deal with the overnight problem
//and if I solve the overnight problem, I have to be careful of the Water fun
//there may be Water fun conflicts Soak fun =>>Water fun starts at 2 am but Soak fun ends at 3 am



int main(void)
{
	char s[] = "1438246329,6,3,5,#0530W#1200P,3,#0800O#1900C#0900O#1800C";  // for str_processing test
	
	
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
	DNS_Init();
	get_schedule(&schedule_got,schedule_string); // schedule string store in schedule_string
	while(!schedule_got); // wait until string got
	
	LCD_DisplayStringLine(Line2, (uint8_t*)schedule_string);
	LCD_DisplayStringLine(Line3, (uint8_t*)"0");
	/* Main Loop */
	
	//process ste str form internet
	Str_Split(s);   // s is temp string
	RTC_Config();
	Time_Date_Setting(init_time.year, init_time.mon, init_time.day, init_time.hour +3, init_time.min, init_time.sec);
	
	
	
	
	while (1)
	{
		uint8_t year, mon, day;
		uint8_t hour, min, sec;
		RTC_TimeTypeDef RTC_TimeStruct_main;
		RTC_DateTypeDef RTC_DateStruct_main;
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct_main);
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct_main);
		
		year = RTC_DateStruct_main.RTC_Year;	
		mon = RTC_DateStruct_main.RTC_Month;
		day = RTC_DateStruct_main.RTC_Date;
		hour = RTC_TimeStruct_main.RTC_Hours;
		min = RTC_TimeStruct_main.RTC_Minutes;
		sec = RTC_TimeStruct_main.RTC_Seconds;
		
		//detect whether it is time to turn on Motor and LED, then execute it.
		Soak(day, hour, min );
		Water(day, hour, min, sec);
		Light(mon, day, hour, min);
		//detect over
			
		
		
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
