#define Motor_GPIO GPIOD
#define Motor_RCC RCC_AHB1Periph_GPIOD
#define Motor_Pin GPIO_Pin_12
void Motor_On(void);
void Motor_Off(void);


#define LED_GPIO GPIOB
#define LED_RCC RCC_AHB1Periph_GPIOB
#define LED_Pin GPIO_Pin_0
void LED_On(void);
void LED_Off(void);

