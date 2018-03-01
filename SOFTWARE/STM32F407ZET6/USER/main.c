#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "LED.h"

int main(void)
{
	LED_Init();
	delay_init(168);
	
	while(1)
	{
		GPIO_ResetBits(GPIOF,GPIO_Pin_9);
		GPIO_SetBits(GPIOF,GPIO_Pin_10);
		delay_ms(500);
		
		GPIO_ResetBits(GPIOF,GPIO_Pin_10);
		GPIO_SetBits(GPIOF,GPIO_Pin_9);
		delay_ms(500);
	}
	
}
