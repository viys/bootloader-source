#include "gd32f10x.h"
#include "main.h"
#include "4g.h"
#include "delay.h"
#include "usart.h"
#include "w25q64.h"
#include "m24c02.h"
#include "mqtt.h"

/*-------------------------------------------------*/
/*函数名：复位4G模块                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void GM4G_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);	                                //打开GPIOB时钟
	gpio_init(GPIOB,GPIO_MODE_IPD,GPIO_OSPEED_50MHZ,GPIO_PIN_0);        //设置PB0，监控连接A的状态
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_2);     //设置PB2，控制4G模块的复位
	
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_12);     //设置PB12，控制LED
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13);     //设置PB13，控制LED
	gpio_bit_set(GPIOB,GPIO_PIN_12);
	gpio_bit_set(GPIOB,GPIO_PIN_13);
	
	//同步复位4G模块
	u0_printf("复位4G模块，请等待... ...");
	gpio_bit_set(GPIOB,GPIO_PIN_2);
	Delay_Ms(500);
	gpio_bit_reset(GPIOB,GPIO_PIN_2);
}
/*-------------------------------------------------*/
/*函数名：处理串口2的数据                          */
/*参  数：data：数据指针      datalen：数据长度    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void U2_Event(uint8_t *data, uint16_t datalen)
{	
	if((datalen == 6)&&(memcmp(data,"chaozi",6) == 0)){
		u0_printf("\r\n4G模块复位成功,等待连接服务器成功... ...\r\n");	
		rcu_periph_clock_enable(RCU_AF);	                                
		exti_deinit();
		exti_init(EXTI_0,EXTI_INTERRUPT,EXTI_TRIG_BOTH);
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB,GPIO_PIN_SOURCE_0);
		exti_interrupt_enable(EXTI_0);
		nvic_irq_enable(EXTI0_IRQn,0,0);                      	
	}
	
	if((datalen == 4)&&(data[0]==0x20)){
		u0_printf("收到CONNACK报文\r\n");	
		if(data[3]==0x00){
			u0_printf("CONNECT报文成功连接服务器\r\n");
			BootStaFlag |= CONNECT_OK;
			MQTT_SubcribPack("/sys/hh6zPbXNLbr/D001/thing/service/property/set");
			MQTT_SubcribPack("/sys/hh6zPbXNLbr/D001/thing/file/download_reply");
			OTA_Version();
		}else{
			u0_printf("CONNECT报文错误，准备重启\r\n");
			NVIC_SystemReset();
		}                   	
	}
	
	if((datalen == 5)&&(data[0]==0x90)){
		u0_printf("收到SUBACK报文\r\n");	
		if((data[datalen-1]==0x00)||(data[datalen-1]==0x01)){
			u0_printf("SUBCRIBE订阅报文成功\r\n");
		}else{
			u0_printf("SUBCRIBE订阅报文错误，准备重启\r\n");
			NVIC_SystemReset();
		}                   	
	}
	
	if((BootStaFlag&CONNECT_OK)&&(data[0]==0x30)){
		u0_printf("收到等级0的PUBLISH报文\r\n");	
		MQTT_DealPublishData(data,datalen); 
		u0_printf("%s\r\n",Aliyun_mqtt.CMD_buff);
		if(strstr((char *)Aliyun_mqtt.CMD_buff,"{\"Switch1\":0}")){
			u0_printf("关闭开关\r\n");
			MQTT_PublishDataQs0("/sys/hh6zPbXNLbr/D001/thing/event/property/post","{\"params\":{\"Switch1\":0}}");
		}
		if(strstr((char *)Aliyun_mqtt.CMD_buff,"{\"Switch1\":1}")){
			u0_printf("打开开关\r\n");
			MQTT_PublishDataQs0("/sys/hh6zPbXNLbr/D001/thing/event/property/post","{\"params\":{\"Switch1\":1}}");
		}
		if(strstr((char *)Aliyun_mqtt.CMD_buff,"/ota/device/upgrade/hh6zPbXNLbr/D001")){
			if(sscanf((char *)Aliyun_mqtt.CMD_buff,"/ota/device/upgrade/hh6zPbXNLbr/D001{\"code\":\"1000\",\"data\":{\"size\":%d,\"streamId\":%d,\"sign\":\"%*32s\",\"dProtocol\":\"mqtt\",\"version\":\"%26s\",\"signMethod\":\"Md5\",\"streamFileId\":1,\"md5\":\"%*32s\"},\"id\":%*d,\"message\":\"success\"}",&Aliyun_mqtt.size,&Aliyun_mqtt.streamId,Aliyun_mqtt.OTA_tempver)==3){
				u0_printf("OTA固件大小：%d\r\n",Aliyun_mqtt.size);
				u0_printf("OTA固件ID：%d\r\n",Aliyun_mqtt.streamId);
				u0_printf("OTA固件版本号：%s\r\n",Aliyun_mqtt.OTA_tempver);
				BootStaFlag |= OTA_EVENT;
				W25Q64_Erase64K(0);
				if(Aliyun_mqtt.size%256==0){
					Aliyun_mqtt.counter = Aliyun_mqtt.size/256;
				}else{
					Aliyun_mqtt.counter = Aliyun_mqtt.size/256 + 1;
				}
				Aliyun_mqtt.num = 1;
				Aliyun_mqtt.downlen = 256;
				OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num - 1)*256);
			}else{
				u0_printf("提取OTA下载命令错误\r\n");
			}
		}
		if(strstr((char *)Aliyun_mqtt.CMD_buff,"/sys/hh6zPbXNLbr/D001/thing/file/download_reply")){
			W25Q64_PageWrite(&data[datalen-Aliyun_mqtt.downlen-2],Aliyun_mqtt.num-1);
			Aliyun_mqtt.num++;
			if(Aliyun_mqtt.num<Aliyun_mqtt.counter){
				Aliyun_mqtt.downlen = 256;
				OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num - 1)*256);
			}else if(Aliyun_mqtt.num==Aliyun_mqtt.counter){
				if(Aliyun_mqtt.size%256==0){
					Aliyun_mqtt.downlen = 256;
					OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num - 1)*256);
				}else{
					Aliyun_mqtt.downlen = Aliyun_mqtt.size%256;
					OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num - 1)*256);	
				}
			}else{
				u0_printf("OTA下载完毕\r\n");
				memset(OTA_Info.OTA_ver,0,32);
				memcpy(OTA_Info.OTA_ver,Aliyun_mqtt.OTA_tempver,26);
				OTA_Info.Firelen[0] = Aliyun_mqtt.size;
				OTA_Info.OTA_flag = OTA_SET_FLAG;
				M24C02_WriteOTAInfo();
				NVIC_SystemReset();
			}
		}
	}
}
void OTA_Version(void)
{
	char temp[128];
	
	memset(temp,0,128);
	sprintf(temp,"{\"id\": \"1\",\"params\": {\"version\": \"%s\"}}",OTA_Info.OTA_ver);
	
	MQTT_PublishDataQs1("/ota/device/inform/hh6zPbXNLbr/D001",temp);
}
void OTA_Download(int size, int offset)
{
	char temp[256];
	
	memset(temp,0,256);
	sprintf(temp,"{\"id\": \"1\",\"params\": {\"fileInfo\":{\"streamId\":%d,\"fileId\":1},\"fileBlock\":{\"size\":%d,\"offset\":%d}}}",Aliyun_mqtt.streamId,size,offset);
	u0_printf("当前第%d/%d次\r\n",Aliyun_mqtt.num,Aliyun_mqtt.counter);
	MQTT_PublishDataQs0("/sys/hh6zPbXNLbr/D001/thing/file/download",temp);
	Delay_Ms(300);
}
