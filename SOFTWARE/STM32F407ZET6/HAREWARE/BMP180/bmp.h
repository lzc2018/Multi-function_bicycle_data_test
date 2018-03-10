#ifndef __BMP_H
#define __BMP_H
#include "iic.c"

//SCL -> PC1
//SDA -> PC2

#define SCL_H()  GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define SCL_L()  GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define SDA_H()  GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define SDA_L()  GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define SDA  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)

#define BMP180_SlaveAddr 0xee   //BMP180的器件地址

//BMP180校准系数

short AC1;
short AC2;
short AC3;
unsigned short AC4;
unsigned short AC5;
unsigned short AC6;
short B1;
short B2;
short MB;
short MC;
short MD;

 
u8 BMP180_ID=0;          //BMP180的ID
float True_Temp=0;       //实际温度
float True_Press=0;      //实际气压,单位:Pa
float True_Altitude=0;   //实际海拔,单位:m

#define EE_TYPE BMP180

u8  BMP180_Check(void);  //检查器件
void  BMP180_Init(void); //初始化IIC
u8 BMP180_ReadOneByte(u8 ReadAddr);
short BMP180_ReadTwoByte(u8 ReadAddr);
void Write_OneByteToBMP180(u8 RegAdd, u8 Data);
void Read_CalibrationData(void);
long Get_BMP180UT(void);
long Get_BMP180UP(void);
void Convert_UncompensatedToTrue(long UT,long UP);




#endif


