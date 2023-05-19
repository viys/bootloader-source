#include "gd32f10x.h"
#include "iic.h"
#include "delay.h"

/*-------------------------------------------------*/
/*函数名：IIC初始化                                */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void IIC_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);	                                //打开GPIOB时钟
	gpio_init(GPIOB,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_6);     //设置PB6，OD输出，模拟SCL
	gpio_init(GPIOB,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_7);     //设置PB7，OD输出，模拟SDA
	IIC_SCL_H;
	IIC_SDA_H;
}
/*-------------------------------------------------*/
/*函数名：IIC起始信号                              */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void IIC_Start(void)
{
	IIC_SCL_H;        //拉高SCL
	IIC_SDA_H;        //拉高SDA
	Delay_Us(2);      //延时
	IIC_SDA_L;        //拉低SDA
	Delay_Us(2);      //延时
	IIC_SCL_L;        //拉低SCL
}
/*-------------------------------------------------*/
/*函数名：IIC停止信号                              */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void IIC_Stop(void)
{
	IIC_SCL_H;        //拉高SCL
	IIC_SDA_L;        //拉低SDA
	Delay_Us(2);      //延时
	IIC_SDA_H;        //拉高SDA
}
/*-------------------------------------------------*/
/*函数名：IIC发送一个字节数据                      */
/*参  数：txd：要发送的数据                        */
/*返回值：无                                       */
/*-------------------------------------------------*/
void IIC_Send_Byte(uint8_t txd)
{
	int8_t i;                  //用于for循环
	
	for(i=7;i>=0;i--){         //一个字节循环8次 从BIT7~BIT0
		IIC_SCL_L;             //拉低SCL
		if(txd&BIT(i))         //判断当前需要发送的二进制，是1的话进入if分支
			IIC_SDA_H;         //拉高SDA
		else                   //判断当前需要发送的二进制，是0的话进入else分支
			IIC_SDA_L;         //拉低SDA
		Delay_Us(2);           //延时
		IIC_SCL_H;             //拉高SCL
		Delay_Us(2);           //延时
	}
	IIC_SCL_L;                 //拉低SCL
	IIC_SDA_H;                 //拉高SDA，等待从机应答
}
/*-------------------------------------------------*/
/*函数名：IIC等待应答                              */
/*参  数：timeout：超时时间                        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
uint8_t IIC_Wait_Ack(int16_t timeout)
{
	do{                                   //循环等待
		timeout--;                        //超时时间减1
		Delay_Us(2);                      //延时
	}while((READ_SDA)&&(timeout>=0));     //2个条件退出while循环，SDA变低从机应答 / 超时时间小于0
	if(timeout<0) return 1;               //如果超时时间小于0，说明没有等到从机应答，返回1，表示错误
	IIC_SCL_H;                            //等到从机应答，拉高SCL
	Delay_Us(2);                          //延时
	if(READ_SDA!=0) return 2;             //再次判断，如果不是稳定的低电平，返回2，表示错误
	IIC_SCL_L;                            //拉低SCL
	Delay_Us(2);                          //延时
	return 0;                             //正确，返回0
} 
/*-------------------------------------------------*/
/*函数名：IIC接收一个字节数据                      */
/*参  数：ack：1 应答从机  0 不应答从机            */
/*返回值：接收的数据                               */
/*-------------------------------------------------*/
uint8_t IIC_Read_Byte(uint8_t ack)
{
	int8_t i;              //用于for循环
	uint8_t rxd;           //用于保存接收的数据
	
	rxd = 0;               //清零rxd
	for(i=7;i>=0;i--){     //一个字节循环8次 从BIT7~BIT0
		IIC_SCL_L;         //拉低SCL
		Delay_Us(2);       //延时
		IIC_SCL_H;         //拉高SCL
		if(READ_SDA)       //判断当前SDA线状态，如果是高电平，进入if
			rxd |= BIT(i); //相应的二进位记录高电平1
		Delay_Us(2);       //延时
	}
	IIC_SCL_L;             //拉低SCL
	Delay_Us(2);           //延时
	if(ack){               //如果ack是1，进入if
		IIC_SDA_L;         //拉低SDA
		IIC_SCL_H;         //拉高SCL
		Delay_Us(2);       //延时
		IIC_SCL_L;         //拉低SCL
		IIC_SDA_H;         //拉高SDA
		Delay_Us(2);       //延时
	}else{                 //如果ack是0，进入else
		IIC_SDA_H;         //拉高SDA
		IIC_SCL_H;         //拉高SCL
		Delay_Us(2);       //延时
		IIC_SCL_L;         //拉低SCL
		Delay_Us(2);       //延时
	}
	return rxd;            //返回接收的数据rxd
}


