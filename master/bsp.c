#include	"bsp.h"

static uint8_t t05_arrival;
static uint32_t sys_tick;
static char uart_buf[UART_BUF_SIZE];
static uint8_t uart_rcv_cnt;
static uint8_t uart_received;

void bsp_init(void) {
	asm("SIM");
//	iwdg_init();
	gpio_init();
	tim1_init(500);
	uart_init(9600);
	rf_init();
	
	asm("RIM");
}

void iwdg_init(void) {
  IWDG_KR = 0xcc;   
  IWDG_KR = 0x55;  
  IWDG_PR = 0x06;
  IWDG_PR = 0xff;   
  IWDG_KR = 0xaa;  
}

void gpio_init(void) {
	PD_DDR_DDR4 = 1;
	PD_CR1_C14 = 1;
	PD_CR2_C24 = 1;			//LED
	PD_DDR_DDR3 = 1;
	PD_CR1_C13 = 1;
	PD_CR2_C23 = 1;			//RF_CE
	PB_DDR_DDR5 = 0;
	PB_CR1_C15 = 1;
	PB_CR2_C25 = 1;			//RF_IRQ	
	EXTI_CR1_PBIS = 0;
}

uint32_t get_sysclk(void) {
uint32_t clk;
	switch(CLK_CMSR) {
	case 0xE1:
		clk = HSI;
		break;
	case 0xD2:
		clk = LSI;
		break;
	case 0xB4:
		clk = HSE;
		break;
	default:
		break;
	}
	
	clk >>= (CLK_CKDIVR>>3);
	return clk;
}

void uart_init(uint32_t baudrate) {
uint16_t brr;

	CLK_PCKENR1 |= 0x08;
	brr = get_sysclk()/baudrate;
	if((get_sysclk()-brr*baudrate) >= (baudrate>>1)) {
		brr += 1;
	}
	UART1_BRR2 = ((brr>>8)&0xF0)|(brr&0x0F);
	UART1_BRR1 = (brr>>4)&0xFF;
	UART1_CR1 = 0x00;
	UART1_CR2 = 0x2c;
	UART1_CR3 = 0x00;	
}

int putchar(int c) {
	while(!UART1_SR_TXE);
	UART1_DR = c;
	return c;
}

uint8_t is_uart_received(void) {
	if(uart_received) {
		uart_received = 0;
		return 1;
	} else {
		return 0;
	}
}

uint8_t *get_uart_buf(void) {
	return (uint8_t *)uart_buf;
}

uint8_t get_uart_cnt(void) {
uint8_t cnt;
	cnt = uart_rcv_cnt;
	uart_rcv_cnt = 0;
	return cnt;
}

void uart_isr(void) {

	if(UART1_SR_RXNE) {
		uart_buf[uart_rcv_cnt++] = UART1_DR;
		uart_rcv_cnt %= UART_BUF_SIZE;
		tim2_init(10);
	}
}

void delay(uint16_t ms) {
uint16_t arr, psc;

	arr = get_sysclk()/1000;

  CLK_PCKENR1 |= 0x10;
	
	for(psc=1; psc<8; psc++) {
		if((arr>>=1) < 256) {
			break;
		}
	}

  TIM4_PSCR = psc;
  TIM4_ARR = arr;
  TIM4_EGR_UG = 1;
  TIM4_EGR_UG = 0;
  TIM4_SR_UIF = 0;
  TIM4_CR1_CEN = 1;

  while(ms--) {
    while(!TIM4_SR_UIF);
    TIM4_SR_UIF = 0;   //maybe the reference manual is wrong,
                        //This register is not read-clear.
  }

  TIM4_CR1_CEN = 0;
  CLK_PCKENR1 &= ~0x10;	
}

void tim1_init(uint16_t ms) {
uint16_t psc;

	psc = get_sysclk()/1000;

  CLK_PCKENR1 |= 0x80;

	TIM1_PSCRH = psc>>8;
	TIM1_PSCRL = psc&0xFF;
	TIM1_ARRH = ms >> 8;
	TIM1_ARRL = ms & 0xFF;
  TIM1_EGR_UG = 1;
  TIM1_EGR_UG = 0;
  TIM1_SR1_UIF = 0;
	TIM1_IER = 0x01;
  TIM1_CR1_CEN = 1;
}

void tim1_isr(void) {
	TIM1_SR1 = 0;
	t05_arrival = 1;
	sys_tick++;
}

uint8_t is_t05_arrival(void) {
	if(t05_arrival) {
		t05_arrival = 0;
		return 1;
	}
	return 0;
}

uint32_t get_systick(void) {
	return sys_tick;
}

void tim2_init(uint16_t ms) {
uint16_t arr, psc;

	arr = get_sysclk()/1000;

  CLK_PCKENR1 |= 0x20;
	
	for(psc=1; psc<16; psc++) {
		if((arr>>=1) == 0) {
			break;
		}
	}

	TIM2_PSCR = psc;
	TIM2_ARRH = ms >> 8;
	TIM2_ARRL = ms & 0xFF;
  TIM2_EGR_UG = 1;
  TIM2_EGR_UG = 0;
  TIM2_SR1_UIF = 0;
	TIM2_IER = 0x01;
  TIM2_CR1_CEN = 1;
}

void tim2_isr(void) {
	TIM2_SR1 = 0;
	TIM2_CR1_CEN = 0;
	
	uart_received = 1;
}

void sleep(uint16_t sec) {
	while(sec--) {
		iwdg_refresh();
		delay(1000);
	}
}

void spi_init(void) {
	PA_DDR_DDR3 = 1;
	PA_CR1_C13 = 1;
	PA_CR2_C23 = 1;			//SPI_NSS
	PC_DDR_DDR7 = 0;
	PC_CR1_C17 = 1;
	PC_CR2_C27 = 0;			//SPI_MISO
	PC_DDR_DDR6 = 1;
	PC_CR1_C16 = 1;
	PC_CR2_C26 = 1;			//SPI_MOSI
	PC_DDR_DDR5 = 1;
	PC_CR1_C15 = 1;
	PC_CR2_C25 = 1;			//SPI_SCK
	
  CLK_PCKENR1 |= 0x02;
  SPI_CR1 = 0x04;
  SPI_CR2 = 0x00;
  SPI_ICR = 0x00;
	
	SPI_CR1_SPE =1;	
}

uint8_t spi_rw_byte(uint8_t data) {
	while(!SPI_SR_TXE);
	SPI_DR = data;
	while(!SPI_SR_RXNE);
	return SPI_DR;
}

uint8_t is_irq_low(void) {
	return (PB_IDR_IDR5==0)? 1 : 0;
}


/******************************* END OF FILE **********************************/
