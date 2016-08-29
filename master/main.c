#include	"bsp.h"

#define		MASTER_ACTIVE			0x01
#define		NODE_ACTIVE				0x02
#define		DEV_ACTIVE				0x04

static 		uint8_t 					nw_state;

static 		uint8_t 					uart_tx_buf[32];

int main( void ) {
	bsp_init();	
	led_off();

	config_mode();
	
	//设置网络状态
	nw_state = MASTER_ACTIVE;

	//选择空闲的信道
	select_channel();
	
	while(1) {
		
		iwdg_refresh();

		//如果串口接收到数据立即发送
		if(is_uart_received()) {
			send_packet(get_uart_buf(), get_uart_cnt());
		}
		
		//定时发送查询指令，读取节点数据，更新网络状态，返回数据至PC
		if(is_t05_arrival()) {
			uint8_t len=0;
			
			led_on();
			rf_write_payload("X", 1);
			delay(50);
			
			if(is_rf_sent()) {
				nw_state &= ~DEV_ACTIVE;
				nw_state |= NODE_ACTIVE;
				//发送成功则清除lost pack计数
				rf_write_reg(0x05, rf_read_reg(0x05));
			}
			if(is_rf_received()) {
				nw_state |= (NODE_ACTIVE|DEV_ACTIVE);
				len = get_rf_cnt();
				memcpy(uart_tx_buf+1, get_rf_buf(), len);						
			}
			if(is_rf_mrt()) {
				nw_state &= ~(NODE_ACTIVE|DEV_ACTIVE);
				//如果丢包严重，则强制复位，重新选择信道
				if(is_lost_of(10)) {
					soft_reset();
				}
			}
			
			uart_tx_buf[0] = (~nw_state<<4)|(nw_state&0x0F);
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