#ifndef		__PROTOCOL_H__
#define		__PROTOCOL_H__

#include	"bsp.h"

#define		RF_BUF_SIZE			32

uint8_t select_channel(void);
uint32_t get_uid(void);
uint8_t *get_uid_char(void);
void set_uid(uint8_t *id, uint8_t len);
void config_mode(void);

#endif		//__PROTOCOL_H__
