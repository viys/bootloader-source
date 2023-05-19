#ifndef MQTT_H
#define MQTT_H

#include "stdint.h"

#define CLIENTID    "hh6zPbXNLbr.D001|securemode=2,signmethod=hmacsha256,timestamp=1681955619281|"
#define USERNAME    "D001&hh6zPbXNLbr"
#define PASSWORD    "01c2e6596e60432c4c7b1eb87393393aafbd91b74951b7981c593f4a6710830a"

typedef struct{
	uint8_t   Pack_buff[512];
	uint16_t  MessageID;
	uint16_t  Fixed_len;
	uint16_t  Variable_len;
	uint16_t  Payload_len;
	uint16_t  Remaining_len;
	uint8_t   CMD_buff[512];
	int size;
	int streamId;
	int counter;
	int num;
	int downlen;
	uint8_t  OTA_tempver[32];
}MQTT_CB;

extern MQTT_CB  Aliyun_mqtt;

void MQTT_ConnectPack(void);
void MQTT_SubcribPack(char *topic);
void MQTT_DealPublishData(uint8_t *data, uint16_t data_len);
void MQTT_PublishDataQs0(char *topic, char *data);
void MQTT_PublishDataQs1(char *topic, char *data);
void MQTT_PublishDataQs1(char *topic, char *data);

#endif
