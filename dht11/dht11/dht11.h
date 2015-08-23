/* simple delay based dht11 library */
/* author: Adam Orcholski, www.tath.eu, tath@o2.pl */

#ifndef _dht_def_
#define _dht_def_

#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio.h"
#include "timer.h"

/* configuration macros: */
/* enable timer setup - optional and implementation defined */
#define ENABLE_TIMER	TIM2->CR1 |= TIM_CR1_CEN
#define DISABLE_TIMER TIM2->CR1 &= ~TIM_CR1_CEN

/* micro seconds delay function */
#define DELAY_US(__time__) delay_us(TIM2, __time__)

/* GPIO port name and number for 1-wire data input/output */
#define GPIO_PORT_NAME		GPIOF
#define GPIO_PORT_NUMBER	GPIO_Pin_7

/* GPIO configuration defines */
#define SET_GPIO_AS_OUTPUT gpio_out_cfg(GPIO_PORT_NAME, GPIO_PORT_NUMBER, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)
#define SET_GPIO_AS_INPUT gpio_in_cfg(GPIO_PORT_NAME, GPIO_PORT_NUMBER, GPIO_Mode_IN, GPIO_PuPd_NOPULL)

/* GPIO actions */
#define GPIO_CLEAR gpio_clr(GPIO_PORT_NAME, GPIO_PORT_NUMBER) /* clear port state */
#define GPIO_SET gpio_set(GPIO_PORT_NAME, GPIO_PORT_NUMBER) /* set port state to 1 */
#define GPIO_GET_INPUT gpio_get_input(GPIO_PORT_NAME, GPIO_PORT_NUMBER)	/* should return 0 or 1 */

/* optional critical section (because of delays slow as 30us) */
#define CRITICAL_SECTION_DECL
#define CRITICAL_SECTION_PROTECT
#define CRITICAL_SECTION_UNPROTECT

/* optional timeouts for while() loops (in case of hardware failure) */
#define ENABLE_TIMEOUTS /* comment to not perform timeout checks */
#define TIMEOUT_VALUE		100000

/* return codes : */
#define DHT11_OK 0
#define DHT11_TIMEOUT -1
#define DHT11_WRONG_CHCKSUM -2

/* prototypes: */
extern int dht11_read(uint8_t* p);
#endif
