/* delay us on timer */
/* author: Adam Orcholski, tath@o2.pl, www.tath.eu */

#ifndef _tim_def_
#define _tim_def_

#include <stm32f4xx.h>

extern void tim_init(TIM_TypeDef *);
extern void	delay_us(TIM_TypeDef *,uint16_t);
#endif
