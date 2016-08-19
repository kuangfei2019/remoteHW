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
