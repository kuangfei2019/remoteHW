#include	"bsp.h"

#define		MASTER_ACTIVE			0x01
#define		NODE_ACTIVE				0x02
#define		DEV_ACTIVE				0x04

static 		uint8_t 					nw_state;

static 		uint8_t 					uart_tx_buf[32];
static 		uint8_t						byte_in_frame_interval;

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
			
			uint8_t len, remain, *pbuf;
			uint8_t buf[64];
			
			len = get_uart_cnt();
			pbuf = get_uart_buf();
				
				//数据小于32字节直接发送
				if(len < RF_BUF_SIZE) {
					buf[0] = 0x11;
					memcpy(buf+1, pbuf, len);
					rf_write_payload(buf, len+1);
				} else {
					//将大于32（含）字节的数据拆分为两帧分别发送
					remain = len - RF_BUF_SIZE+2;
					buf[0] = 0x21;
					memcpy(buf+1, pbuf, RF_BUF_SIZE-1);
					buf[RF_BUF_SIZE] = 0x22;
					memcpy(buf+RF_BUF_SIZE+1, pbuf+RF_BUF_SIZE-1, remain-1);
					
					rf_write_payload(buf, RF_BUF_SIZE);
					
					//等待第一帧数据发送完成
					for(uint8_t i=0; i<10; i++) {
						delay(3);
						if(is_rf_sent()) {
							nw_state &= ~DEV_ACTIVE;
							nw_state |= NODE_ACTIVE;
							break;
						}
					}
					
					//获取两帧间隔之间可能收到的数据
					if(is_rf_received()) {
						nw_state |= (NODE_ACTIVE|DEV_ACTIVE);
						len = get_rf_cnt();
						memcpy(uart_tx_buf+1, get_rf_buf(), len);
						byte_in_frame_interval = len;
					}			
					
					//发送第二帧数据
					rf_write_payload(buf+RF_BUF_SIZE, remain);
				}

				//等待数据发送完成
				for(uint8_t i=0; i<10; i++) {
					delay(3);
					if(is_rf_sent()) {
						nw_state &= ~DEV_ACTIVE;
						nw_state |= NODE_ACTIVE;
						break;
					}
				}
		}
		
		//定时发送查询指令，读取节点数据，更新网络状态，返回数据至PC
		if(is_t05_arrival()) {
			uint8_t len=0;
			
			led_on();
			if(byte_in_frame_interval) {
				nw_state |= (NODE_ACTIVE|DEV_ACTIVE);
				len = byte_in_frame_interval;
				byte_in_frame_interval = 0;
			} else if(is_rf_received()) {
				nw_state |= (NODE_ACTIVE|DEV_ACTIVE);
				len = get_rf_cnt();
				memcpy(uart_tx_buf+1, get_rf_buf(), len);						
			} else {
				rf_write_payload("X", 1);
				//等待数据发送完成
				for(uint8_t i=0; i<10; i++) {
					delay(2);
					if(is_rf_sent()) {
						nw_state &= ~DEV_ACTIVE;
						nw_state |= NODE_ACTIVE;
						//发送成功则清除lost pack计数
						rf_write_reg(0x05, rf_read_reg(0x05));	
						break;
					}
				}				
				
				if(is_rf_mrt()) {
					nw_state &= ~(NODE_ACTIVE|DEV_ACTIVE);
					//如果丢包严重，则强制复位，重新选择信道
					if(is_lost_of(10)) {
						soft_reset();
					}
				}
				
				if(is_rf_received()) {
					nw_state |= (NODE_ACTIVE|DEV_ACTIVE);
					len = get_rf_cnt();
					memcpy(uart_tx_buf+1, get_rf_buf(), len);						
				}
			}
			
			if(len>0) {
				//发送成功则清除lost pack计数
				rf_write_reg(0x05, rf_read_reg(0x05));	
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