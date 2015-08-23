/* gpio peripheral library */
/* author: Adam Orcholski, tath@o2.pl, www.tath.eu */

#ifndef _gpio_def_
#define _gpio_def_

#include <stdint.h>
#include "stm32f4xx.h"

/* prototypes */
extern void gpio_out_cfg(GPIO_TypeDef*, uint16_t, GPIOMode_TypeDef, GPIOSpeed_TypeDef, GPIOOType_TypeDef, GPIOPuPd_TypeDef);
extern void gpio_in_cfg(GPIO_TypeDef*, uint16_t, GPIOMode_TypeDef, GPIOPuPd_TypeDef);
extern void gpio_set(GPIO_TypeDef*, uint16_t);
extern void gpio_clr(GPIO_TypeDef*, uint16_t);
extern uint8_t gpio_get_output(GPIO_TypeDef*, uint16_t);
extern uint8_t gpio_get_input(GPIO_TypeDef*, uint16_t);
#endif
