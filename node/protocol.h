#ifndef		__PROTOCOL_H__
#define		__PROTOCOL_H__

#include	"bsp.h"

uint8_t select_channel(void);
uint32_t get_uid(void);
void set_uid(uint8_t *id, uint8_t len) ;
uint8_t *get_uid_char(void);
void update_offline_cnt(void) ;
void clr_offline_cnt(void);
uint8_t is_offline(uint8_t cnt);
void config_mode(void);

#endif		//__PROTOCOL_H__
