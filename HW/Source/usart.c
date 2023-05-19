#include "gd32f10x.h"
#include "usart.h"

uint8_t U0_TxBuff[U0_TX_SIZE];     //串口0发送缓冲区
UCB_CB  U0CB;                      //串口控制结构体

uint8_t U2_RxBuff[U2_RX_SIZE];     //串口2接收缓冲区
uint8_t U2_TxBuff[U2_TX_SIZE];     //串口2发送缓冲区
UCB_CB  U2CB;                      //串口控制结构体

/*-------------------------------------------------*/
/*函数名：初始化串口0                              */
/*参  数：bandrate：波特率                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Usart0_Init(uint32_t bandrate)
{	
	rcu_periph_clock_enable(RCU_USART0);                                    //打开串口0时钟
	rcu_periph_clock_enable(RCU_GPIOA);                                     //打开GPIOA时钟
	
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_9);          //初始化PA9-串口0 TX引脚

	 
	usart_deinit(USART0);                                  //复位串口0
	usart_baudrate_set(USART0,bandrate);                   //设置波特率
	usart_parity_config(USART0,USART_PM_NONE);             //不使用校验位
	usart_word_length_set(USART0,USART_WL_8BIT);           //8位数据位
	usart_stop_bit_set(USART0,USART_STB_1BIT);             //1个停止位
	usart_transmit_config(USART0,USART_TRANSMIT_ENABLE);   //开启发送功能 
	
	usart_enable(USART0);                                  //开串口0
}
/*-------------------------------------------------*/
/*函数名：初始化串口2                              */
/*参  数：bandrate：波特率                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Usart2_Init(uint32_t bandrate)
{	
	rcu_periph_clock_enable(RCU_USART2);                                    //打开串口2时钟
	rcu_periph_clock_enable(RCU_GPIOB);                                     //打开GPIOB时钟
	
	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_10);         //初始化PB10-串口2 TX引脚
	gpio_init(GPIOB,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_11);   //初始化PB11-串口2 RX引脚
	 
	usart_deinit(USART2);                                  //复位串口2
	usart_baudrate_set(USART2,bandrate);                   //设置波特率
	usart_parity_config(USART2,USART_PM_NONE);             //不使用校验位
	usart_word_length_set(USART2,USART_WL_8BIT);           //8位数据位
	usart_stop_bit_set(USART2,USART_STB_1BIT);             //1个停止位
	usart_transmit_config(USART2,USART_TRANSMIT_ENABLE);   //开启发送功能
	usart_receive_config(USART2,USART_RECEIVE_ENABLE);     //开启接收功能
	usart_dma_receive_config(USART2,USART_DENR_ENABLE);    //开接收DMA
	usart_dma_transmit_config(USART2,USART_DENT_ENABLE);   //开发送DMA
	
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);      //设置中断分组：2
	nvic_irq_enable(USART2_IRQn,0,0);                      //打开串口2中断，并设置优先级
	usart_interrupt_enable(USART2,USART_INT_IDLE);         //打开串口2空闲中断  
	
	U2Rx_PtrInit();                                        //串口2控制结构体各个指针初始化
	U2Tx_PtrInit();                                        //串口2控制结构体各个指针初始化	
	DMA_Init();                                            //DMA初始化	
	usart_enable(USART2);                                  //开串口2
}
/*-------------------------------------------------*/
/*函数名：DMA初始化                                */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void DMA_Init(void)
{
	dma_parameter_struct dma_init_struct;                      //DMA初始化结构体
	
	rcu_periph_clock_enable(RCU_DMA0);	                       //打开DMA0时钟
	
	dma_deinit(DMA0,DMA_CH2);                                  //复位DMA0的通道2-串口2接收通道的	
	dma_init_struct.periph_addr = USART2+4;                    //设置外设地址，串口0的数据寄存器
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;  //外设数据宽度，8位1个字节
	dma_init_struct.memory_addr = (uint32_t)U2_RxBuff;         //设置接收缓冲区地址
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;      //缓冲区数据宽度，8位1个字节
	dma_init_struct.number = U2_RX_MAX+1;                      //设置最大接收量
	dma_init_struct.priority = DMA_PRIORITY_HIGH;              //设置优先级
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;  //外设地址增加关闭
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;   //内存地址增加打开
	dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;      //传输方向：外设寄存器到内存缓冲区
	dma_init(DMA0,DMA_CH2,&dma_init_struct);                   //初始化DMA0的通道2-串口2接收通道的
	dma_circulation_disable(DMA0,DMA_CH2);	                   //关闭循环模式
	dma_channel_enable(DMA0,DMA_CH2);                          //打开DMA0的通道2-串口2接收通道的
	
	dma_deinit(DMA0,DMA_CH1);                                  //复位DMA0的通道2-串口2发送通道的	
	dma_init_struct.periph_addr = USART2+4;                    //设置外设地址，串口0的数据寄存器
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;  //外设数据宽度，8位1个字节
	dma_init_struct.memory_addr = (uint32_t)U2_TxBuff;         //设置发送缓冲区地址
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;      //缓冲区数据宽度，8位1个字节
	dma_init_struct.number = 0;                                //发送量
	dma_init_struct.priority = DMA_PRIORITY_HIGH;              //设置优先级
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;  //外设地址增加关闭
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;   //内存地址增加打开
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;      //传输方向：内存缓冲区到外设寄存器
	dma_init(DMA0,DMA_CH1,&dma_init_struct);                   //初始化DMA0的通道2-串口2发送通道的
	dma_circulation_disable(DMA0,DMA_CH1);	                   //关闭循环模式
	dma_channel_disable(DMA0,DMA_CH1);                         //关闭DMA0的通道2-串口2发送通道的
	
	dma_interrupt_enable(DMA0,DMA_CH1,DMA_INT_FTF);            //是能完成中断
	nvic_irq_enable(DMA0_Channel1_IRQn,0,0);                   //打开中断，并设置优先级
}
/*-------------------------------------------------*/
/*函数名：串口2控制结构体各个指针初始化            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void U2Rx_PtrInit(void)
{
	U2CB.URxDataIN = &U2CB.URxDataPtr[0];           //IN指针指向SE指针对结构体数组的0号成员
	U2CB.URxDataOUT = &U2CB.URxDataPtr[0];	        //OUT指针指向SE指针对结构体数组的0号成员
	U2CB.URxDataEND = &U2CB.URxDataPtr[NUM-1];      //END指针指向SE指针对结构体数组的最后一个成员
	U2CB.URxDataIN->start = U2_RxBuff;              //使用IN指针指向的SE指针对中的S指针标记第一次接收的起始位置
	U2CB.URxCounter = 0;                            //累计接收数量清零
}
void U2Tx_PtrInit(void)
{
	U2CB.UTxDataIN = &U2CB.UTxDataPtr[0];           
	U2CB.UTxDataOUT = &U2CB.UTxDataPtr[0];	        
	U2CB.UTxDataEND = &U2CB.UTxDataPtr[NUM-1];      
	U2CB.UTxDataIN->start = U2_TxBuff;              
	U2CB.UTxCounter = 0;                            
	U2CB.UTxState = 0;
}
/*-------------------------------------------------*/
/*函数名：串口0 printf 函数                        */
/*参  数：char *format,...：输出格式以及参数       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u0_printf(char *format,...)
{	
	uint16_t i;                                           //用于for循环
	
	va_list listdata;                                     //建立一个va_list变量listdata
	va_start(listdata,format);                            //向listdata加载...代表的不定长的参数
	vsprintf((char *)U0_TxBuff,format,listdata);          //格式化输出到缓冲区U0_TxBuff
	va_end(listdata);                                     //释放listdata

	for(i=0;i<strlen((const char*)U0_TxBuff);i++){        //根据U0_TxBuff缓冲区数据量，一个字节一个字节的循环发送
		while(usart_flag_get(USART0,USART_FLAG_TBE)!=1);  //等待发送寄存器空
		usart_data_transmit(USART0,U0_TxBuff[i]);         //填入数据
	}
	while(usart_flag_get(USART0,USART_FLAG_TC)!=1);       //等到最后一个字节数据发送完毕，再退出u0_printf子函数
}
/*-------------------------------------------------*/
/*函数名：串口2 发送数据 函数                      */
/*返回值：无                                       */
/*-------------------------------------------------*/
void u2_sdata(uint8_t *data, uint16_t data_len)
{	
	if(U2_TX_SIZE - U2CB.UTxCounter >= data_len){
		U2CB.UTxDataIN->start = &U2_TxBuff[U2CB.UTxCounter];
	}else{
		U2CB.UTxCounter = 0;
		U2CB.UTxDataIN->start = U2_TxBuff;
	}
	memcpy(U2CB.UTxDataIN->start,data,data_len);
	U2CB.UTxCounter += data_len;
	U2CB.UTxDataIN->end = &U2_TxBuff[U2CB.UTxCounter-1];
	
	U2CB.UTxDataIN++;
	if(U2CB.UTxDataIN == U2CB.UTxDataEND){
		U2CB.UTxDataIN = &U2CB.UTxDataPtr[0];
	}
}
