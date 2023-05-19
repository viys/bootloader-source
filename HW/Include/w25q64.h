#ifndef W25Q64_H
#define W25Q64_H

#include "stdint.h"

#define  CS_ENABLE     gpio_bit_reset(GPIOA,GPIO_PIN_4)             //打开CS，PA4低电平
#define  CS_DISENABLE  gpio_bit_set(GPIOA,GPIO_PIN_4)               //关闭CS，PA4高电平

void W25Q64_Init(void);                                             //函数声明
void W25Q64_WaitBusy(void);                                         //函数声明
void W25Q64_Enable(void);                                           //函数声明
void W25Q64_Erase64K(uint8_t blockNB);                              //函数声明
void W25Q64_PageWrite(uint8_t *wbuff, uint16_t pageNB);             //函数声明
void W25Q64_Read(uint8_t *rbuff, uint32_t addr, uint32_t datalen);  //函数声明



#endif

