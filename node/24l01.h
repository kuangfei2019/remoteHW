#ifndef		__24L01_H__
#define		__24L01_H__

#include	"bsp.h"

#define		nss_low()					{PA_ODR_ODR3=0;}
#define		nss_high()				{PA_ODR_ODR3=1;}

#define		rf_ce_high()			{PD_ODR_ODR3=1;}
#define		rf_ce_low()				{PD_ODR_ODR3=0;}


/************ 24L01 command definition *************/
#define		CMD_READ_REG			0x00
#define		CMD_WRITE_REG			0x20
#define		CMD_RX_PAYLOAD		0x61
#define		CMD_TX_PAYLOAD		0xA0
#define		CMD_FLUSH_TX			0xE1
#define		CMD_FLUSH_RX			0xE2
#define		CMD_REUSE_TXPL		0xE3
#define		CMD_READ_RXLEN		0x60
#define		CMD_ACK_PAYLOAD		0xA8
#define		CMD_TX_NOACK			0xB0
#define		CMD_NOP						0xFF

/*************** public function prototype ********/
void rf_init(void) ;
void set_rx_addr(uint32_t addr);
void set_tx_addr(uint32_t addr);
uint8_t rf_read_reg(uint8_t addr);
void rf_write_reg(uint8_t addr, uint8_t data);
void rf_write_bytes(uint8_t addr, uint8_t *buf, uint8_t len) ;
void rf_read_bytes(uint8_t addr, uint8_t *buf, uint8_t len) ;
void rf_flush_tx(void);
void rf_flush_rx(void);
void rf_enter_rx(void);
void set_rf_channel(uint8_t ch);
void rf_ack_payload(uint8_t *buf, uint8_t len);
uint8_t rf_read_rxlen(void);
uint8_t rf_read_payload(uint8_t *buf);
void rf_write_payload(uint8_t *buf, uint8_t len);
void rf_isr(void) ;
void clr_rf_buf(void);
uint8_t is_rf_received(void) ;
uint8_t is_rf_sent(void) ;
uint8_t is_rf_mrt(void) ;
uint8_t *get_rf_buf(void) ;
uint8_t get_rf_cnt(void);

#endif		//__24L01_H__
