#ifndef _MY_MISC_H
#define _MY_MISC_H

#include "0_all.h"
extern volatile uint32_t timer_cnt_ms;
void app_init(void);
void lower_string(char s[], uint16_t len);
void data_flash_driver_init(void);
void store_in_data_flash(void);
void process_keyboard(uint8_t kb_input);
void msleep(int milli_seconds);
void sleep_ms(int ms);
void dump_oper(void);
void reset_mcu(int reset_reason);

extern char *relay_status_string[];
extern char *relay_number_string[];
extern char *motor_oper_status_string[];
extern void diag_gsm(void);
void print_float(float f);
void print_line_break(void);
void print_line_star_break(void);

#if MACHINE == RL78   
void sleep(unsigned int seconds);
#endif

#endif
