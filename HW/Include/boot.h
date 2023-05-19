#ifndef BOOT_H
#define BOOT_H

#include "stdint.h"

typedef void (*load_a)(void);                            //函数指针类型声明

void BootLoader_Brance(void);                            //函数声明
__asm void MSR_SP(uint32_t addr);                        //函数声明
void LOAD_A(uint32_t addr);                              //函数声明
void BootLoader_Clear(void);                             //函数声明
uint8_t BootLoader_Enter(uint8_t timeout);               //函数声明
void BootLoader_Info(void);                              //函数声明
void BootLoader_Event(uint8_t *data, uint16_t datalen);  //函数声明
uint16_t Xmodem_CRC16(uint8_t *data, uint16_t datalen);  //函数声明

#endif



