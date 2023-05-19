#include "gd32f10x.h"
#include "boot.h"
#include "main.h"
#include "usart.h"
#include "delay.h"
#include "fmc.h"
#include "iic.h"
#include "m24c02.h"
#include "w25q64.h"

load_a load_A;        //函数指针load_A

/*-------------------------------------------------*/
/*函数名：BootLoader分支判断                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void BootLoader_Brance(void)
{
	if(BootLoader_Enter(20)==0){	
		if(OTA_Info.OTA_flag == OTA_SET_FLAG){       //判断OTA_flag是不是OTA_SET_FLAG定义的值，是的话进入if
			u0_printf("OTA更新\r\n");                //串口0输出信息
			BootStaFlag |= UPDATA_A_FLAG;            //置位标志位，表明需要更新A区
			UpDataA.W25Q64_BlockNB = 0;              //W25Q64_BlockNB等于0，表明是OTA要更新A区		
		}else{                                       //判断OTA_flag是不是OTA_SET_FLAG定义的值，不是的话进入else
			u0_printf("跳转A分区\r\n");              //串口0输出信息
			LOAD_A(GD32_A_SADDR);                    //跳转到A区
		}
	}
	u0_printf("进入BootLoader命令行\r\n");           //串口0输出信息
	
}
/*-------------------------------------------------*/
/*函数名：判断是否进入BootLoader命令行             */
/*参  数：timeout ：超时时间                       */
/*返回值：1：进入  0：不进入                       */
/*-------------------------------------------------*/
uint8_t BootLoader_Enter(uint8_t timeout)
{
	u0_printf("%dms内，输入小写字母 w ,进入BootLoader命令行\r\n",timeout*100);   //串口0输出信息
	while(timeout--){                                                            //等待超时到0
		Delay_Ms(100);                                                           //延时200ms
		if(U0_RxBuff[0] == 'w'){                                                 //串口收到的第一个字符必须是小写w，才进入if
			return 1;                                                            //返回1 进入BootLoader命令行 
		}
	}
	return 0;                                                                    //返回0 不进入BootLoader命令行 
}
/*-------------------------------------------------*/
/*函数名：串口输出命令行信息                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void BootLoader_Info(void)
{ 
	u0_printf("\r\n");                           //串口0输出信息
	u0_printf("[1]擦除A区\r\n");                 //串口0输出信息
	u0_printf("[2]串口IAP下载A区程序\r\n");      //串口0输出信息
	u0_printf("[3]设置OTA版本号\r\n");           //串口0输出信息
	u0_printf("[4]查询OTA版本号\r\n");           //串口0输出信息
	u0_printf("[5]向外部Flash下载程序\r\n");     //串口0输出信息
	u0_printf("[6]使用外部Flash内程序\r\n");     //串口0输出信息
	u0_printf("[7]重启\r\n");
	u0_printf("[8]设置服务器连接信息\r\n");
}
/*-------------------------------------------------*/
/*函数名：BootLoader处理串口数据                   */
/*参  数：data：数据指针      datalen：数据长度    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void BootLoader_Event(uint8_t *data, uint16_t datalen)
{
	int temp,i;                                                                            //temp用于版本号sscanf判断格式    i用于for循环
	
	/*--------------------------------------------------*/
	/*          没有任何事件，判断顶层命令              */
	/*--------------------------------------------------*/
	if(BootStaFlag == 0){                                                                  //如果BootStaFlag等于0，没有任何事件，进入if，判断是哪个命令
		if((datalen==1)&&(data[0]=='1')){                                                  //如果数据长度1字节 且 是字符 1
			u0_printf("擦除A区\r\n");                                                      //串口0输出信息                                                   
			GD32_EraseFlash(GD32_A_START_PAGE,GD32_A_PAGE_NUM);                            //擦除A分区占用的扇区
		}
		else if((datalen==1)&&(data[0]=='2')){                                             //如果数据长度1字节 且 是字符 2
			u0_printf("通过Xmodem协议，串口IAP下载A区程序，请使用bin格式文件\r\n");        //串口0输出信息 
			GD32_EraseFlash(GD32_A_START_PAGE,GD32_A_PAGE_NUM);	                           //擦除A分区占用的扇区  
			BootStaFlag |= (IAP_XMODEMC_FLAG|IAP_XMODEMD_FLAG);                            //置位 IAP_XMODEMC_FLAG 和 IAP_XMODEMD_FLAG 标志位
			UpDataA.XmodemTimer = 0;                                                       //Xmodem发送大写C 间隔时间变量清零
			UpDataA.XmodemNB= 0;                                                           //保持接收Xmodem协议数据包个数的变量清零
		}
		else if((datalen==1)&&(data[0]=='3')){                                             //如果数据长度1字节 且 是字符 3
			u0_printf("设置版本号\r\n");                                                   //串口0输出信息
			BootStaFlag |= SET_VERSION_FLAG;                                               //置位 SET_VERSION_FLAG 
		}
		else if((datalen==1)&&(data[0]=='4')){                                             //如果数据长度1字节 且 是字符 4
			u0_printf("查询版本号\r\n");                                                   //串口0输出信息
			M24C02_ReadOTAInfo();                                                          //从24C02读取保持的数据
			u0_printf("版本号:%s\r\n",OTA_Info.OTA_ver);                                   //串口0输出信息
			BootLoader_Info();                                                             //串口输出命令行信息      
		}
		else if((datalen==1)&&(data[0]=='5')){                                             //如果数据长度1字节 且 是字符 5
			u0_printf("向外部Flash下载程序，输入需要使用的块编号（1~9）\r\n");             //串口0输出信息
			BootStaFlag |= CMD_5_FLAG;                                                     //置位 CMD_5_FLAG      
		}
		else if((datalen==1)&&(data[0]=='6')){                                             //如果数据长度1字节 且 是字符 6
			u0_printf("使用外部Flash内的程序，输入需要使用的块编号（1~9）\r\n");           //串口0输出信息
			BootStaFlag |= CMD_6_FLAG;                                                     //置位 CMD_6_FLAG 
		}
		else if((datalen==1)&&(data[0]=='7')){                                             //如果数据长度1字节 且 是字符 7
			u0_printf("重启\r\n");                                                         //串口0输出信息
			Delay_Ms(100);                                                                 //延时
			NVIC_SystemReset(); 	                                                       //重启		
		}
		else if((datalen==1)&&(data[0]=='8')){                                             //如果数据长度1字节 且 是字符 8
			u0_printf("请设置服务器连接信息\r\n");                                         //串口0输出信息
			BootStaFlag |= CMD_8_FLAG;                                                     //置位 CMD_8_FLAG 		
		}
	}
	/*--------------------------------------------------*/
	/*          发生Xmodem事件，处理该事件              */
	/*--------------------------------------------------*/
	else if(BootStaFlag&IAP_XMODEMD_FLAG){                                                 //如果 IAP_XMODEMD_FLAG 置位表示开始Xmodem协议接收数据
		if((datalen==133)&&(data[0]==0x01)){                                               //判断 Xmodem协议一包总长133字节 且 第一个字节帧头是0x01
			BootStaFlag &=~ IAP_XMODEMC_FLAG;                                              //已经收到数据包了，所以清除 IAP_XMODEMC_FLAG，不再发送大写C
			UpDataA.XmodemCRC = Xmodem_CRC16(&data[3],128);                                //计算本次接收的数据包数据的CRC
			if(UpDataA.XmodemCRC == data[131]*256 + data[132]){                            //计算的CRC 和 接收的CRC 比较，一样说明正确，进入if
				UpDataA.XmodemNB++;                                                        //已接收的数据包数量+1
				memcpy(&UpDataA.Updatabuff[((UpDataA.XmodemNB-1)%(GD32_PAGE_SIZE/128))*128],&data[3],128);   //将本次接收的数据，暂存到UpDataA.Updatabuff缓冲区
				if((UpDataA.XmodemNB%(GD32_PAGE_SIZE/128))==0){                            //对C8T6而言，如果已接收数据包数量是8的整数倍，说明都满1扇区1024字节，进入if
					if(BootStaFlag&CMD5_XMODEM_FLAG){                                      //判断如果是命令5启动Xmodem的话，进入if
						for(i=0;i<4;i++){                                                  //W25Q64每次写入256字节，对C8T6而言，1扇区1024字节，需要循环4次写
							W25Q64_PageWrite(&UpDataA.Updatabuff[i*256], (UpDataA.XmodemNB/8 - 1) * 4 + i + UpDataA.W25Q64_BlockNB * 64 *4);    //将接受的数据写入W25Q64
						}					
					}else{                                                                 //判断如果不是命令5启动Xmodem的话，那就是串口IAP启动的，进入else
						GD32_WriteFlash(GD32_A_SADDR + ((UpDataA.XmodemNB/(GD32_PAGE_SIZE/128))-1)*GD32_PAGE_SIZE,(uint32_t *)UpDataA.Updatabuff,GD32_PAGE_SIZE);    //写入到单片机A区相应的扇区
					}
				}
				u0_printf("\x06");    //正确，返回ACK给CRT软件
			}else{                    //如果CRC校验错误，进入else
				u0_printf("\x15");    //返回NCK给CRT软件
			}
		}
		if((datalen==1)&&(data[0]==0x04)){                              //如果收到1个字节数据 且 是0x04，进入if，说明收到EOT，表明数据已经发生完毕
			u0_printf("\x06");                                          //返回ACK给CRT软件
			if((UpDataA.XmodemNB%(GD32_PAGE_SIZE/128))!=0){             //对C8T6而言，判断是否还有不满1扇区1024字节的数据，如果有进入if，把剩余的小尾巴写入
				if(BootStaFlag&CMD5_XMODEM_FLAG){                       //判断如果是命令5启动Xmodem的话，进入if
					for(i=0;i<4;i++){                                   //W25Q64每次写入256字节，对C8T6而言，1扇区1024字节，需要循环4次写
						W25Q64_PageWrite(&UpDataA.Updatabuff[i*256], (UpDataA.XmodemNB/8) * 4 + i + UpDataA.W25Q64_BlockNB * 64 *4); //将接受的数据写入W25Q64
					}
				}else{                                                  //判断如果不是命令5启动Xmodem的话，那就是串口IAP启动的，进入else
					GD32_WriteFlash(GD32_A_SADDR + ((UpDataA.XmodemNB/(GD32_PAGE_SIZE/128)))*GD32_PAGE_SIZE,(uint32_t *)UpDataA.Updatabuff,(UpDataA.XmodemNB%(GD32_PAGE_SIZE/128))*128);    //写入到单片机A区相应的扇区
				}
			}
			BootStaFlag &=~ IAP_XMODEMD_FLAG;                           //Xmodem接收完毕，清除标志位
			if(BootStaFlag&CMD5_XMODEM_FLAG){                           //判断如果是命令5启动Xmodem的话，进入if
				BootStaFlag &=~ CMD5_XMODEM_FLAG;                       //清除CMD5_XMODEM_FLAG
				OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = UpDataA.XmodemNB * 128;   //计算并保存本次传输的程序大小
				M24C02_WriteOTAInfo();                                  //保存到24C02
				Delay_Ms(100);                                          //延时
				BootLoader_Info();                                      //输出命令行信息
			}else{                                                      //判断如果不是命令5启动Xmodem的话，那就是串口IAP启动的，进入else
				Delay_Ms(100);                                          //延时
				NVIC_SystemReset();                                     //重启
			}
		}
	}
	/*--------------------------------------------------*/
	/*         发生设置版本号事件，处理改事件           */
	/*--------------------------------------------------*/
	else if(BootStaFlag&SET_VERSION_FLAG){                              //进入分支，处理设置版本号事件
		if(datalen==26){                                                //判断版本号长度是不是26，正确进入if
			if(sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)==8){  //判断版本号格式，正确进入if
				memset(OTA_Info.OTA_ver,0,32);                          //清除OTA_Info.OTA_ver缓冲区
				memcpy(OTA_Info.OTA_ver,data,26);                       //将串口发送过来的版本号拷贝到OTA_Info.OTA_ver缓冲区       
				M24C02_WriteOTAInfo();                                  //写入24C02
				u0_printf("版本正确\r\n");                              //串口0输出信息
				BootStaFlag &=~ SET_VERSION_FLAG;                       //清除标志位
				BootLoader_Info();                                      //输出命令行信息
			}else u0_printf("版本号格式错误\r\n");                      //判断版本号格式错误，串口0提示
		}else u0_printf("版本号长度错误\r\n");                          //判断版本号长度错误，串口0提示
	}
	/*--------------------------------------------------*/
	/*            发生命令5事件，处理改事件             */
	/*--------------------------------------------------*/
	else if(BootStaFlag&CMD_5_FLAG){                                    //进入分支，处理命令5，将程序文件写入W25Q64
		if(datalen==1){                                                 //数据长度是1正确，进入if，表示W25Q64的块标号
			if((data[0]>=0x31)&&(data[0]<=0x39)){                       //判断W25Q64的块标号，范围1~9，正确进入if
				UpDataA.W25Q64_BlockNB = data[0] - 0x30;                //将块标号由字符1~9编号，转换成数字1~9
				BootStaFlag |= (IAP_XMODEMC_FLAG|IAP_XMODEMD_FLAG|CMD5_XMODEM_FLAG); //置位3个标志位
				UpDataA.XmodemTimer = 0;                                             //Xmodem发送大写C 间隔时间变量清零
				UpDataA.XmodemNB= 0;                                                 //保持接收Xmodem协议数据包个数的变量清零
				OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = 0;                        //W25Q64的块标号对应的程序大小变量清零
				W25Q64_Erase64K(UpDataA.W25Q64_BlockNB);                             //擦除相应的W25Q64块
				u0_printf("通过Xmodem协议，向外部Flash第%d个块下载程序，请使用bin格式文件\r\n",UpDataA.W25Q64_BlockNB); //串口0输出信息
				BootStaFlag &=~ CMD_5_FLAG;                                          //清除标志位
			}else u0_printf("编号错误\r\n");                            //判断W25Q64的块标号，范围1~9，错误进入else，串口0输出信息
		}else u0_printf("数据长度错误\r\n");                            //判断数据长度，不是1的话错误进入else，串口0输出信息
	}
	/*--------------------------------------------------*/
	/*            发生命令6事件，处理改事件             */
	/*--------------------------------------------------*/
	else if(BootStaFlag&CMD_6_FLAG){                     //进入分支，处理命令6，使用W25Q64内的程序
		if(datalen==1){                                  //数据长度是1正确，进入if，表示W25Q64的块标号    
			if((data[0]>=0x31)&&(data[0]<=0x39)){        //判断W25Q64的块标号，范围1~9，正确进入if
				UpDataA.W25Q64_BlockNB = data[0] - 0x30; //将块标号由字符1~9编号，转换成数字1~9
				BootStaFlag |= UPDATA_A_FLAG;            //置位标志位，表明需要更新A区
				BootStaFlag &=~ CMD_6_FLAG;              //清除标志位
			}else u0_printf("编号错误\r\n");             //判断W25Q64的块标号，范围1~9，错误进入else，串口0输出信息
		}else u0_printf("数据长度错误\r\n");             //判断数据长度，不是1的话错误进入else，串口0输出信息
	}
	/*--------------------------------------------------*/
	/*            发生命令8事件，处理改事件             */
	/*--------------------------------------------------*/
	else if(BootStaFlag&CMD_8_FLAG){                     //进入分支，处理命令6，使用W25Q64内的程序
		u2_printf("AT+SOCKA=%s\r\n",data);
		Delay_Ms(30);
		u2_printf("AT+SOCKAEN=ON\r\n");
		Delay_Ms(30);
		u2_printf("AT+SOCKBEN=OFF\r\n");
		Delay_Ms(30);
		u2_printf("AT+SOCKCEN=OFF\r\n");
		Delay_Ms(30);
		u2_printf("AT+SOCKDEN=OFF\r\n");
		Delay_Ms(30);
		u2_printf("AT+HEART=ON,NET,USER,60,C000\r\n");
		Delay_Ms(30);
		u2_printf("AT+S\r\n");
		Delay_Ms(30);
		BootStaFlag &=~ CMD_8_FLAG;                      //清除标志位
	}
}
/*-------------------------------------------------*/
/*函数名：设置SP                                   */
/*参  数：addr：栈顶指针初始值                     */
/*返回值：无                                       */
/*-------------------------------------------------*/
__asm void MSR_SP(uint32_t addr)
{
	MSR MSP, r0        //addr的值加载到了r0通用寄存器，然后通过MSR指令，将通用寄存器r0的值写入到MSP主堆栈指针
	BX r14             //返回调用MSR_SP函数的主函数
}
/*-------------------------------------------------*/
/*函数名：跳转到A区                                */
/*参  数：addr：A区的起始地址                      */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LOAD_A(uint32_t addr)
{
	if((*(uint32_t *)addr>=0x20000000)&&(*(uint32_t *)addr<=0x20004FFF)){     //判断sp栈顶指针的范围是否合法，在对应型号的RAM控件范围内
		MSR_SP(*(uint32_t *)addr);                                            //设置SP
		load_A = (load_a)*(uint32_t *)(addr+4);                               //将函数指针load_A指向A区的复位向量
		BootLoader_Clear();                                                   //清除B区使用的外设
		load_A();                                                             //调用函数指针load_A，改变PC指针，从而转向A区的复位向量位置，完成跳转
	}else u0_printf("跳转A分区失败\r\n"); 
}
/*-------------------------------------------------*/
/*函数名：清除B区使用的外设                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void BootLoader_Clear(void)
{
	usart_deinit(USART0);   //复位串口0
	gpio_deinit(GPIOA);     //复位GPIOA
	gpio_deinit(GPIOB);     //复位GPIOB
}
/*-------------------------------------------------*/
/*函数名：Xmodem_CRC16校验                         */
/*参  数：data：数据指针  datalen：数据长度        */
/*返回值：校验后的数据                             */
/*-------------------------------------------------*/
uint16_t Xmodem_CRC16(uint8_t *data, uint16_t datalen){
	
	uint8_t i;                                               //用于for循环
	uint16_t Crcinit = 0x0000;                               //Xmdoem CRC校验的初始值，必须是0x0000
	uint16_t Crcipoly = 0x1021;                              //Xmdoem CRC校验的多项式，必须是0x1021
	
	while(datalen--){                                        //根据datalen大小，有多少字节循环多少次
		Crcinit = (*data << 8) ^ Crcinit;                    //先将带校验的字节，挪到高8位
		for(i=0;i<8;i++){                                    //每个字节8个二进制位，循环8次
			if(Crcinit&0x8000)                               //判断BIT15是1还是0,是1的话，进入if
				Crcinit = (Crcinit << 1) ^ Crcipoly;         //是1的话，先左移，再异或多项式
			else                                             //判断BIT15是1还是0,是0的话，进入else
				Crcinit = (Crcinit << 1);                    //是0的话，只左移
		}
		data++;                                              //下移，计算一个字节数据
	}
	return Crcinit;                                          //返回校验后的数据
}















