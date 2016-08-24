#include	"protocol.h"

const char channel[16] = {85, 87, 90, 93, 96, 99, 102, 105,\
													 107, 110, 113, 115, 117, 120, 123, 126};

uint8_t select_channel(void) {
uint32_t ticks;	
uint8_t ch, ccr, start;

	ccr = 0;
	ch = 0;
	start = get_uid()%sizeof(channel);
	
	while(1) {
		iwdg_refresh();
		set_rf_channel(channel[(start+ch)%16]);
//		printf("freq=%d\r\n", 2400+rf_read_reg(0x05));
		ticks = get_systick();
		while(1) {
			if(rf_read_reg(0x09) == 1) {
				ccr = 1;
				break;
			}
			if(get_systick()-ticks>1) {
				ccr = 0;
				break;
			}
		}
		if(ccr == 0) {
			break;
		}
		
		ch++;
	}
	
	while(1) {
		iwdg_refresh();
		led_on();
		rf_write_payload("X", 1);
		delay(50);
		led_off();
		delay(150);
		if(is_rf_sent()) {
			rf_write_reg(0x05, rf_read_reg(0x05));
			break;
		}
		putchar(0xF0);
	}	
	
	return rf_read_reg(0x05);
}

void send_packet(uint8_t *pbuf, uint8_t len) {
uint8_t buf[RF_BUF_SIZE];
	
	if(len < RF_BUF_SIZE) {
		buf[0] = 0x11;
		memcpy(buf+1, pbuf, len);
		rf_write_payload(buf, len+1);
	} else {
		buf[0] = 0x21;
		memcpy(buf+1, pbuf, RF_BUF_SIZE-1);
		rf_write_payload(buf, RF_BUF_SIZE);
		
		for(uint8_t i=0; i<20; i++) {
			delay(2);
			if(is_rf_sent()) {
				break;
			}
		}
		
		len -= (RF_BUF_SIZE-1);
		buf[0] = 0x22;
		memcpy(buf+1, pbuf+RF_BUF_SIZE-1, len);
		rf_write_payload(buf, len+1);
	}
	
	for(uint8_t i=0; i<20; i++) {
		delay(2);
		if(is_rf_sent()) {
			break;
		}
	}	
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

void config_mode(void) {
uint32_t ticks;
uint8_t loop=0;

	ticks = get_systick();
	
	while(1) {
	uint8_t cnt, *pbuf;
		
		iwdg_refresh();
		if(is_uart_received()) {
			loop = 1;
			pbuf = get_uart_buf();
			cnt = get_uart_cnt();			
			if(memcmp("RXADD\r\n", pbuf, cnt) == 0) {
				loop = 1;
				printf("RX|Z|%08ld|\r\n", get_uid());
			} else if(memcmp("WR|Z|", pbuf, 5) == 0) {
				uint32_t addr;
				loop = 1;
				addr = atol((const char *)(pbuf+5));
				if(addr != 0) {
					set_uid(addr);
					printf("WR|Z|%08ld|\r\n", get_uid());
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
