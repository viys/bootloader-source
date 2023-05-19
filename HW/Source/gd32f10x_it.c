/*!
    \file    gd32f10x_it.c
    \brief   interrupt service routines

    \version 2014-12-26, V1.0.0, firmware for GD32F10x
    \version 2017-06-20, V2.0.0, firmware for GD32F10x
    \version 2018-07-31, V2.1.0, firmware for GD32F10x
    \version 2020-09-30, V2.2.0, firmware for GD32F10x
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/
#include "gd32f10x_it.h"
#include "usart.h"
#include "mqtt.h"

/*-------------------------------------------------*/
/*函数名：串口2中断服务函数                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void USART2_IRQHandler(void)
{
	if(usart_interrupt_flag_get(USART2,USART_INT_FLAG_IDLE) != 0){                       //如果USART_INT_FLAG_IDLE置位，表示空闲中断发生，进入if
		usart_flag_get(USART2,USART_FLAG_IDLEF);                                         //清除空闲中断标志位步骤1：读取状态标志寄存器
		usart_data_receive(USART2);                                                      //清除空闲中断标志位步骤2：读取数据寄存器
		U2CB.URxCounter += (U2_RX_MAX+1) - dma_transfer_number_get(DMA0,DMA_CH2);        //+=操作，将本次的接收数量，累计到URxCounter变量
		U2CB.URxDataIN->end = &U2_RxBuff[U2CB.URxCounter-1];                             //IN指针指向的结构体中的e指针记录本次接收的结束位置
		U2CB.URxDataIN++;                                                                //IN指针后移
		if(U2CB.URxDataIN == U2CB.URxDataEND){                                           //如何后移到了END标记的位置，进入if
			U2CB.URxDataIN = &U2CB.URxDataPtr[0];                                        //回卷，重新指向0号成员
		}
		if(U2_RX_SIZE - U2CB.URxCounter >= U2_RX_MAX){                                   //判断，如果剩余的空间大于等于单次接收最大量，进入if
			U2CB.URxDataIN->start = &U2_RxBuff[U2CB.URxCounter];                         //IN指针指向的结构体中的s指针记录下次接收的起始位置
		}else{                                                                           //判断，如果剩余的空间小于单次接收最大量，进入else
			U2CB.URxDataIN->start = U2_RxBuff;                                           //剩余的空间不用了，回卷，下次的接收位置返回缓冲区起始位置
			U2CB.URxCounter = 0;                                                         //累计值也清零
		}
		dma_channel_disable(DMA0,DMA_CH2);                                               //关闭通道4
		dma_transfer_number_config(DMA0,DMA_CH2,U2_RX_MAX+1);                            //重新设置接收量
		dma_memory_address_config(DMA0,DMA_CH2,(uint32_t)U2CB.URxDataIN->start);         //重新设置接收位置
		dma_channel_enable(DMA0,DMA_CH2);                                                //开启通道4
	}
}
/*-------------------------------------------------*/
/*函数名：外部中断0服务函数                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void EXTI0_IRQHandler(void)
{
	if(exti_interrupt_flag_get(EXTI_0) != 0){
		exti_interrupt_flag_clear(EXTI_0);
		if(gpio_input_bit_get(GPIOB,GPIO_PIN_0) == 1){
			u0_printf("\r\n连接服务器成功\r\n");
			MQTT_ConnectPack();			
		}else{
			u0_printf("\r\n连接服务器断开\r\n");
		}
	}
}

void DMA0_Channel1_IRQHandler(void)
{
	if(dma_interrupt_flag_get(DMA0,DMA_CH1,DMA_INT_FLAG_FTF) != 0){
		dma_interrupt_flag_clear(DMA0,DMA_CH1,DMA_INT_FLAG_FTF);
		dma_channel_disable(DMA0,DMA_CH1);                       	
		u0_printf("数据发送完成\r\n");
		U2CB.UTxState = 0;
	}
}






















/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{

}
