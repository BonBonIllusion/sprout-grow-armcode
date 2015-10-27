#include "S_W_L.h"

//RTC_TimeTypeDef RTC_TimeStruct;
//RTC_DateTypeDef RTC_DateStruct;

int Enter_Water = 0;     //0 for cant enter Water, 1 for can enter Water
int Soak_Drain_Time = 5; //in min.  when soaking, the period should end a little earlier for draining
int Soak_Drain = 1;      //when soaking, decides which action is been boing. 
												 //0 for not in soak period. 1 for in soaking. 2 for in draining
int Light_Period = 0;    //0 for doesn't start, 1 for start Light_Period

char *Internet_Str[7];//string spilt

struct Soak_Time *soak_time_head = NULL;
struct Soak_Time *soak_time_cur = NULL;
struct Soak_Time *soak_time_prev = NULL;
int soak_time_end_hour;
int soak_time_end_min;
long int soak_time_end_epoch;

struct Water_Mode_Time *water_time_head = NULL;
struct Water_Mode_Time *water_time_cur = NULL;
struct Water_Mode_Time *water_time_prev = NULL;
int water_start_day;

struct Light_Time *light_time_head = NULL;
struct Light_Time *light_time_cur = NULL;
struct Light_Time *light_time_prev = NULL;

struct Schedule *schedule_head = NULL;
struct Schedule *schedule_cur = NULL;
struct Schedule *schedule_prev = NULL;

struct Init_Time *init_time;

long int Epoch_Converter(int year, int mon, int mday, int hour, int min, int sec)
{
	long int n;
  struct tm time;
		
	time.tm_year = year - 1900;
	time.tm_mon = mon - 1;
	time.tm_mday = mday;
	time.tm_hour = hour;
	time.tm_min = min;
	time.tm_sec = sec;

 	n = mktime(&time);
	return n;
}

void Soak(int day, int hour, int min)  
{
	
	if(Soak_Drain == 0 || soak_time_cur == NULL)
	{
	}
	else if(Soak_Drain == 1) // for Soak
	{
		if(soak_time_cur->start_hour == hour && soak_time_cur->start_min == min)
		{
			
			Motor_On();
			Delay(30000); 
			Motor_Off();	
			
			Soak_Drain = 2;  // as soon as it starts to soak, Soak_Drain change to 2
		}	
		else if(soak_time_end_hour == hour && soak_time_end_min == min)
		{
			Motor_On();
			Delay(20000);
			Motor_Off();
			
			Soak_Drain = 0;
		}		
	}
	else if(Soak_Drain == 2)  // for Drain
	{
		if(soak_time_cur->end_hour == hour && soak_time_cur->end_min == min)
		{
			Motor_On();
			Delay(20000);
			Motor_Off();
			
			Soak_Drain = 1;  // as soon as it starts to drain, Soak_Drain change to 1
			
			soak_time_cur = soak_time_cur->next;
		}
		else if(soak_time_end_hour == hour && soak_time_end_min == min)
		{
			Motor_On();
			Delay(30000);
			Motor_Off();
			
			Soak_Drain = 0;
		}
	}
}
void Water(int day, int hour, int min, int sec)
{	
	if(water_time_cur->start_hour == hour && water_time_cur->start_min == min && water_time_cur->start_sec == sec)
	{		
		Motor_On();
	}
	else if(water_time_cur->end_hour == hour && water_time_cur->end_min == min && water_time_cur->end_sec == sec)
	{
		Motor_Off();
		if(water_time_cur->next == NULL)
		{
			water_time_cur = water_time_head;
		}
		else
		{
			water_time_cur = water_time_cur->next;
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
	if(soak_total == 0 || soak_change == 0)
	{
		soak_time_cur = NULL;		
	}
	else
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
				soak_time_cur->start_hour = init_time->hour;
				soak_time_cur->start_min = init_time->min;
							
				if(init_time->min < Soak_Drain_Time)
				{
					soak_time_cur->end_hour = init_time->hour + soak_change - 1;
					soak_time_cur->end_min = init_time->min - Soak_Drain_Time + 60;								
				}
				else  //gernerl way 
				{
					soak_time_cur->end_hour = init_time->hour + soak_change;
					soak_time_cur->end_min = init_time->min - Soak_Drain_Time;
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
			soak_time_end_hour = init_time->hour + soak_total; //if init_time add soak_total over 24 there would be prblem
			soak_time_end_min = init_time->min;
			soak_time_end_epoch = Epoch_Converter(init_time->year, init_time->mon, init_time->day, soak_time_end_hour, soak_time_end_min,0);
			time_ptr = localtime(&soak_time_end_epoch);
			soak_time_end_hour = time_ptr->tm_hour;
		}
		else
		{
			Soak_Drain = 0;
		}
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
			
			if(soak_total + init_time->hour > 24)
			{
				water_start_day = init_time->day + 1;
			}
			else
			{
				water_start_day = init_time->day;
			}
			water_time_cur->start_epoch = Epoch_Converter(init_time->year, init_time->mon, water_start_day, water_time_cur->start_hour, water_time_cur->start_min, water_time_cur->start_sec);
			
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
			
			water_time_cur->end_epoch = Epoch_Converter(init_time->year, init_time->mon, water_start_day, water_time_cur->end_hour, water_time_cur->end_min, water_time_cur->end_sec);
			
			water_time_head = water_time_cur;
		}
		else
		{
			water_time_cur->str = strtok(NULL, "#");
			
			num = atoi(water_time_cur->str);
			water_time_cur->start_hour = num/100;
			water_time_cur->start_min = num%100;
			water_time_cur->start_sec = 0;
			
			if(soak_total + init_time->hour > 24)
			{
				water_start_day = init_time->day + 1;
			}
			else
			{
				water_start_day = init_time->day;
			}
			water_time_cur->start_epoch = Epoch_Converter(init_time->year, init_time->mon, water_start_day, water_time_cur->start_hour, water_time_cur->start_min, water_time_cur->start_sec);
			
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
			
			water_time_cur->end_epoch = Epoch_Converter(init_time->year, init_time->mon, water_start_day, water_time_cur->end_hour, water_time_cur->end_min, water_time_cur->end_sec);
			
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
	water_time_cur->start_epoch = Epoch_Converter(init_time->year, init_time->mon, water_start_day, water_time_cur->start_hour, water_time_cur->start_min, water_time_cur->start_sec);			
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
			light_time_cur->start_day = init_time->day + LD - 1;
			
			light_time_cur->start_epoch = Epoch_Converter(init_time->year, init_time->mon, light_time_cur->start_day, light_time_cur->start_hour, light_time_cur->start_min, 0);
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
	init_time->year = tm_ptr->tm_year + 1900;
	init_time->mon = tm_ptr->tm_mon + 1;
	init_time->day = tm_ptr->tm_mday;
	init_time->hour = tm_ptr->tm_hour;   // maybe is GMT. have to be careful about it
  init_time->min = tm_ptr->tm_min;
  init_time->sec = tm_ptr->tm_sec;			
}
void Str_Split(char s[], struct Init_Time *time) //s[] is the str from the Internet    //passed test
 {
	 int i =0;
   Internet_Str[i] = strtok(s, ",");
   init_time = time;
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