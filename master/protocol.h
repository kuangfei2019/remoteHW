#ifndef		__PROTOCOL_H__
#define		__PROTOCOL_H__

#include	"bsp.h"

#define		RF_BUF_SIZE			32

uint8_t select_channel(void);
void send_packet(uint8_t *pbuf, uint8_t len);
uint32_t get_uid(void);
void set_uid(uint32_t id);

#endif		//__PROTOCOL_H__
