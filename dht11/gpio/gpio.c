/* gpio peripheral library */
/* author: Adam Orcholski, tath@o2.pl, www.tath.eu */

#include "gpio.h"

void gpio_out_cfg(GPIO_TypeDef* p, uint16_t pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed, GPIOOType_TypeDef otype, GPIOPuPd_TypeDef pupd)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
  GPIO_InitStructure.GPIO_Pin = pin;
  GPIO_InitStructure.GPIO_Mode = mode;
	GPIO_InitStructure.GPIO_Speed = speed;
  GPIO_InitStructure.GPIO_OType = otype;
  GPIO_InitStructure.GPIO_PuPd = pupd;
  GPIO_Init(p, &GPIO_InitStructure);
}

void gpio_in_cfg(GPIO_TypeDef* p, uint16_t pin, GPIOMode_TypeDef mode, GPIOPuPd_TypeDef pupd)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
  GPIO_InitStructure.GPIO_Pin = pin;
  GPIO_InitStructure.GPIO_Mode = mode;
  GPIO_InitStructure.GPIO_PuPd = pupd;
  GPIO_Init(p, &GPIO_InitStructure);
}

/* gpio set */
void gpio_set(GPIO_TypeDef* p, uint16_t pin)
{
	GPIO_WriteBit(p, pin, Bit_SET);
}

/* gpio clear */
void gpio_clr(GPIO_TypeDef* p, uint16_t pin)
{
	GPIO_WriteBit(p, pin, Bit_RESET);
}

/* gpio output data */
uint8_t gpio_get_output(GPIO_TypeDef* p, uint16_t pin)
{
 return GPIO_ReadOutputDataBit(p, pin);
}

/* gpio input data */
uint8_t gpio_get_input(GPIO_TypeDef* p, uint16_t pin)
{
 return GPIO_ReadInputDataBit(p, pin);
}
