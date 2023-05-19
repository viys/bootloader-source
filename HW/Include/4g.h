#ifndef GM4G_H
#define GM4G_H

void GM4G_Init(void);                             //Ί―ΚύΙωΓχ
void U2_Event(uint8_t *data, uint16_t datalen);   //Ί―ΚύΙωΓχ
void OTA_Version(void);
void OTA_Download(int size, int offset);

#endif
