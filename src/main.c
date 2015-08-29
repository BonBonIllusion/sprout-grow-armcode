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
#include "time.h"
#include "Motor_LED_Control.h"


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

#define Water_Time_Diff 5;
#define Pour_Time_Diff 10;

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
int Enter_Water = 0;     //0 for cant enter Water, 1 for can enter Water
int Soak_Drain_Time = 5; //in min.  when soaking, the period should end a little earlier for draining
int Soak_Drain = 1;      //when soaking, decides which action is been boing. 
												 //0 for not in soak period. 1 for in soaking. 2 for in draining
int Light_Period = 0;    //0 for doesn't start, 1 for start Light_Period

struct Init_Time
{
	int year;
	int mon;
	int day;
	
	int hour;
	int min;
	int sec;
};
struct Init_Time init_time;


//string spilt
char *Internet_Str[7];

struct Soak_Time 
{
	char *str;
	char *ch_hour;
	char *ch_min;
	
	long int start_epoch;
	
	int start_mon;
	int start_day;
	int start_hour;
	int start_min;
	int start_sec;
	
	long int end_epoch;
	
	int end_mon;
	int end_day;
	int end_hour;
	int end_min;
	int end_sec;
	

	struct Soak_Time *next;
};


struct Soak_Time *soak_time_head = NULL;
struct Soak_Time *soak_time_cur = NULL;
struct Soak_Time *soak_time_prev = NULL;
int soak_time_end_hour;
int soak_time_end_min;
long int soak_time_end_epoch;

struct Water_Mode_Time // 2 mode. 1st for short time, 2nd for long time
{
	char *str;
	char *ch_hour;
	char *ch_min;
	
	long int start_epoch;
	
	int start_hour;
	int start_min;
	int start_sec;
	
	long int end_epoch;
	
	int end_hour;
	int end_min;
	int end_sec;
	
	struct Water_Mode_Time *next;
};

struct Water_Mode_Time *water_time_head = NULL;
struct Water_Mode_Time *water_time_cur = NULL;
struct Water_Mode_Time *water_time_prev = NULL;
int water_start_day;


struct Light_Time 
{
	char *str;
	char *ch_hour;
	char *ch_min;
	
	long int start_epoch;
	
	int start_mon;
	int start_day;
	int start_hour;
	int start_min;
	int start_sec;
	
	long int end_epoch;
	
	int end_mon;
	int end_day;
	int end_hour;
	int end_min;
	int end_sec;
	

	struct Light_Time *next;
};


struct Light_Time *light_time_head = NULL;
struct Light_Time *light_time_cur = NULL;
struct Light_Time *light_time_prev = NULL;




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
	/*
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	*/
	
	RTC_TimeStruct.RTC_Hours = hour;
	RTC_TimeStruct.RTC_Minutes = min;
	RTC_TimeStruct.RTC_Seconds = sec;
	
	RTC_DateStruct.RTC_Year = year;
	RTC_DateStruct.RTC_Month = mon;
	RTC_DateStruct.RTC_Date = day;
	
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
	
}


//if I start the soaking period at 11 O'clock and the total soaking period is 5 hours
//what I coded would not do well, because I can't deal with the overnight problem
//and if I solve the overnight problem, I have to be careful of the Water fun
//there may be Water fun conflicts Soak fun =>>Water fun starts at 2 am but Soak fun ends at 3 am
long int Epoch_Converter(int year, int mon, int mday, int hour, int min, int sec)
{
	time_t n;
  struct tm time;
		
	time.tm_year = year - 1900;
	time.tm_mon = mon - 1;
	time.tm_mday = mday;
	time.tm_hour = hour;
	time.tm_min = min;
	time.tm_sec = sec;

 	
}

void Soak(int day, int hour, int min)  
{
	
	if(Soak_Drain == 0)
	{
	}
	else if(Soak_Drain == 1) // for Soak
	{
		if(soak_time_cur->start_hour == hour && soak_time_cur->start_min == min)
		{
			
			Motor_On();
			//Delay forr unknown secs  <<< needs to be measured 
			Motor_Off();	
			
			Soak_Drain = 2;  // as soon as it starts to soak, Soak_Drain change to 2
		}	
		else if(soak_time_end_hour == hour && soak_time_end_min == min)
		{
			Motor_On();
			//Delay forr unknown secs  <<< needs to be measured 
			Motor_Off();
			
			Soak_Drain = 0;
		}		
	}
	else if(Soak_Drain == 2)  // for Drain
	{
		if(soak_time_cur->end_hour == hour && soak_time_cur->end_min == min)
		{
			Motor_On();
			//Delay forr unknown secs  <<< needs to be measured 
			Motor_Off();
			
			Soak_Drain = 1;  // as soon as it starts to drain, Soak_Drain change to 1
			
			soak_time_cur = soak_time_cur->next;
		}
		else if(soak_time_end_hour == hour && soak_time_end_min == min)
		{
			Motor_On();
			//Delay forr unknown secs  <<< needs to be measured 
			Motor_Off();
			
			Soak_Drain = 0;
		}
	}
}
void Water(int day, int hour, int min, int sec)
{	
	if(water_start_day == day)
	{
		Enter_Water = 1;
	}
	
	if(Soak_Drain == 0 && Enter_Water == 1)
	{
		if(water_time_cur->start_hour == hour && water_time_cur->start_min == min && water_time_cur->start_sec == sec)
		{		
			Motor_On();
		}
		else if(water_time_cur->end_hour == hour && water_time_cur->end_min == min && water_time_cur->end_sec == sec)
		{
			water_time_cur = water_time_cur->next;
		}		
	}
			{
				water_time_cur = water_time_head;
			}
			else
			{
				water_time_cur = water_time_cur->next;
			}		
		}
	}	
}



void Light(int mon, int day, int hour, int min)
{
	if(light_time_cur->start_day == day || Light_Period == 1)
	{
		Light_Period = 1;
		
		if(light_time_cur->start_hour == hour && light_time_cur->start_min == min)
		{		
			LED_On();
		}
		else if(light_time_cur->end_hour == hour && light_time_cur->end_min == min)
		{
			LED_Off();
			if(light_time_cur->next == NULL)
			{
				light_time_cur = light_time_head;
			}
			else
			{
				light_time_cur = light_time_cur->next;
			}		
		}
	}
	else
	{
		//before light on
	}	
}

void Soak_Str_Processing(int soak_total, int soak_change) //passed test
{
	int i = 0;
	int times = soak_total/soak_change;
	struct tm *time_ptr;
	
	for(i = 0; i < times; i++)
	{
		soak_time_cur = (struct Soak_Time *)malloc(sizeof(struct Soak_Time));
		soak_time_cur->next = NULL;
		
		if( i == 0) //head
		{
			// set the time
			soak_time_cur->start_hour = init_time.hour;
			soak_time_cur->start_min = init_time.min;
						
			if(init_time.min < Soak_Drain_Time)
			{
				soak_time_cur->end_hour = init_time.hour + soak_change - 1;
				soak_time_cur->end_min = init_time.min - Soak_Drain_Time + 60;								
			}
			else  //gernerl way 
			{
				soak_time_cur->end_hour = init_time.hour + soak_change;
				soak_time_cur->end_min = init_time.min - Soak_Drain_Time;
			}
						
			soak_time_head = soak_time_cur;
		}
		else
		{			
			// set the time
			soak_time_cur->start_hour = soak_time_prev->start_hour + soak_change;
			soak_time_cur->start_min = soak_time_prev->start_min;
					
			if(soak_time_prev->start_min < Soak_Drain_Time)
			{
				soak_time_cur->end_hour = soak_time_cur->start_hour + soak_change - 1;
				soak_time_cur->end_min = soak_time_cur->start_min - Soak_Drain_Time + 60;								
			}
			else  //gernerl way 
			{
				soak_time_cur->end_hour = soak_time_cur->start_hour + soak_change;
				soak_time_cur->end_min = soak_time_cur->start_min - Soak_Drain_Time;
			}
			
			soak_time_prev->next = soak_time_cur;
		}
		
		soak_time_prev = soak_time_cur;
	}
	if(soak_time_head != NULL)
	{
		soak_time_cur = soak_time_head;
		soak_time_end_hour = init_time.hour + soak_total; //if init_time add soak_total over 24 there would be prblem
		soak_time_end_min = init_time.min;
		soak_time_end_epoch = Epoch_Converter(init_time.year, init_time.mon, init_time.day, soak_time_end_hour, soak_time_end_min,0);
		time_ptr = localtime(&soak_time_end_epoch);
		soak_time_end_hour = time_ptr->tm_hour;
	}
	else
	{
		Soak_Drain = 0;
	}

}

void Water_Str_Processing(int soak_total, char s[]) //passed test
{
	// set the start_time
	int num;
	
	struct tm *time_ptr;
	do
	{
		water_time_cur = (struct Water_Mode_Time *)malloc(sizeof(struct Water_Mode_Time));
		water_time_cur->next = NULL;


		if(water_time_head == NULL)
		{
			water_time_cur->str = strtok(s, "#");
									
			num = atoi(water_time_cur->str);
			water_time_cur->start_hour = num/100;
			water_time_cur->start_min = num%100;
			water_time_cur->start_sec = 0;
			
			if(soak_total + init_time.hour > 24)
			{
				water_start_day = init_time.day + 1;
			}
			else
			{
				water_start_day = init_time.day;
			}
			water_time_cur->start_epoch = Epoch_Converter(init_time.year, init_time.mon, water_start_day, water_time_cur->start_hour, water_time_cur->start_min, water_time_cur->start_sec);
			
			water_time_cur->end_hour = water_time_cur->start_hour;
			water_time_cur->end_min = water_time_cur->start_min;
			
			//scan which action it should do
			if(strcspn(water_time_cur->str, "W") == 4)  //for water
			{
				water_time_cur->end_sec = water_time_cur->start_sec + Water_Time_Diff;
			}
			else  //for pour
			{
				water_time_cur->end_sec = water_time_cur->start_sec + Pour_Time_Diff;
			}
			
			water_time_cur->end_epoch = Epoch_Converter(init_time.year, init_time.mon, water_start_day, water_time_cur->end_hour, water_time_cur->end_min, water_time_cur->end_sec);
			
			water_time_head = water_time_cur;
		}
		else
		{
			water_time_cur->str = strtok(NULL, "#");
			
			num = atoi(water_time_cur->str);
			water_time_cur->start_hour = num/100;
			water_time_cur->start_min = num%100;
			water_time_cur->start_sec = 0;
			
			if(soak_total + init_time.hour > 24)
			{
				water_start_day = init_time.day + 1;
			}
			else
			{
				water_start_day = init_time.day;
			}
			water_time_cur->start_epoch = Epoch_Converter(init_time.year, init_time.mon, water_start_day, water_time_cur->start_hour, water_time_cur->start_min, water_time_cur->start_sec);
			
			water_time_cur->end_hour = water_time_cur->start_hour;
			water_time_cur->end_min = water_time_cur->start_min;
						
			//scan which action it should do
			if(strcspn(water_time_cur->str, "W") == 4)  //for water
			{
				water_time_cur->end_sec = water_time_cur->start_sec + Water_Time_Diff;
			}
			else  //for pour
			{
				water_time_cur->end_sec = water_time_cur->start_sec + Pour_Time_Diff;
			}
			
			water_time_cur->end_epoch = Epoch_Converter(init_time.year, init_time.mon, water_start_day, water_time_cur->end_hour, water_time_cur->end_min, water_time_cur->end_sec);
			
			water_time_prev->next = water_time_cur;
		}
		water_time_prev = water_time_cur;
		
	} while (water_time_cur->str != NULL);
	
	
	water_time_cur = water_time_head;  // for the poinnter start form head
	
	while(soak_time_end_epoch > water_time_cur->start_epoch)
	{
		water_time_cur = water_time_cur->next;
		if(water_time_cur == NULL)
		{
			water_time_cur = water_time_head;  
			water_start_day++;
			
		}
	}
	water_time_cur->start_epoch = Epoch_Converter(init_time.year, init_time.mon, water_start_day, water_time_cur->start_hour, water_time_cur->start_min, water_time_cur->start_sec);			
	time_ptr = localtime(&water_time_cur->start_epoch);
	water_start_day = time_ptr->tm_mday;
	
	
}
void Light_Str_Processing(char light_date[], char s[])  //passed test 
{
	
	//set the start_time
	int num;
	int LD = atoi(light_date);
	
	struct tm *nPtr;
	
	do 
	{
		light_time_cur = (struct Light_Time *)malloc(sizeof(struct Light_Time));
		light_time_cur->next = NULL;


		if(light_time_head == NULL)
		{
			
			light_time_cur->str = strtok(s, "#"); //for turn on		

			num = atoi(light_time_cur->str);
			
			light_time_cur->start_hour = num/100;
			light_time_cur->start_min = num%100;
			light_time_cur->start_day = init_time.day + LD - 1;
			
			light_time_cur->start_epoch = Epoch_Converter(init_time.year, init_time.mon, light_time_cur->start_day, light_time_cur->start_hour, light_time_cur->start_min, 0);
			nPtr = localtime(&light_time_cur->start_epoch);
			light_time_cur->start_day = nPtr->tm_mday;		
			
			
			
			light_time_cur->str = strtok(NULL, "#");//for turn off
			
			num = atoi(light_time_cur->str);
			light_time_cur->end_hour = num/100;
			light_time_cur->end_min = num%100;

			
						
			light_time_head = light_time_cur;
		}
		else
		{			
			light_time_cur->str = strtok(NULL, "#"); //for turn on
			
			num = atoi(light_time_cur->str);
			
			light_time_cur->start_hour = num/100;
			light_time_cur->start_min = num%100;
			
		
			light_time_cur->str = strtok(NULL, "#");//for turn off
			
			num = atoi(light_time_cur->str);
			light_time_cur->end_hour = num/100;
			light_time_cur->end_min = num%100;
			
			
			light_time_prev->next = light_time_cur; 
		}
		

		light_time_prev = light_time_cur;
	} while (light_time_cur->str != NULL);
	
	light_time_cur = light_time_head;
}
void Sever_Time_Str_Processing(long int epoch) // passed test
{
	time_t time = epoch;
	struct tm *tm_ptr = localtime(&time);
	init_time.year = tm_ptr->tm_year + 1900;
	init_time.mon = tm_ptr->tm_mon + 1;
	init_time.day = tm_ptr->tm_mday;
	init_time.hour = tm_ptr->tm_hour;   // maybe is GMT. have to be careful about it
  init_time.min = tm_ptr->tm_min;
  init_time.sec = tm_ptr->tm_sec;			
}
void Str_Split(char s[]) //s[] is the str from the Internet    //passed test
 {
	 int i =0;
    
   Internet_Str[i] = strtok(s, ",");
    
   while (Internet_Str[i] != NULL &&  i < 6)
	 {
      i++;
      Internet_Str[i] = strtok(NULL, ",");             
   }	 	 
	 
	 Sever_Time_Str_Processing(atoi(Internet_Str[0]));
	 Soak_Str_Processing(atoi(Internet_Str[1]), atoi(Internet_Str[2]));
	 Water_Str_Processing(atoi(Internet_Str[1]), Internet_Str[4]);
	 Light_Str_Processing(Internet_Str[5], Internet_Str[6]);
}


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
	get_schedule(schedule_got,schedule_string); // schedule string store in schedule_string
	while(!schedule_got); // wait until string got
	
	/* Main Loop */
	
	//process ste str form internet
	Str_Split(s);   // s is temp string
	RTC_Config();
	Time_Date_Setting(init_time.year, init_time.mon, init_time.day, init_time.hour +3, init_time.min, init_time.sec);
	
	
	
	
	while (1)
	{
		uint8_t year, mon, day;
		uint8_t hour, min, sec;
		
		RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		
		year = RTC_DateStruct.RTC_Year;	
		mon = RTC_DateStruct.RTC_Month;
		day = RTC_DateStruct.RTC_Date;
		hour = RTC_TimeStruct.RTC_Hours;
		min = RTC_TimeStruct.RTC_Minutes;
		sec = RTC_TimeStruct.RTC_Seconds;
		
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
