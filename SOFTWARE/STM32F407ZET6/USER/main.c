#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "LED.h"
#include "key.h"

 int main(void)
 {
 	vu8 key=0;	
	delay_init(168);	    	 //��ʱ������ʼ��	  
 	LED_Init();			     //LED�˿ڳ�ʼ��
	KEY_Init();          //��ʼ���밴�����ӵ�Ӳ���ӿ�
	
	LED0=0;					//�ȵ������
	while(1)
	{
 		key=KEY_Scan(0);	//�õ���ֵ
	   	if(key)
		{						   
			switch(key)
			{				 
				
				case KEY2_PRES:	//����LED0��ת
					LED0=!LED0;
					break;
				case KEY1_PRES:	//����LED1��ת	 
					LED1=!LED1;
					break;
				case KEY0_PRES:	//ͬʱ����LED0,LED1��ת 
					LED0=!LED0;
					LED1=!LED1;
					break;
			}
		}else delay_ms(10); 
	}	 
}

