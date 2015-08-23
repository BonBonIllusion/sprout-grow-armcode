/* delay us on timer */
/* author: Adam Orcholski, tath@o2.pl, www.tath.eu */

#include "timer.h"

void tim_init(TIM_TypeDef *p)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
 
	p->CR1 = 0;
	p->ARR = 0xffff;
	p->PSC = 83;
	p->EGR = TIM_EGR_UG;
}

void delay_us(TIM_TypeDef *p, uint16_t us)
{
	p->CNT = 0;
	while((uint16_t)(p->CNT) <= us);
}
