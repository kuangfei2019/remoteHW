#include	"protocol.h"

const char channel[16] = {85, 87, 90, 93, 96, 99, 102, 105,\
													 107, 110, 113, 115, 117, 120, 123, 126};

static uint8_t offline_cnt;

uint8_t select_channel(void) {
uint8_t i=0;

	while(1) {
		iwdg_refresh();
		set_rf_channel(channel[i%16]);
//		printf("freq=%d\r\n", 2400+rf_read_reg(0x05));
		delay(500);
		if(is_rf_received()) {
			break;
		}
		i++;
	}
	
	return (i>=16) ? 0 : channel[i]; 
}

uint8_t read_packet(uint8_t *buf, uint8_t max_len) {
uint8_t remain=0;
uint8_t len=0;
uint8_t *pbuf;

	len = get_rf_cnt();
	pbuf = get_rf_buf();
	
	if(len <= 1) {
		return 0;
	}
	
	len -= 1;
	
	if(pbuf[0] == 0x11) {
		memcpy(buf, pbuf+1, len);
	} else if(pbuf[0] == 0x21) {
		memcpy(buf, pbuf+1, len);
		
		for(uint8_t i=0; i<25; i++) {
			delay(2);
			
			if(is_rf_received()) {
				remain = get_rf_cnt();
				pbuf = get_rf_buf();
				
				if(remain >= 1) {
					remain -= 1;
				} else {
					return 0;
				}
				if(pbuf[0] == 0x22) {
					memcpy(buf+len, pbuf+1, remain);
				}
			} else {
				return 0;
			}			
		}
	}
	
	len += remain;
	
	return len;
}

uint32_t get_uid(void) {
	return *(uint32_t *)DEV_ID_LOC;
}

void set_uid(uint32_t id) {
	if(!(FLASH_IAPSR & 0x80)) {
		FLASH_DUKR = 0xAE;
		FLASH_DUKR = 0x56;
	}
	*(uint32_t *)DEV_ID_LOC = id;
	FLASH_DUKR = 0xDE;
	FLASH_DUKR = 0xAD;
}

void update_offline_cnt(void) {
	offline_cnt++;
}

void clr_offline_cnt(void) {
	offline_cnt = 0;
}

uint8_t is_offline(uint8_t cnt) {
	if(offline_cnt >= cnt) {
		offline_cnt = 0;
		return 1;
	}
	return 0;
}

void config_mode(void) {
uint32_t ticks;
uint8_t loop=0;

	ticks = get_systick();
	
	while(1) {
	uint8_t cnt, *pbuf;
		
		iwdg_refresh();
		if(is_uart_received()) {
			pbuf = get_uart_buf();
			cnt = get_uart_cnt();			
			if(memcmp("RXADD\r\n", pbuf, cnt) == 0) {
				loop = 1;
				printf("RX|C|%08ld|\r\n", get_uid());
			} else if(memcmp("WR|C|", pbuf, 5) == 0) {
				uint32_t addr;
				loop = 1;
				addr = atol((const char *)(pbuf+5));
				if(addr != 0) {
					set_uid(addr);
					printf("WR|C|%08ld|\r\n", get_uid());
				} else {
					printf("ERROR\r\n");
				}
			} else if(memcmp("RUN\r\n", pbuf, cnt) == 0) {
				printf("RUN\r\n");
				return;
			} else {
				printf("ERROR\r\n");
			}
		}
		
		if(!loop && (get_systick()-ticks>4)) {
			return;
		}		
	}
}

