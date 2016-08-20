#include	"24l01.h"

uint8_t uid[5]={0x2D, 0x1A, 0x00, 0x00, 0x01};

static uint8_t rf_rx_buf[32];
static uint8_t rf_rx_done;
static uint8_t rf_rx_len;
static uint8_t rf_tx_done;
static uint8_t rf_max_rt;

uint8_t rf_read_reg(uint8_t addr) {
uint8_t data;

	addr &= 0x1F;
	nss_low();
	spi_rw_byte(CMD_READ_REG | addr);	
	data = spi_rw_byte(0x00);
	nss_high();
	
	return data;
}

void rf_write_reg(uint8_t addr, uint8_t data) {
	addr &= 0x1F;
	nss_low();
	spi_rw_byte(CMD_WRITE_REG | addr);
	spi_rw_byte(data);
	nss_high();
}

void rf_write_bytes(uint8_t addr, uint8_t *buf, uint8_t len) {
uint8_t i;
	nss_low();
	spi_rw_byte(addr);
	for(i=0; i<len; i++) {
		spi_rw_byte(*buf++);
	}
	nss_high();
}

void rf_read_bytes(uint8_t addr, uint8_t *buf, uint8_t len) {
uint8_t i;
	nss_low();
	spi_rw_byte(addr);
	for(i=0; i<len; i++) {
		*buf++ = spi_rw_byte(0x00);
	}
	nss_high();
}

void rf_flush_tx(void) {
	nss_low();
	spi_rw_byte(CMD_FLUSH_TX);
	nss_high();	
}

void rf_flush_rx(void) {
	nss_low();
	spi_rw_byte(CMD_FLUSH_RX);
	nss_high();	
}

uint8_t rf_read_rxlen(void) {
uint8_t len;

	nss_low();
	spi_rw_byte(CMD_READ_RXLEN);
	len = spi_rw_byte(0x00);
	nss_high();
	return len;
}

void set_rf_channel(uint8_t ch) {
	rf_ce_low();
	rf_write_reg(0x05, ch);	
	rf_ce_high();
}

void rf_init(void) {
	spi_init();
	nss_high();	
	rf_ce_low();
	rf_write_reg(0x01, 0x01);	//enable pipe 0 auto ack
	rf_write_reg(0x02, 0x01);	//enable data pipe 0
	rf_write_reg(0x03, 0x03);	//5 bytes address
	rf_write_reg(0x04, 0x3A);	//10 times retransmit, wait 1000us
	rf_write_reg(0x05, 0x00);	//set rf channel 0
	rf_write_reg(0x06, 0x23);	//250kbps 0dbm
	
	rf_write_bytes(CMD_WRITE_REG|0x0A, uid, sizeof(uid));	//set rx address
	rf_write_bytes(CMD_WRITE_REG|0x10, uid, sizeof(uid));	//set tx address
	rf_write_reg(0x11, 32);		//data pipe0 32 bytes
	rf_write_reg(0x1c, 0x01);	//pipe 0 dynamic payload len
	rf_write_reg(0x1d, 0x07);	//dynamic payload len
	rf_flush_tx();
	rf_flush_rx();
	rf_write_reg(0x00, 0x0F);	//enable crc, power up, ptx
	rf_ce_high();
}

void rf_enter_rx(void) {
	rf_ce_low();
	rf_flush_rx();
	rf_write_reg(0x07, 0x70);
	rf_write_reg(0x00, 0x0F);
	rf_ce_high();
}

uint8_t rf_read_payload(uint8_t *buf) {
uint8_t len=0, i;
	
	len = rf_read_rxlen();
	if(len <= 32) {
		nss_low();
		spi_rw_byte(CMD_RX_PAYLOAD);
		for(i=0; i<len; i++) {
			*buf++ = spi_rw_byte(0x00);
		}
		nss_high();
	}	else {
		len = 0;
	}
	return len;
}

void rf_ack_payload(uint8_t *buf, uint8_t len) {
	rf_ce_low();
	rf_write_bytes(CMD_ACK_PAYLOAD, buf, len);
	rf_ce_high();
}

void rf_write_payload(uint8_t *buf, uint8_t len) {
	if((len>0) && (len<=32)) {
		rf_ce_low();
		rf_write_bytes(CMD_TX_PAYLOAD, buf, len);
		rf_write_reg(0x00, 0x0E);
		rf_ce_high();	
	}
}

uint8_t is_rf_received(void) {
	if(rf_rx_done) {
		rf_rx_done = 0;
		return 1;
	} else {
		return 0;
	}
}

uint8_t is_rf_sent(void) {
	if(rf_tx_done) {
		rf_tx_done = 0;
		return 1;
	} else {
		return 0;
	}
}

uint8_t is_rf_mrt(void) {
	if(rf_max_rt) {
		rf_max_rt = 0;
		return 1;
	} else {
		return 0;
	}
}

uint8_t *get_rf_buf(void) {
	return rf_rx_buf;
}

uint8_t get_rf_cnt(void) {
	return rf_rx_len;
}

void rf_isr(void) {
uint8_t status;

	status = rf_read_reg(0x07);

	if(status & 0x40) {		//rx_dr
		rf_rx_len = rf_read_payload((uint8_t *)rf_rx_buf);
		rf_rx_done = 1;
		rf_write_reg(0x07, 0x40);
	}
	
	if(status & 0x20) {		//tx_ds
		rf_tx_done = 1;
		rf_write_reg(0x07, 0x20);
	}
	
	if(status & 0x10) {		//max_rt
		rf_max_rt = 1;
		rf_write_reg(0x07, 0x10);
	}
	rf_flush_tx();
	rf_flush_rx();
	rf_ce_low();
}

/************************* end of file ************************************/
