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
