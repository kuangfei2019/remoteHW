#include	"bsp.h"

static uint8_t uart_tx_buf[UART_BUF_SIZE];

int main( void ) {

	bsp_init();	
	led_off();
	
	config_mode();

	while(!select_channel());
	
	//清除离线计数，避免启动后再次复位
	clr_offline_cnt();

	while(1) {
		
		iwdg_refresh();
		
		//串口收到数据后存入2401 TX buf，等待主站下一次读取时发送
		if(is_uart_received()) {
			rf_ack_payload(get_uart_buf(), get_uart_cnt());
		}
		
		if(is_rf_sent()) {
			
		}
		
		//将2401接收到的数据通过串口发送
		if(is_rf_received()) {
		uint8_t len;

			led_on();
			len = read_packet(uart_tx_buf, sizeof(uart_tx_buf));
			for(uint8_t i=0; i<len; i++) {
				putchar(uart_tx_buf[i]);
			}
			led_off();
			
			//接收到无线数据清除离线计数
			clr_offline_cnt();			
		}
		
		//如果超过10次未接收到无线数据则强制重启，重新搜索信道
		if(is_offline(10)) {
			soft_reset();
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
	update_offline_cnt();
  tim1_isr();
}

#pragma vector=TIM2_OVR_UIF_vector      //TIM2_OVR_UIF
__interrupt void tim2_handler(void) {
  tim2_isr();
}