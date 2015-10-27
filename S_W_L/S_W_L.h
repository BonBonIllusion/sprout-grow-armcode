#ifndef S_W_L_H
#define S_W_L_H

#include "stdio.h"
#include <stdlib.h>
#include "time.h"
#include "Motor_LED_Control.h"
#include "string.h"
#include "main.h"

#define Water_Time_Diff 10;
#define Pour_Time_Diff 10;

struct Init_Time
{
	int year;
	int mon;
	int day;
	
	int hour;
	int min;
	int sec;
};


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






struct Schedule
{
	struct Soak_Time *soak_time;
	struct Water_Mode_Time *water_mode_time;
	struct Light_Time *light_time;
	struct Schedule *next;
};



long int Epoch_Converter(int year, int mon, int mday, int hour, int min, int sec);
void Soak(int day, int hour, int min);
void Water(int day, int hour, int min, int sec);
void Light(int mon, int day, int hour, int min);
void Soak_Str_Processing(int soak_total, int soak_change);
void Water_Str_Processing(int soak_total, char s[]) ;
void Light_Str_Processing(char light_date[], char s[]);
void Sever_Time_Str_Processing(long int epoch) ;
void Str_Split(char s[], struct Init_Time *time);

#endif //for S_W_L