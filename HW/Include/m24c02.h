#ifndef M24C02_H
#define M24C02_H

#include "stdint.h"

#define M24C02_WADDR  0xA0     //24C02写操作器件地址
#define M24C02_RADDR  0xA1     //24C02读操作器件地址

uint8_t M24C02_WriteByte(uint8_t addr, uint8_t wdata);                     //函数声明
uint8_t M24C02_WritePage(uint8_t addr, uint8_t *wdata);                    //函数声明
uint8_t M24C02_ReadData(uint8_t addr, uint8_t *rdata, uint16_t datalen);   //函数声明
void M24C02_ReadOTAInfo(void);                                             //函数声明
void M24C02_WriteOTAInfo(void);                                            //函数声明

#endif



