#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "adc.h"
#include "rtc.h"
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "key.h" 
#include "usart3.h" 
#include "string.h"	 	 
#include "gps.h"	 


u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//����1,���ͻ�����
nmea_msg gpsx; 											//GPS��Ϣ
__align(4) u8 dtbuf[50];   								//��ӡ������
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode�ַ��� 

//ͨ�����ڴ�ӡSD�������Ϣ
void show_sdcard_info(void)
{
	switch(SDCardInfo.CardType)
	{
		case SDIO_STD_CAPACITY_SD_CARD_V1_1:printf("Card Type:SDSC V1.1\r\n");break;
		case SDIO_STD_CAPACITY_SD_CARD_V2_0:printf("Card Type:SDSC V2.0\r\n");break;
		case SDIO_HIGH_CAPACITY_SD_CARD:printf("Card Type:SDHC V2.0\r\n");break;
		case SDIO_MULTIMEDIA_CARD:printf("Card Type:MMC Card\r\n");break;
	}	
  	printf("Card ManufacturerID:%d\r\n",SDCardInfo.SD_cid.ManufacturerID);	//������ID
 	printf("Card RCA:%d\r\n",SDCardInfo.RCA);								//����Ե�ַ
	printf("Card Capacity:%d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));	//��ʾ����
 	printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.CardBlockSize);			//��ʾ���С
}


//��ʾGPS��λ��Ϣ 
void Gps_Msg_Show(void)
{
 	float tp;		   
	POINT_COLOR=BLUE;  	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//�õ������ַ���
 	LCD_ShowString(30,230,200,16,16,dtbuf);	 	   
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//�õ�γ���ַ���
 	LCD_ShowString(30,250,200,16,16,dtbuf);	 	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//�õ��߶��ַ���
 	LCD_ShowString(30,270,200,16,16,dtbuf);	 			   
	tp=gpsx.speed;	   
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//�õ��ٶ��ַ���	 
 	LCD_ShowString(30,290,200,16,16,dtbuf);	 				    
	if(gpsx.fixmode<=3)														//��λ״̬
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
	  	LCD_ShowString(30,310,200,16,16,dtbuf);			   
	}	 	   
	sprintf((char *)dtbuf,"Valid satellite:%02d",gpsx.posslnum);	 		//���ڶ�λ��������
 	LCD_ShowString(30,330,200,16,16,dtbuf);	    
	sprintf((char *)dtbuf,"Visible satellite:%02d",gpsx.svnum%100);	 		//�ɼ�������
 	LCD_ShowString(30,350,200,16,16,dtbuf);		 
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//��ʾUTC����
	//printf("year2:%d\r\n",gpsx.utc.year);
	LCD_ShowString(30,370,200,16,16,dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾUTCʱ��
  	LCD_ShowString(30,390,200,16,16,dtbuf);		  
}	 





int main(void)
{ 

	
 	u8 x=0;
	short temp; 
	
	 
	u32 sd_size;
	u8 *buf;
	u16 i,rxlen;
	u16 lenx;
	u8 key=0XFF;
	u8 upload=0;
	
	u8 lcd_id[12];				//���LCD ID�ַ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);      //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200	
	usart3_init(38400);			//��ʼ������3������Ϊ38400
//	usmart_dev.init(84); 		//��ʼ��USMART	
	LED_Init();					  //��ʼ��LED
 	LCD_Init();           //��ʼ��LCD FSMC�ӿ�
	Adc_Init();  	//�ڲ��¶ȴ�����ADC��ʼ��
	KEY_Init();					//������ʼ��  
//	usmart_dev.init(72); 		//��ʼ��USMART 
	
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ��
	
	POINT_COLOR=RED;      //������ɫ����ɫ
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣				 	
  	while(1) 
	{		 
		switch(x)
		{
			case 0:LCD_Clear(WHITE);break;
			case 1:LCD_Clear(BLACK);break;
			case 2:LCD_Clear(BLUE);break;
			case 3:LCD_Clear(RED);break;
			case 4:LCD_Clear(MAGENTA);break;
			case 5:LCD_Clear(GREEN);break;
			case 6:LCD_Clear(CYAN);break; 
			case 7:LCD_Clear(YELLOW);break;
			case 8:LCD_Clear(BRRED);break;
			case 9:LCD_Clear(GRAY);break;
			case 10:LCD_Clear(LGRAY);break;
			case 11:LCD_Clear(BROWN);break;
		}
		POINT_COLOR=RED;	  
		LCD_ShowString(30,20,210,24,24,"LZC STM32F4");	
		LCD_ShowString(30,50,200,16,16,"TFTLCD TEST");
		
 		LCD_ShowString(30,70,200,16,16,lcd_id);		//��ʾLCD ID	      					 
	  LCD_ShowString(30,90,200,16,16,"KEY1:Upload NMEA Data SW");
		LCD_ShowString(30,110,200,16,16,"NMEA Data Upload:OFF");  		
		LCD_ShowString(30,150,200,16,16,"KEY0:Read Sector 0");
		while(SD_Init())//��ⲻ��SD��
	{
		LCD_ShowString(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		LCD_ShowString(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
	show_sdcard_info();	//��ӡSD�������Ϣ
 	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	//���SD���ɹ� 											    
	LCD_ShowString(30,170,200,16,16,"SD Card OK    ");
	LCD_ShowString(30,190,200,16,16,"SD Card Size:     MB");
	LCD_ShowNum(30+13*8,190,SDCardInfo.CardCapacity>>20,5,16);//��ʾSD������
		
	  x++;
		if(x==12)x=0;
		
		POINT_COLOR=BLUE;//��������Ϊ��ɫ      
	LCD_ShowString(30,130,200,16,16,"TEMPERATE: 00.00C");//���ڹ̶�λ����ʾС����	

	if(Ublox_Cfg_Rate(1000,1)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ1000ms,˳���ж�GPSģ���Ƿ���λ. 
	{
   		LCD_ShowString(30,230,200,16,16,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//�����ж�,ֱ�����Լ�鵽NEO-6M,�����ݱ���ɹ�
		{
			usart3_init(9600);				//��ʼ������3������Ϊ9600(EEPROMû�б������ݵ�ʱ��,������Ϊ9600.)
	  		Ublox_Cfg_Prt(38400);			//��������ģ��Ĳ�����Ϊ38400
			usart3_init(38400);				//��ʼ������3������Ϊ38400
			Ublox_Cfg_Tp(1000000,100000,1);	//����PPSΪ1�������1��,������Ϊ100ms	    
			key=Ublox_Cfg_Cfg_Save();		//��������  
		}	  					 
	   	LCD_ShowString(30,230,200,16,16,"NEO-6M Set Done!!");
		delay_ms(500);
		LCD_Fill(30,230,30+200,120+16,WHITE);//�����ʾ 
	}
	
	while(1)
		{
			
			
			temp=Get_Temprate();	//�õ��¶�ֵ 
			if(temp<0)
			{
				temp=-temp;
				LCD_ShowString(30+10*8,130,16,16,16,"-");	    //��ʾ����
			}else LCD_ShowString(30+10*8,130,16,16,16," ");	//�޷���
		
			LCD_ShowxNum(30+11*8,130,temp/100,2,16,0);		//��ʾ��������
			LCD_ShowxNum(30+14*8,130,temp%100,2,16,0);		//��ʾС������ 
			
			
			
		key=KEY_Scan(0);
		if(key==KEY0_PRES)//KEY0������
		{
			buf=mymalloc(0,512);		//�����ڴ�
			if(SD_ReadDisk(buf,0,1)==0)	//��ȡ0����������
			{	
				LCD_ShowString(30,230,200,16,16,"USART1 Sending Data...");
				printf("SECTOR 0 DATA:\r\n");
				for(sd_size=0;sd_size<512;sd_size++)printf("%x ",buf[sd_size]);//��ӡ0��������    	   
				printf("\r\nDATA ENDED\r\n");
				LCD_ShowString(30,230,200,16,16,"USART1 Send Data Over!");
			}
			myfree(0,buf);//�ͷ��ڴ�	   
		}   
			
			
		delay_ms(1);
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{
			rxlen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//������һ�ν���
			USART1_TX_BUF[i]=0;			//�Զ���ӽ�����
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ���
			Gps_Msg_Show();				//��ʾ��Ϣ	
			if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//���ͽ��յ������ݵ�����1
 		}
		key=KEY_Scan(0);
		if(key==KEY1_PRES)
		{
			upload=!upload;
			POINT_COLOR=RED;
			if(upload)LCD_ShowString(30,150,200,16,16,"NMEA Data Upload:ON ");
			else LCD_ShowString(30,150,200,16,16,"NMEA Data Upload:OFF");
 		}
		if((lenx%500)==0)LED0=!LED0; 	    				 
		lenx++;	
			
			LED0=!LED0;	 
			delay_ms(1000);	
		} 
	}
}

