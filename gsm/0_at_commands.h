#ifndef _AT_COMMANDS_H
#define _AT_COMMANDS_H

#include "0_all.h"

extern uint8_t at_buf[at_buf_sz];

void delete_all_sms(void);
void init_sim(void);
void parse_at_resp_buffer(void);
uint16_t parse_gsm_buf(void);
#endif
