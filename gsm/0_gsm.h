#ifndef _0_GSM_H
#define _0_GSM_H

#include "0_all.h"

extern char send_sms_body_160[160];
extern bool_t send_sms_flag;
extern bool_t activate_relays_based_on_oper_stat_flag;
extern int time_t_mcuable_changed;


extern volatile char  at_rx_buf[AT_RX_BUF_SZ];
void reset_at_rx_buf(void);
void reset_send_sms_body_160(void);
void issue_AT_command(void);

int read_unsolicited_result_code(void);
void strip_country_code(char  *phone_no);
void process_all_gsm_input(void);
int process_call(void);

void reset_modem(void);
void detect_at_rx_buf_overflow_and_clear(void);
void dump_at_rx_buf(void);
void remove_ok(char *buf);

void dump_at_rx_buf_len(void);
void init_gsm1(void);
void process_all_gsm_input(void);
void reset_gsm_modem(void);
void issue_cmgl_all(void);
int process_cmgl_output(void );
void process_modem(void);

int process_clcc_output(void);
void issue_clcc(void);
void issue_cclk(void);
void process_cclk_output(void);
void gsm_config_write(void);
void enable_date_time_sync_and_sms_storage_area(void);

// AT commands
#define OK_RESP_STR "\r\nOK\r\n"
#if 0
#define AT   "AT"
#define ATE0  "ATE0"
#define CMGF1 "AT+CMGF=1"
#define CLIP1 "AT+CLIP=1"
#define CLIP0 "AT+CLIP=0"
#define CSQ   "AT+CSQ"
#define ATH0  "ATH0"
#define CSCLK2  "AT+CSCLK=2"

#define CPMS  "AT+CPMS?"
#define CPMS_EQ_Q  "AT+CPMS=?"
#define CPMS_EQ_ME_ME_ME "AT+CPMS=\"ME\",\"ME\",\"ME\""

#define CTZU_EQ_1  "AT+CTZU=1\r"
#define CLTS_EQ_1 "AT+CLTS=1\r"
#define CFUN "AT+CFUN=1,1\r"
#define AT_AND_W "\nAT&W0\r\n"
#define AT_AND_V AT&V

#define AT_CMGL_ALL "AT+CMGL=\"ALL\""

#define CTRL_Z "\x1A"
#define ESCAPE "\x1B"
#endif
#endif

