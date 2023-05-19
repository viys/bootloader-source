#ifndef SPI_H
#define SPI_H

#include "stdint.h"
 
void SPI0_Init(void);                                 //函数声明
uint8_t SPI0_ReadWriteByte(uint8_t txd);              //函数声明
void SPI0_Write(uint8_t *wdata, uint16_t datalen);    //函数声明
void SPI0_Read(uint8_t *rdata, uint16_t datalen);     //函数声明

#endif


