#include	"bsp.h"

#define		MASTER_ACTIVE			0x01
#define		NODE_ACTIVE				0x02
#define		DEV_ACTIVE				0x04

static 		uint8_t 					nw_state;

static 		uint8_t 					uart_tx_buf[32];

int main( void ) {

	bsp_init();	
	led_off();
	nw_state = MASTER_ACTIVE;
	
	while(1) {

		if(is_uart_received()) {
			send_packet(get_uart_buf(), get_uart_cnt());
			tim1_init(500);
			is_t05_arrival();
		}
		
		if(is_t05_arrival()) {
			uint8_t len=0;
			
			led_on();
			rf_write_payload("x", 1);
			delay(10);
			
			if(is_rf_sent()) {
				nw_state &= ~DEV_ACTIVE;
				nw_state |= NODE_ACTIVE;
			}
			if(is_rf_received()) {
				nw_state |= (NODE_ACTIVE|DEV_ACTIVE);
				len = get_rf_cnt();
				memcpy(uart_tx_buf+1, get_rf_buf(), len);
			}
			if(is_rf_mrt()) {
				nw_state &= ~(NODE_ACTIVE|DEV_ACTIVE);
			}
			
			uart_tx_buf[0] = nw_state;
			for(uint8_t i=0; i<len+1; i++) {
				putchar(uart_tx_buf[i]);
			}
			
			led_off();
		}
	}
}

#pragma vector=UART1_R_RXNE_vector      //USART RXNE OR IDLE PE
__interrupt void usartr_handler(void) {
  uart_isr();
}

#pragma vector=EXTI1_vector       			//EXTI1
__interrupt void exti1_handler(void) {
	if(PB_IDR_IDR5 == 0) {
		rf_isr();
	}
}

#pragma vector=TIM1_OVR_UIF_vector      //TIM1_OVR_UIF
__interrupt void tim1_handler(void) {
  tim1_isr();
}

#pragma vector=TIM2_OVR_UIF_vector      //TIM2_OVR_UIF
__interrupt void tim2_handler(void) {
  tim2_isr();
}