#ifndef _0_UART_H
#define _0_UART_H

#include "0_all.h"


#define UART_DEBUG 0
#define UART_GSM 1
#define uart0_rx_buf_sz 10
extern char uart0_rx_buf[uart0_rx_buf_sz];
extern volatile int printf_uart_0_1;

void read_uart0(uint8_t *buffer, int buffer_size, int sleep_ms_time);


void UART0_print_string(uint8_t *tx_buf);
void my_reverse(char str[], int len);
char* my_itoa(int num, char* str, int base);

void R_UART0_Send_blocking( uint8_t *  tx_buf, uint16_t tx_num);
void R_UART0_Receive_non_blocking(uint8_t *  rx_buf, uint16_t rx_num);
int  has_R_UART0_Receive_data(void);
void R_UART0_Receive_blocking(uint8_t *  rx_buf, uint16_t rx_num);

void R_UART1_Send_blocking(uint8_t * const tx_buf, uint16_t tx_num);
void R_UART1_Receive_non_blocking(uint8_t *  rx_buf, uint16_t rx_num);
int  R_UART1_Receive_is_full(void);
void R_UART1_Receive_blocking(uint8_t * const rx_buf, uint16_t rx_num);
void init_uart(void);

extern volatile unsigned int g_Uart0TxEnd;
extern volatile unsigned int g_Uart1TxEnd;
extern volatile unsigned int g_Uart2TxEnd;

extern volatile unsigned int g_Uart0RxEnd;
extern volatile unsigned int g_Uart1RxEnd;
extern volatile unsigned int g_Uart2RxEnd;

#if MACHINE == UBUNTU
MD_STATUS  R_UART0_Receive(uint8_t * const rx_buf, uint16_t rx_num);
MD_STATUS  R_UART1_Receive(uint8_t * const rx_buf, uint16_t rx_num);
int connect_to_at_server(void);
int create_at_server_thread(void);
extern int sock_fd_at_server;
int create_debug_socket(void);

#endif


#endif
