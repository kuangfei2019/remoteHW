#include	"protocol.h"

const char channel[16] = {05, 11, 14, 19, 26, 31, 37, 42,\
													 51, 57, 63, 71, 78, 82, 87, 91};

uint8_t select_channel(void) {
uint8_t i;

	for(i=0; i<16; i++) {
		set_rf_channel(i);
		delay(1000);
		if(!is_uart_received()) {
			break;
		}
	}
	
	return i;
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
