#include	"bsp.h"
#include	<string.h>

uint8_t net_status;
uint8_t tx_buf[64];
uint8_t tx_cnt;

int main( void ) { 

	bsp_init();	
	led_off();
	net_status = 0x01;
	while(1) {
		//如果串口接收到数据则发送，否则发送心跳包
		if(is_uart_received()) {
			rf_write_payload(get_uart_buf(), get_uart_cnt());
			delay(100);
		} else {
			rf_write_payload("x", 1);
		}
		led_on();
		delay(2);
		
		//通过检查RF中断判断是否为带数据的ack，从而更新网络状态标识
		if(is_rf_mrt()) {
			net_status &= ~0x06;
		}
		
		if(is_rf_sent()) {
			net_status &= ~0x04;
			net_status |= 0x02;
		}
		
		tx_cnt = 1;
		if(is_rf_received()) {
			net_status |= 0x06;
			tx_cnt = get_rf_cnt();
			memcpy(tx_buf+1, get_rf_buf(), tx_cnt);
			tx_cnt += 1;
		}
		//如果节点返回数据，则将数据附加在网络状态之后发送到上位机，否则只发送网络状态
		tx_buf[0] = net_status;
		for(uint8_t i=0; i<tx_cnt; i++) {
			putchar(tx_buf[i]);
		}
		led_off();
		
		delay(1000);
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