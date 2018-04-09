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


u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//串口1,发送缓存区
nmea_msg gpsx; 											//GPS信息
__align(4) u8 dtbuf[50];   								//打印缓存器
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode字符串 

//通过串口打印SD卡相关信息
void show_sdcard_info(void)
{
	switch(SDCardInfo.CardType)
	{
		case SDIO_STD_CAPACITY_SD_CARD_V1_1:printf("Card Type:SDSC V1.1\r\n");break;
		case SDIO_STD_CAPACITY_SD_CARD_V2_0:printf("Card Type:SDSC V2.0\r\n");break;
		case SDIO_HIGH_CAPACITY_SD_CARD:printf("Card Type:SDHC V2.0\r\n");break;
		case SDIO_MULTIMEDIA_CARD:printf("Card Type:MMC Card\r\n");break;
	}	
  	printf("Card ManufacturerID:%d\r\n",SDCardInfo.SD_cid.ManufacturerID);	//制造商ID
 	printf("Card RCA:%d\r\n",SDCardInfo.RCA);								//卡相对地址
	printf("Card Capacity:%d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));	//显示容量
 	printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.CardBlockSize);			//显示块大小
}


//显示GPS定位信息 
void Gps_Msg_Show(void)
{
 	float tp;		   
	POINT_COLOR=BLUE;  	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//得到经度字符串
 	LCD_ShowString(30,230,200,16,16,dtbuf);	 	   
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//得到纬度字符串
 	LCD_ShowString(30,250,200,16,16,dtbuf);	 	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//得到高度字符串
 	LCD_ShowString(30,270,200,16,16,dtbuf);	 			   
	tp=gpsx.speed;	   
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//得到速度字符串	 
 	LCD_ShowString(30,290,200,16,16,dtbuf);	 				    
	if(gpsx.fixmode<=3)														//定位状态
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
	  	LCD_ShowString(30,310,200,16,16,dtbuf);			   
	}	 	   
	sprintf((char *)dtbuf,"Valid satellite:%02d",gpsx.posslnum);	 		//用于定位的卫星数
 	LCD_ShowString(30,330,200,16,16,dtbuf);	    
	sprintf((char *)dtbuf,"Visible satellite:%02d",gpsx.svnum%100);	 		//可见卫星数
 	LCD_ShowString(30,350,200,16,16,dtbuf);		 
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//显示UTC日期
	//printf("year2:%d\r\n",gpsx.utc.year);
	LCD_ShowString(30,370,200,16,16,dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC时间
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
	
	u8 lcd_id[12];				//存放LCD ID字符串
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200	
	usart3_init(38400);			//初始化串口3波特率为38400
//	usmart_dev.init(84); 		//初始化USMART	
	LED_Init();					  //初始化LED
 	LCD_Init();           //初始化LCD FSMC接口
	Adc_Init();  	//内部温度传感器ADC初始化
	KEY_Init();					//按键初始化  
//	usmart_dev.init(72); 		//初始化USMART 
	
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池
	
	POINT_COLOR=RED;      //画笔颜色：红色
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。				 	
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
		
 		LCD_ShowString(30,70,200,16,16,lcd_id);		//显示LCD ID	      					 
	  LCD_ShowString(30,90,200,16,16,"KEY1:Upload NMEA Data SW");
		LCD_ShowString(30,110,200,16,16,"NMEA Data Upload:OFF");  		
		LCD_ShowString(30,150,200,16,16,"KEY0:Read Sector 0");
		while(SD_Init())//检测不到SD卡
	{
		LCD_ShowString(30,150,200,16,16,"SD Card Error!");
		delay_ms(500);					
		LCD_ShowString(30,150,200,16,16,"Please Check! ");
		delay_ms(500);
		LED0=!LED0;//DS0闪烁
	}
	show_sdcard_info();	//打印SD卡相关信息
 	POINT_COLOR=BLUE;	//设置字体为蓝色 
	//检测SD卡成功 											    
	LCD_ShowString(30,170,200,16,16,"SD Card OK    ");
	LCD_ShowString(30,190,200,16,16,"SD Card Size:     MB");
	LCD_ShowNum(30+13*8,190,SDCardInfo.CardCapacity>>20,5,16);//显示SD卡容量
		
	  x++;
		if(x==12)x=0;
		
		POINT_COLOR=BLUE;//设置字体为蓝色      
	LCD_ShowString(30,130,200,16,16,"TEMPERATE: 00.00C");//先在固定位置显示小数点	

	if(Ublox_Cfg_Rate(1000,1)!=0)	//设置定位信息更新速度为1000ms,顺便判断GPS模块是否在位. 
	{
   		LCD_ShowString(30,230,200,16,16,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//持续判断,直到可以检查到NEO-6M,且数据保存成功
		{
			usart3_init(9600);				//初始化串口3波特率为9600(EEPROM没有保存数据的时候,波特率为9600.)
	  		Ublox_Cfg_Prt(38400);			//重新设置模块的波特率为38400
			usart3_init(38400);				//初始化串口3波特率为38400
			Ublox_Cfg_Tp(1000000,100000,1);	//设置PPS为1秒钟输出1次,脉冲宽度为100ms	    
			key=Ublox_Cfg_Cfg_Save();		//保存配置  
		}	  					 
	   	LCD_ShowString(30,230,200,16,16,"NEO-6M Set Done!!");
		delay_ms(500);
		LCD_Fill(30,230,30+200,120+16,WHITE);//清除显示 
	}
	
	while(1)
		{
			
			
			temp=Get_Temprate();	//得到温度值 
			if(temp<0)
			{
				temp=-temp;
				LCD_ShowString(30+10*8,130,16,16,16,"-");	    //显示负号
			}else LCD_ShowString(30+10*8,130,16,16,16," ");	//无符号
		
			LCD_ShowxNum(30+11*8,130,temp/100,2,16,0);		//显示整数部分
			LCD_ShowxNum(30+14*8,130,temp%100,2,16,0);		//显示小数部分 
			
			
			
		key=KEY_Scan(0);
		if(key==KEY0_PRES)//KEY0按下了
		{
			buf=mymalloc(0,512);		//申请内存
			if(SD_ReadDisk(buf,0,1)==0)	//读取0扇区的内容
			{	
				LCD_ShowString(30,230,200,16,16,"USART1 Sending Data...");
				printf("SECTOR 0 DATA:\r\n");
				for(sd_size=0;sd_size<512;sd_size++)printf("%x ",buf[sd_size]);//打印0扇区数据    	   
				printf("\r\nDATA ENDED\r\n");
				LCD_ShowString(30,230,200,16,16,"USART1 Send Data Over!");
			}
			myfree(0,buf);//释放内存	   
		}   
			
			
		delay_ms(1);
		if(USART3_RX_STA&0X8000)		//接收到一次数据了
		{
			rxlen=USART3_RX_STA&0X7FFF;	//得到数据长度
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//启动下一次接收
			USART1_TX_BUF[i]=0;			//自动添加结束符
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//分析字符串
			Gps_Msg_Show();				//显示信息	
			if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//发送接收到的数据到串口1
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

