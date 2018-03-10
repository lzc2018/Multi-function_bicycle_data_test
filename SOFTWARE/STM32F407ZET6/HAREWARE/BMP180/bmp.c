#include "iic.h"
#include "bmp.h"
#include "delay.h"

void BMP_Init(void)
{
    IIC_Init();   //IIC初始化
}

/*从BMP180读一个字节的数据*/

u8 BMP180_ReadOneByte(u8 ReadAddr)
{
    u8 temp = 0;
    u8 IIC_ComFlag = 1;   //IIC通信标志,为0标志正常,1表示通信错误

    IIC_Start();     //IIC start
    IIC_ComFlag = IIC_SendByte(BMP180_SlaveAddr);   //slave address+W:0
    //printf("IIC_ComFlag=%u \r\n",IIC_ComFlag);

    if (IIC_ComFlag == 0)                           //返回值为0表示通信正常，可以继续通信
    {
        IIC_SendByte(ReadAddr);                      //发送命令代码
        IIC_Start();
        IIC_SendByte(BMP180_SlaveAddr|0x01);         //slave address+R:1
        temp = IIC_ReadByte();                       //读数据
        IIC_SendACK(1);                
        IIC_Stop();

    }
    return temp;     
}

/*从BMP180读两个字节的数据*/

short BMP180_ReadTwoByte(u8 ReadAddr)
{
    u8 IIC_ComFlag = 1;   //IIC通信标志,为0标志正常,1表示通信错误
    u8 MSB,LSB;
    short temp;

     
    IIC_Start();
    IIC_ComFlag = IIC_SendByte(BMP180_SlaveAddr);
    if (IIC_ComFlag == 0)
    {
        IIC_SendByte(ReadAddr);
        IIC_Start();
        IIC_SendByte(BMP180_SlaveAddr|0x01);
        MSB = IIC_ReadByte();       //先读高位
        IIC_SendACK(0);         //ACK
        LSB = IIC_ReadByte();      //再读低位
        IIC_SendACK(1);        //NACK
        IIC_Stop();
    }
    temp = MSB*256+LSB;
    return temp;                                                   
}
 
/*从BMP180写一个字节的数据*/
void Write_OneByteToBMP180(u8 RegAdd, u8 Data)
{
    IIC_Start();                       //IIC start
    IIC_SendByte(BMP180_SlaveAddr);   //slave address+W:0
    IIC_SendByte(RegAdd);
    IIC_SendByte(Data);
    IIC_Stop();
}

/*读取BMP180校准系数*/

void Read_CalibrationData(void)
{
    AC1 = BMP180_ReadTwoByte(0xaa);
    AC2 = BMP180_ReadTwoByte(0xac);
    AC3 = BMP180_ReadTwoByte(0xae);
    AC4 = BMP180_ReadTwoByte(0xb0);
    AC5 = BMP180_ReadTwoByte(0xb2);
    AC6 = BMP180_ReadTwoByte(0xb4);
    B1 = BMP180_ReadTwoByte(0xb6);
    B2 = BMP180_ReadTwoByte(0xb8);
    MB = BMP180_ReadTwoByte(0xba);
    MC = BMP180_ReadTwoByte(0xbc);
    MD = BMP180_ReadTwoByte(0xbe);

    printf("AC1:%d \r\n",AC1);
    printf("AC2:%d \r\n",AC2);
    printf("AC3:%d \r\n",AC3);
    printf("AC4:%d \r\n",AC4);
    printf("AC5:%d \r\n",AC5);
    printf("AC6:%d \r\n",AC6);
    printf("B1:%d \r\n",B1);
    printf("B2:%d \r\n",B2);
    printf("MB:%d \r\n",MB);
    printf("MC:%d \r\n",MC);
    printf("MD:%d \r\n",MD);   
}

/*读BMP180没有经过补偿的温度值*/
long Get_BMP180UT(void)
{

    long UT;

    Write_OneByteToBMP180(0xf4,0x2e);       //write 0x2E into reg 0xf4
    delay_ms(10);                                   //wait 4.5ms
    UT = BMP180_ReadTwoByte(0xf6);          //read reg 0xF6(MSB),0xF7(LSB)
    printf("UT:%ld \r\n",UT);

    return UT;
}

/*读BMP180没有经过补偿的压力值*/

long Get_BMP180UP(void)
{

    long UP=0;
    Write_OneByteToBMP180(0xf4,0x34);       //write 0x34 into reg 0xf4
    delay_ms(10);                                    //wait 4.5ms
    UP = BMP180_ReadTwoByte(0xf6);
    UP &= 0x0000FFFF;
    printf("UP:%ld \r\n",UP);

    return UP;     
}

/*把未经过补偿的温度和压力值转换为时实际的温度和压力值
272
 *True_Temp:实际温度值
273
 *True_Press:实际压力值，单位:Pa
274
 *True_Altitude:实际海拔高度，单位:m
275
*/

void Convert_UncompensatedToTrue(long UT,long UP)
{

    long X1,X2,X3,B3,B5,B6,B7,T,P;
    unsigned long B4;

    X1 = ((UT-AC6)*AC5)>>15;      //printf("X1:%ld \r\n",X1);
    X2 = ((long)MC<<11)/(X1+MD);  //printf("X2:%ld \r\n",X2);
    B5 = X1+X2;                        //printf("B5:%ld \r\n",B5);
    T = (B5+8)>>4;                      //printf("T:%ld \r\n",T);
    True_Temp = T/10.0;            
	printf("Temperature:%.1f \r\n",True_Temp);
    B6 = B5-4000;                       //printf("B6:%ld \r\n",B6);
    X1 = (B2*B6*B6)>>23;              //printf("X1:%ld \r\n",X1);
    X2 = (AC2*B6)>>11;                //printf("X2:%ld \r\n",X2);
    X3 = X1+X2;                         //printf("X3:%ld \r\n",X3);
    B3 = (((long)AC1*4+X3)+2)/4;    //printf("B3:%ld \r\n",B3);
    X1 = (AC3*B6)>>13;                //printf("X1:%ld \r\n",X1);
    X2 = (B1*(B6*B6>>12))>>16;      //printf("X2:%ld \r\n",X2);
    X3 = ((X1+X2)+2)>>2;              //printf("X3:%ld \r\n",X3);
    B4 = AC4*(unsigned long)(X3+32768)>>15;   //printf("B4:%lu \r\n",B4);
    B7 = ((unsigned long)UP-B3)*50000;        //printf("B7:%lu \r\n",B7);
    if (B7 < 0x80000000)
    {
        P = (B7*2)/B4; 
    }
    else P=(B7/B4)*2;                   //printf("P:%ld \r\n",P);          
    X1 = (P/256.0)*(P/256.0);       //printf("X1:%ld \r\n",X1);
    X1 = (X1*3038)>>16;               //printf("X1:%ld \r\n",X1);
    X2 = (-7357*P)>>16;               //printf("X2:%ld \r\n",X2);
    P = P+((X1+X2+3791)>>4);      //printf("P:%ld \r\n",P);
    True_Press = P;                 
	printf("Press:%.1fPa \r\n",True_Press);
    True_Altitude = 44330*(1-pow((P/101325.0),(1.0/5.255)));           
    printf("Altitude:%.3fm \r\n",True_Altitude);   
}

