#include "gd32f10x.h"
#include "delay.h"

/*-------------------------------------------------*/
/*函数名：延时初始化                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Delay_Init(void)
{
	systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);    //设置Systick时钟
}
/*-------------------------------------------------*/
/*函数名：延时微妙                                 */
/*参  数：us：延时多少微妙                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Delay_Us(uint16_t us)
{
	SysTick->LOAD = us*108;                                //设置重载值
	SysTick->VAL = 0x00;                                   //清除当前计数器值
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;              //开Systick
	while(!(SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk));    //等到Systick从LOAD重载值倒数到0，标志位置位，表示延时时间到
	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;             //关Systick
} 
/*-------------------------------------------------*/
/*函数名：延时毫秒                                 */
/*参  数：us：延时多少毫秒                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Delay_Ms(uint16_t ms)
{
	while(ms--){             //循环
		Delay_Us(1000);      //每次延时1000us
	}
}


