#include	"bsp.h"
#include	<string.h>

int main( void ) {

	bsp_init();
	
	led_off();
	while(1) {
		if(is_uart_received()) {
			rf_ack_payload(get_uart_buf(), get_uart_cnt());
		}
		
		if(is_rf_sent()) {
			led_on();
			delay(10);
			led_off();
		}
		
		if(is_rf_received()) {
			uint8_t *buf = get_rf_buf();
			for(uint8_t i=0; i<get_rf_cnt(); i++) {
				putchar(buf[i]);
			}
			led_on();
			delay(10);
			led_off();
		}
		delay(100);
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