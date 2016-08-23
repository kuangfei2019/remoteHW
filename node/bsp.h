#ifndef		__BSP_H__
#define		__BSP_H__

#include 	<iostm8s003f3.h>
#include	<stdint.h>
#include	<stdio.h>
#include	<string.h>
#include	"24l01.h"
#include	"protocol.h"

#define		HSI								16000000
#define		HSE								8000000
#define		LSI								128000

#define		DEV_ID_LOC				0x4000

#define		UART_BUF_SIZE			64

#define		led_on()					{PD_ODR_ODR4=0;}
#define		led_off()					{PD_ODR_ODR4=1;}

#define		iwdg_refresh()		{IWDG_KR = 0xaa;}
#define 	soft_reset()    	((void (*) (void)) 0x6000) ()

//public function prototype
void bsp_init(void);
void iwdg_init(void);
void gpio_init(void);
void tim1_init(uint16_t ms);
void tim1_isr(void);
uint32_t get_systick(void);
void tim2_init(uint16_t ms);
void tim2_isr(void);
void delay(uint16_t ms);
void sleep(uint16_t sec);
uint32_t get_sysclk(void);
void uart_init(uint32_t baudrate);
void uart_isr(void);
int putchar(int c);
uint8_t is_uart_received(void);
uint8_t *get_uart_buf(void);
uint8_t get_uart_cnt(void);
void spi_init(void);
uint8_t spi_rw_byte(uint8_t data);
uint8_t is_irq_low(void);

#endif		//__BSP_H__
