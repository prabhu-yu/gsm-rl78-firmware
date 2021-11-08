#ifndef MY_DATAFLASH_H
#define MY_DATAFLASH_H

#include "0_all.h"

extern void data_flash_driver_init(void);
extern void store_in_data_flash(void);
extern void store_relay_status(uint16_t relay, uint16_t status);
extern void dump_conf(void);
extern void reset_config_mem(void);
extern int store_phone_no(int index, const char *phone_no);
extern void ram_to_dataflash(void);
extern void dataflash_to_ram(void);
extern unsigned int is_it_stored_phone_no(const char *phone_no);
extern int get_security_byte(void);
extern int delete_phone_no(const char *phone_no);
extern void store_deviceA_mode(uint8_t mode);
extern char* get_stored_ph_number(int index);
extern void delete_phone_no_at_index(int index);
extern int store_to_dataflash(text_command_t cmd, const void *param1, const void *param2, const void *param3, const void *param4, const void *param5);
extern void fake_fixed_timer_switch_tt(void);
extern void dump_conf_oper(void);
extern unsigned int is_valid_text_cmd(unsigned int text_command);
extern char *bool_str(bool_t i);
char* new_event_t_str(new_event_t index);
char *device_type_str(device_type_t index);
char* relay_status_t_str(relay_status_t index);
void provision_to_defaults(void);
int is_valid_device_type(device_type_t type);
char *starter_mode_str(starter_mode_t i);
extern prov_t prov;
extern oper_t oper;

#endif
