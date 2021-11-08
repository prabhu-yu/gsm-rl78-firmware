#ifndef _0_RELAY_H1
#define _0_RELAY_H1

#include "0_all.h"

void set_relay_oper_status(unsigned int relay, relay_status_t status);
void init_relays(void);
int get_relay_oper_status(int relay);
void set_relay(int relay, int status);
void init_oper_data1(new_event_t reason);
void activate_relays_based_on_oper_stat(void);
void power_up_sms_append(void);
void update_relays_oper_status_based_on_time_t_mcuable(void);
void init_oper_data(void);
int get_relay_hold_seconds(int relay);
void relays_oper_status_realted_work(void);
void set_motor_oper_status(int motor_no, bool_t status);
void update_relays_oper_status_based_on_starter_status(void);
void update_relays_oper_status_based_on_manual_switch_provision(void);
void set_starter_oper_status(unsigned int starter_number, bool_t on_off);
void copy_reason_to_sms1(new_event_t reason);
void append_to_sms_memory(char *text);
void handle_manual_switch(new_event_t reason);

void handle_starter_status_mode_automatic (new_event_t reason,
    unsigned int cmd, unsigned starter_number, unsigned int value1, char *phone_no);
void handle_starter_status_temporary (new_event_t reason,
    unsigned int cmd, unsigned starter_number, unsigned int value1, char *phone_no);

void handle_starter_mode(new_event_t reason,
    unsigned int cmd, unsigned starter_number, unsigned int starter_mode, char *phone_no);

#define REV_1_1 0x0101
#define REV_1_2 0x0102

#define HW_REV REV_1_1

#if HW_REV == REV_1_1

#define RL1_PORT       P1
#define RL1_PORT_PIN_OP_1   _20_Pn5_OUTPUT_1
#define RL2_PORT       P1
#define RL2_PORT_PIN_OP_1   _40_Pn6_OUTPUT_1

#elif  HW_REV == REV_1_2

#if MACHINE == RL78

#define RL1_PORT            P6
#define RL1_PORT_PIN_OP_1   _04_Pn2_OUTPUT_1
#define RL2_PORT            P3
#define RL2_PORT_PIN_OP_1   _02_Pn1_OUTPUT_1

#endif //MACHINE == RL78

#endif // HW_REV
#endif // _0_RELAY_H1
