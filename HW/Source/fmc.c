#include "gd32f10x.h"
#include "fmc.h"


/*-------------------------------------------------*/
/*函数名：擦除FLASH                                */
/*参  数：start：擦除起始扇区   num：擦几个扇区    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void GD32_EraseFlash(uint16_t start, uint16_t num)
{
	uint16_t i;        //用于for循环

	fmc_unlock();                                                       //解锁
	for(i=0;i<num;i++){                                                 //循环num次，一个扇区一个扇区的擦除
		fmc_page_erase((0x08000000 + start * 1024) + (1024 * i));       //擦除一个扇区
	}
	fmc_lock();                                                         //上锁
} 
/*---------------------------------------------------------------------*/
/*函数名：写入FLASH                                                    */
/*参  数：saddr：写入地址 wdata：写入数据指针  wnum：写入多少个字节    */
/*返回值：无                                                           */
/*---------------------------------------------------------------------*/
void GD32_WriteFlash(uint32_t saddr, uint32_t *wdata, uint32_t wnum)
{
	fmc_unlock();                          //解锁
	while(wnum){                           //循环写入，知道num等于0退出while，表示写入完毕
		fmc_word_program(saddr,*wdata);    //写入一个字（4个字节）
		wnum-=4;                           //写入字节数量-4
		saddr+=4;                          //写入地址+4
		wdata++;                           //写入数据的指针，往后挪动一次
	}
	fmc_lock();                            //上锁
}

