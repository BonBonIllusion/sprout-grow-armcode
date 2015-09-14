#include "Motor_LED_Control.h"
#include "main.h"  //�ȭ��Ʃw�q�A�i��|�����D
void Motor_On()
{
	//�������F�}�Үɶ�

	GPIO_InitTypeDef g;
	g.GPIO_Pin = Motor_Pin;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_PuPd = GPIO_PuPd_DOWN;
	g.GPIO_Speed = GPIO_Speed_25MHz;
	RCC_AHB1PeriphClockCmd(Motor_RCC, ENABLE);
	GPIO_Init(Motor_GPIO, &g);
	
	GPIO_SetBits(Motor_GPIO, Motor_Pin);
	
}
void Motor_Off()
{
	GPIO_ResetBits(Motor_GPIO, Motor_Pin);
	
}
void LED_On()
{
	//����LED�}�Үɶ�
	GPIO_InitTypeDef g;
	g.GPIO_Pin = LED_Pin;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_100MHz;
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);
	GPIO_Init(LED_GPIO, &g);
	
	//Turn on the LED
	GPIO_SetBits(LED_GPIO, LED_Pin);	
}
void LED_Off()
{
	GPIO_ResetBits(LED_GPIO, LED_Pin);
}
