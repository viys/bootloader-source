#include "gd32f10x.h"
#include "main.h"
#include "usart.h"
#include "delay.h"
#include "fmc.h"
#include "iic.h"
#include "m24c02.h"
#include "w25q64.h"
#include "4g.h"
#include "mqtt.h"

MQTT_CB  Aliyun_mqtt;

void MQTT_ConnectPack(void)
{
	Aliyun_mqtt.MessageID = 1;
	Aliyun_mqtt.Fixed_len = 1;
	Aliyun_mqtt.Variable_len = 10;
	Aliyun_mqtt.Payload_len = 2 + strlen(CLIENTID) + 2 + strlen(USERNAME) + 2 + strlen(PASSWORD);
	Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;
	
	Aliyun_mqtt.Pack_buff[0] = 0x10;
	do{
		if(Aliyun_mqtt.Remaining_len/128 == 0){
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len;
		}else{
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len%128)|0x80;
		}
		Aliyun_mqtt.Fixed_len++;
		Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len/128;
	}while(Aliyun_mqtt.Remaining_len);
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+0] = 0x00;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+1] = 0x04;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+2] = 0x4D;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+3] = 0x51;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+4] = 0x54;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+5] = 0x54;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+6] = 0x04;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+7] = 0xC2;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+8] = 0x00;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+9] = 0x64;
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+10] = strlen(CLIENTID)/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+11] = strlen(CLIENTID)%256;
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+12],CLIENTID,strlen(CLIENTID));
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+12+strlen(CLIENTID)] = strlen(USERNAME)/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+13+strlen(CLIENTID)] = strlen(USERNAME)%256;
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+14+strlen(CLIENTID)],USERNAME,strlen(USERNAME));
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+14+strlen(CLIENTID)+strlen(USERNAME)] = strlen(PASSWORD)/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+15+strlen(CLIENTID)+strlen(USERNAME)] = strlen(PASSWORD)%256;
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+16+strlen(CLIENTID)+strlen(USERNAME)],PASSWORD,strlen(PASSWORD));
	
	u2_sdata(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);
}
void MQTT_SubcribPack(char *topic)
{
	Aliyun_mqtt.Fixed_len = 1;
	Aliyun_mqtt.Variable_len = 2;
	Aliyun_mqtt.Payload_len = 2 + strlen(topic) + 1;
	Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;
	
	Aliyun_mqtt.Pack_buff[0] = 0x82;
	do{
		if(Aliyun_mqtt.Remaining_len/128 == 0){
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len;
		}else{
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len%128)|0x80;
		}
		Aliyun_mqtt.Fixed_len++;
		Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len/128;
	}while(Aliyun_mqtt.Remaining_len);	
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+0] = Aliyun_mqtt.MessageID/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+1] = Aliyun_mqtt.MessageID%256;
	Aliyun_mqtt.MessageID++;

	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+2] = strlen(topic)/256;	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+3] = strlen(topic)%256;	
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+4],topic,strlen(topic));
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+4+strlen(topic)] = 0;

	u2_sdata(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);
}
void MQTT_DealPublishData(uint8_t *data, uint16_t data_len)
{
	uint8_t i;
	
	for(i=1;i<5;i++){
		if((data[i]&0x80)==0)
			break;
	}
	
	memset(Aliyun_mqtt.CMD_buff,0,512);
	memcpy(Aliyun_mqtt.CMD_buff,&data[1+i+2],data_len-1-i-2);
}
void MQTT_PublishDataQs0(char *topic, char *data)
{
	Aliyun_mqtt.Fixed_len = 1;
	Aliyun_mqtt.Variable_len = 2 + strlen(topic);
	Aliyun_mqtt.Payload_len = strlen(data);
	Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;
	
	Aliyun_mqtt.Pack_buff[0] = 0x30;
	do{
		if(Aliyun_mqtt.Remaining_len/128 == 0){
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len;
		}else{
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len%128)|0x80;
		}
		Aliyun_mqtt.Fixed_len++;
		Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len/128;
	}while(Aliyun_mqtt.Remaining_len);	
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+0] = strlen(topic)/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+1] = strlen(topic)%256;
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+2],topic,strlen(topic));
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+2+strlen(topic)],data,strlen(data));
	
	u2_sdata(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);	
}
void MQTT_PublishDataQs1(char *topic, char *data)
{
	Aliyun_mqtt.Fixed_len = 1;
	Aliyun_mqtt.Variable_len = 2 + 2 + strlen(topic);
	Aliyun_mqtt.Payload_len = strlen(data);
	Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;
	
	Aliyun_mqtt.Pack_buff[0] = 0x32;
	do{
		if(Aliyun_mqtt.Remaining_len/128 == 0){
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len;
		}else{
			Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len%128)|0x80;
		}
		Aliyun_mqtt.Fixed_len++;
		Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len/128;
	}while(Aliyun_mqtt.Remaining_len);	
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+0] = strlen(topic)/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+1] = strlen(topic)%256;
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+2],topic,strlen(topic));
	
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+2+strlen(topic)] = Aliyun_mqtt.MessageID/256;
	Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+3+strlen(topic)] = Aliyun_mqtt.MessageID%256;
	Aliyun_mqtt.MessageID++;
	
	memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len+4+strlen(topic)],data,strlen(data));
	
	u2_sdata(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);	
}
