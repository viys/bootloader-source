#include "gd32f10x.h"
#include "iic.h"
#include "m24c02.h"
#include "main.h"
#include "string.h"
#include "delay.h"

/*-------------------------------------------------*/
/*函数名：24C02写入一字节数据                      */
/*参  数：addr：地址  wdata：需要写入的数据        */
/*返回值：0：正确 其他：错误                       */
/*-------------------------------------------------*/
uint8_t M24C02_WriteByte(uint8_t addr, uint8_t wdata)
{
	IIC_Start();                           //发送起始信号
	IIC_Send_Byte(M24C02_WADDR);           //发送24C02写操作器件地址
	if(IIC_Wait_Ack(100)!=0) return 1;     //等待应答，错误的话，返回1
	IIC_Send_Byte(addr);                   //发送内部存储空间地址
	if(IIC_Wait_Ack(100)!=0) return 2;     //等待应答，错误的话，返回2
	IIC_Send_Byte(wdata);                  //发送数据
	if(IIC_Wait_Ack(100)!=0) return 3;     //等待应答，错误的话，返回3
	IIC_Stop();                            //发送停止信号
	return 0;	                           //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：24C02写入一页（16字节）数据              */
/*参  数：addr：地址  wdata：需要写入的数据指针    */
/*返回值：0：正确 其他：错误                       */
/*-------------------------------------------------*/
uint8_t M24C02_WritePage(uint8_t addr, uint8_t *wdata){
	
	uint8_t i;                                  //用于for循环
	
	IIC_Start();                                //发送起始信号
	IIC_Send_Byte(M24C02_WADDR);                //发送24C02写操作器件地址
	if(IIC_Wait_Ack(100)!=0) return 1;          //等待应答，错误的话，返回1
	IIC_Send_Byte(addr);                        //发送内部存储空间地址
	if(IIC_Wait_Ack(100)!=0) return 2;          //等待应答，错误的话，返回2
	for(i=0;i<16;i++){                          //循环16次写入一页
		IIC_Send_Byte(wdata[i]);                //发送数据
		if(IIC_Wait_Ack(100)!=0) return 3+i;    //等待应答，错误的话，返回3+i
	}
	IIC_Stop();                                 //发送停止信号
	return 0;	                                //正确，返回0
}
/*---------------------------------------------------------*/
/*函数名：24C02读取数据                                    */
/*参  数：addr：地址  rdata：接收缓冲区 datalen:读取长度   */
/*返回值：0：正确 其他：错误                               */
/*---------------------------------------------------------*/
uint8_t M24C02_ReadData(uint8_t addr, uint8_t *rdata, uint16_t datalen){
	
	uint8_t i;                                  //用于for循环
	
	IIC_Start();                                //发送起始信号
	IIC_Send_Byte(M24C02_WADDR);                //发送24C02写操作器件地址
	if(IIC_Wait_Ack(100)!=0) return 1;          //等待应答，错误的话，返回1
	IIC_Send_Byte(addr);                        //发送内部存储空间地址
	if(IIC_Wait_Ack(100)!=0) return 2;          //等待应答，错误的话，返回2
	IIC_Start();                                //再次发送起始信号
	IIC_Send_Byte(M24C02_RADDR);                //发送24C02读操作器件地址
	if(IIC_Wait_Ack(100)!=0) return 3;          //等待应答，错误的话，返回3
	for(i=0;i<datalen-1;i++){                   //循环datalen-1次，一个字节一个字节接收数据
		rdata[i] = IIC_Read_Byte(1);            //接收数据，发送应答给从机
	}
	rdata[datalen-1] = IIC_Read_Byte(0);        //接收最后一个字节，不发送应答信号
	IIC_Stop();                                 //发送停止信号
	return 0;		                            //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：读取24C02结构体到OTA_Info结构体缓冲区    */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void M24C02_ReadOTAInfo(void)
{
	memset(&OTA_Info,0,OTA_INFOCB_SIZE);                         //清空OTA_Info结构体缓冲区
	M24C02_ReadData(0,(uint8_t *)&OTA_Info,OTA_INFOCB_SIZE);     //从24C02读取数据，存放到OTA_Info结构体
}
/*-------------------------------------------------*/
/*函数名：把OTA_Info结构体缓冲区数据保存到24C02    */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void M24C02_WriteOTAInfo(void)
{
	uint8_t i;                               //用于for循环
	uint8_t *wptr;                           //uint8_t类型指针
	
	wptr = (uint8_t *)&OTA_Info;             //wptr指向OTA_Info结构体首地址
	for(i=0;i<OTA_INFOCB_SIZE/16;i++){       //每次写入一页16个字节
		M24C02_WritePage(i*16,wptr+i*16);    //写入一页数据
		Delay_Ms(5);                         //延时
	}		
}

