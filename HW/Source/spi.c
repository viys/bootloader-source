#include "gd32f10x.h"
#include "spi.h"

/*-------------------------------------------------*/
/*函数名：PSI0初始化                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI0_Init(void){
	
	spi_parameter_struct spi_parameter;                                             //初始化SPI的结构体
	
	rcu_periph_clock_enable(RCU_SPI0);                                              //打开SPI0时钟
	rcu_periph_clock_enable(RCU_GPIOA);                                             //打开GPIOA时钟
	
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5|GPIO_PIN_7);       //配置PA5和PA7
	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_6);            //配置PA6
	
	spi_i2s_deinit(SPI0);                                           //复位SPI0
	spi_parameter.device_mode = SPI_MASTER;                         //主机模式
	spi_parameter.trans_mode = SPI_TRANSMODE_FULLDUPLEX;            //双线双向全双工
	spi_parameter.frame_size = SPI_FRAMESIZE_8BIT;                  //数据宽度8位一个字节
	spi_parameter.nss = SPI_NSS_SOFT;                               //软件控制NSS
	spi_parameter.endian = SPI_ENDIAN_MSB;                          //从最高位开始传输
	spi_parameter.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;    //模式0
	spi_parameter.prescale = SPI_PSC_2;                             //2分频
	spi_init(SPI0,&spi_parameter);                                  //初始化SPI0
	spi_enable(SPI0);	                                            //打开SPI0
}  
/*-------------------------------------------------*/
/*函数名：SPI收发一个字节                          */
/*参  数：txd：要发送的数据                        */
/*返回值：接收到的数据                             */
/*-------------------------------------------------*/
uint8_t SPI0_ReadWriteByte(uint8_t txd)
{
	while(spi_i2s_flag_get(SPI0,SPI_FLAG_TBE)!=1);  //等到发送缓冲区空
	spi_i2s_data_transmit(SPI0,txd);                //填入数据
	while(spi_i2s_flag_get(SPI0,SPI_FLAG_RBNE)!=1); //等待接收缓冲区有数据
	return spi_i2s_data_receive(SPI0);              //返回接收到的数据
}
/*-------------------------------------------------*/
/*函数名：SPI发数据                                */
/*参  数：wdata：发送数据指针  datalen：发送长度   */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI0_Write(uint8_t *wdata, uint16_t datalen)
{
	uint16_t i;                         //用于for循环
	
	for(i=0;i<datalen;i++){             //循环datalen次，一个字节一个字节发送
		SPI0_ReadWriteByte(wdata[i]);   //发送一个字节数据
	}
}
/*-------------------------------------------------*/
/*函数名：SPI接收数据                              */
/*参  数：rdata：接收缓冲区  datalen：接收长度     */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI0_Read(uint8_t *rdata, uint16_t datalen)
{
	uint16_t i;                                   //用于for循环
	
	for(i=0;i<datalen;i++){                       //循环datalen次，一个字节一个字节接收
		rdata[i] = SPI0_ReadWriteByte(0xff);      //接收保存一个字节数据
	}
}


