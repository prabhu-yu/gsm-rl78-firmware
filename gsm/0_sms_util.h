#ifndef _0_SMS_UTIL_H
#define _0_SMS_UTIL_H

#include "0_all.h"
//following is prefixed for every sms request. by this, we save a lot of unwanted sms

//#define CMD_PRE "108*"
#define CMD_PRE "108*"
#define STRING_CMGL "+CMGL:"

void sms_delete_index_based(int index);
int sms_parse(
        char *ptr_cmgl,
        int *ptr_index,
        char address[],
        int *ptr_yy,int *ptr_MM,int *dd,int *hh,int *mm,int *ss,int *tz,
        char sms_message_body[] );
void sms_delete_all(void);
void issue_CPMS_Q(void);
void issue_CPMS_EQ_Q(void);
void sms_delete_all(void);
void sms_delete_index_based(int index);
void sms_list_all(void);
int  process_sms(void);
void process_sms_all(void);
void sms_list_all_cmgr(void);
int sms_write_to_modem( char *mob_no, char *sms );

void sms_list_cmgr(int index);
int process_sms_cmti(void );
void pwr_up_sms_append(void);
void broadcast_sms(char *sms);
int parse_cmss_timeout(void  * buf, unsigned int timeout_seconds );

extern char *sms_cmd_str[];
extern text_command_t sms_cmd;

#endif


