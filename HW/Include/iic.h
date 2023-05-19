#ifndef IIC_H
#define IIC_H

#define IIC_SCL_H   gpio_bit_set(GPIOB,GPIO_PIN_6)           //拉高SCL，PB6高电平
#define IIC_SCL_L   gpio_bit_reset(GPIOB,GPIO_PIN_6)         //拉低SCL，PB6低电平      

#define IIC_SDA_H   gpio_bit_set(GPIOB,GPIO_PIN_7)           //拉高SDA，PB7高电平
#define IIC_SDA_L   gpio_bit_reset(GPIOB,GPIO_PIN_7)         //拉低SDA，PB7低电平

#define READ_SDA    gpio_input_bit_get(GPIOB,GPIO_PIN_7)     //读取SDA状态，读取PB7电平

void IIC_Init(void);                      //函数声明
void IIC_Start(void);                     //函数声明
void IIC_Stop(void);                      //函数声明
void IIC_Send_Byte(uint8_t txd);          //函数声明
uint8_t IIC_Wait_Ack(int16_t timeout);    //函数声明
uint8_t IIC_Read_Byte(uint8_t ack);       //函数声明


#endif
