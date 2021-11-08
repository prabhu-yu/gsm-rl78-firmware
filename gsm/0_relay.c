#include "0_all.h"

void set_starter_oper_status(unsigned int starter_number,  bool_t on_off) {
	printf("Enter >>> set_starter_oper_status(starter_number=%u on_off=%u)\n",starter_number,on_off );
	switch(on_off){
		case on:
			printf("ON\n");
			set_relay_oper_status(starter_number*2, relay_on_hold_off);
			set_relay_oper_status((starter_number*2)+1, relay_off);
			break;
		case off:
			printf("OFF\n");
			set_relay_oper_status((starter_number*2)+1, relay_on_hold_off);
			set_relay_oper_status(starter_number*2, relay_off);
			break;
	}
	printf("Return <<< set_starter_oper_status\n");
}

void init_oper_data(void) {
    printf("Enter: init_oper_data()\n");
	switch( prov.basic_prov.device_type ) {
        case invalid_max_device_mode:
            printf("invalid_max_device_mode\n");
			break;
		case manual_switch:
		case fixed_timer_switch:
		case motor_starter:
        case astro_timer_switch:
			init_oper_data1(mcu_reboot);
			break;
	}
    oper.current_water_level = ~0;
    oper.previous_water_level = ~0;
    printf("Return: init_oper_data()\n");
}

#if MACHINE == RL78
#endif
void activate_relays_based_on_oper_stat(void) {
    int relay,status,i;
    int hold_seconds;
    printf("activate_relays_based_on_oper_stat \n");
    if(!activate_relays_based_on_oper_stat_flag) {
    	 printf("Return: flag_activate_relays_based_on_oper_stat : false \n");
    	return;
    }
    for(relay = 0; relay < MAX_RELAYS; relay++) {
        status = get_relay_oper_status(relay);
    	if(relay_on_hold_off == status) {
            set_relay(relay, on);
    		printf("hold seconds[%u] = %u \n", relay, get_relay_hold_seconds(relay));
    		hold_seconds = get_relay_hold_seconds(relay);
    		for(i = 0; i < hold_seconds; i++) {
                printf("\n sleep i=%u", i);
                R_WDT_Restart();sleep_ms(1000);R_WDT_Restart();
            }
    		set_relay(relay, off);
    	} else {
    		set_relay(relay, status);
    	}
    }
    activate_relays_based_on_oper_stat_flag = false;
    printf("Return:activate_relays_based_on_oper_stat \n");
}

void power_up_sms_append(void) {
    printf("enter : power_up_sms_append\n");
	char temp_str[100];
	send_sms_body_160[0] = 0;
	switch(prov.basic_prov.device_type) {
    case manual_switch:
    case fixed_timer_switch:
    case astro_timer_switch:
    case motor_starter:
    case water_sensor_tanks_1:
        //snprintf(temp_str, sizeof(temp_str), "POWER UP\nTYPE: %s\n", device_type_str(prov.basic_prov.device_type));
        append_to_sms_memory("POWER UP\n");
        send_sms_flag = true;
        break;
        //printf(" For water sensor: no pwr up: it'll soak/send\n");
        //break;
    default:
        printf(" power up sms is not for this mode...\n");
    }
    printf("return : power_up_sms_append()\n");
}

/* relay 0,1
 * status 1 (on), 0 (off)
 * */
void set_relay(int relay, int status) {
    printf("Enter: set_relay(relay=%d, status=%d) \n", relay, status);
    switch(relay){
        case 0:
            if(status) {
#if MACHINE == RL78
                RL1_PORT |= RL1_PORT_PIN_OP_1; //turn ON
#endif
            } else {
#if MACHINE == RL78
    		RL1_PORT &= ~RL1_PORT_PIN_OP_1; //turn OFF
#endif
            }
            break;
        case 1:
            if(status) {
#if MACHINE == RL78
                RL2_PORT |= RL2_PORT_PIN_OP_1; //turn ON
#endif
            } else {
#if MACHINE == RL78
                RL2_PORT &= ~RL2_PORT_PIN_OP_1; //turn OFF
#endif
            }
            break;
        default:
            printf(" Error\n");
    }
    R_WDT_Restart();
    printf("Return: set_relay()\n");
}

void set_relay_oper_status(unsigned int relay, relay_status_t status) {
    printf("Enter :set_relay_oper_status(relay=%d, status=%d)\n", relay, status);
    if (relay >= MAX_RELAYS) {
    	printf("Return : set_relay_oper_status\n");
        return;
    }
    oper.relay_status[relay] = status;
    return;
}

int get_relay_oper_status(int relay) {
    int status;
    printf("get_relay_oper_status(relay=%d)\n", relay);
    status = oper.relay_status[relay]; // TODO CHECK BOUNDERY
    printf("status=%d\n", status);
    return status;
}

int get_relay_hold_seconds(int relay) {
	int hold_seconds=0;
    printf("Enter : get_relay_hold_seconds(relay=%d)\n", relay);
    switch(relay){
    case 0:
        hold_seconds = prov.motor_prov_obj.start_relay_hold_duration;
        break;
    case 1:
        hold_seconds = prov.motor_prov_obj.stop_relay_hold_duration;
        break;
    }
    printf("Return : get_relay_hold_seconds() hold_seconds=%d\n", hold_seconds);
    return hold_seconds;
}

void copy_reason_to_sms1(new_event_t reason) {
    printf("Enter: copy_reason_to_sms1\n");
    switch(reason) {
    case new_text_command_received:
        printf(", reason = cmd\n");
        append_to_sms_memory("TRIGGER: SMS\n");
        break;
    case mcu_reboot:
        printf(", reason = reboot\n");
        append_to_sms_memory("TRIGGER: ReBOOT\n");
        break;
    case new_phone_call_received:
        append_to_sms_memory("TRIGGER: CALL\n");
        break;
    default:
        break;
    }
    printf("Return : copy_reason_to_sms1\n");
}
void append_to_sms_memory(char *text) {
    printf("Enter: append_to_sms_memory\n");
    strcat(send_sms_body_160, text);
    printf("Return : append_to_sms_mem\n");
}
/* return 0 if all is okay*/
unsigned int validate_starter_number(unsigned int starter_number) {
    if (starter_number < MAX_STARTERS) {
        return 0;
    }
    return 1;
}
/* return 0 if all is okay*/
unsigned int validate_starter_status(unsigned int starter_number) {
    if (starter_number < bool_invalid) {
        return 0;
    }
    return 1;
}
/* return 0 if all is okay*/
unsigned int validate_starter_mode(unsigned int starter_mode) {
    if (starter_mode < starter_mode_invalid) {
        return 0;
    }
    return 1;
}

void handle_starter_mode(new_event_t reason,
    unsigned int cmd, unsigned starter_number, unsigned int starter_mode, char *phone_no) {
    printf("Enter: handle_starter_mode()\n");
    if (validate_starter_number(starter_number)) {
        printf("Error in Starter number\n");
        return;
    }
    if (validate_starter_mode(starter_mode)) {
        printf("Error in Starter number\n");
        return;
    }
    char temp_str_100[100];
    switch(reason) {
    case new_text_command_received:
        switch(cmd) {
            case cmd_write_starter_mode: //cmd*motor_N*status ;  0 or 1
                printf("starter_number=%u, starter_mode=%u\n", starter_number, starter_mode);
                store_to_dataflash(cmd, &starter_number, &starter_mode, NULL, NULL, NULL);
                break;
            default:
                break;
        }
        break;
    default:
        printf(" ERROR 121234123");
        break;
    }

    switch(reason) {
    case new_text_command_received:
        send_sms_flag = activate_relays_based_on_oper_stat_flag = true;
        snprintf(temp_str_100, sizeof(temp_str_100), "WRITE: MOTOR-%hhu: %s\nBy:%s",
            starter_number, starter_mode_str(starter_mode), phone_no);
        append_to_sms_memory(temp_str_100);
        copy_reason_to_sms1(reason);
        activate_relays_based_on_oper_stat_flag = true;
        break;
    }
    printf("Return: handle_starter_mode\n");
}

void handle_starter_status_mode_automatic(new_event_t reason,
    unsigned int cmd, unsigned starter_number, unsigned int starter_status, char *phone_no) {
    printf("Enter: handle_starter_status_mode_automatic()\n");
    if (validate_starter_number(starter_number)) {
        printf("Error in Starter number\n");
        return;
    }
    if (validate_starter_status(starter_status)) {
        printf("Error in Starter number\n");
        return;
    }

    char temp_str_100[100];
    switch(reason) {
    case mcu_reboot:
        if(on == prov.motor_prov_obj.starter_status[starter_number]) {
            starter_status = on;
        } else {
            starter_status = off;
        }
        set_starter_oper_status(starter_number, starter_status);
        break;
    case new_text_command_received:
        switch(cmd) {
            case cmd_write_starter_status: //cmd*motor_N*status ;  0 or 1
                printf("starter_number=%u, starter_status=%u\n", starter_number, starter_status);
                store_to_dataflash(cmd, &starter_number, &starter_status, NULL, NULL, NULL);
                set_starter_oper_status(starter_number, starter_status);
                break;
            default:
                printf(" Error 324235\n");
                break;
        }
        break;
    case new_phone_call_received:
        if(on == prov.motor_prov_obj.starter_status[starter_number]) {
            printf("TURN OFF\n");
            starter_status = off;
        } else {
            printf("TURN ON\n");
            starter_status = on;
        }
        store_to_dataflash(cmd_write_starter_status, (void *) &starter_number, (void *) &starter_status, NULL, NULL, NULL);
        set_starter_oper_status(starter_number, starter_status);
        break;
    default:
        printf(" ERROR 121234123");
        break;
    }

    switch(reason) {
    case new_phone_call_received:
    case mcu_reboot:
    case new_text_command_received:
        send_sms_flag = activate_relays_based_on_oper_stat_flag = true;
        snprintf(temp_str_100, sizeof(temp_str_100), "MOTOR%hhu: %s\nMODE: AUTOMATIC\nBY: %s\n",
            starter_number, bool_str(starter_status), phone_no);
        append_to_sms_memory(temp_str_100);
        copy_reason_to_sms1(reason);
        activate_relays_based_on_oper_stat_flag = true;
        break;
    }
    printf("Return: handle_starter_status_mode_automatic\n");
}

void handle_starter_status_temporary(new_event_t reason,
    unsigned int cmd, unsigned starter_number, unsigned int starter_status, char *phone_no) {
	char temp_str_100[100];
    printf("Enter: handle_starter_status_temporary V2 ...\n");

    if (validate_starter_number(starter_number)) {
        printf("Error in Starter number");
        return;
    }
    if (validate_starter_status(starter_status)) {
        printf("Error in Starter number");
        return;
    }

    switch(reason) {
    case new_text_command_received:
        switch(cmd) {
            case cmd_write_starter_status: //cmd*motor_N*status ;  0 or 1
                set_starter_oper_status(starter_number, starter_status);
                break;
            default:
                printf("Error 23653434\n");
                return;
                break;
        }
        break;
    case new_phone_call_received:
        printf("new_phone_call_received\n");
        if(on == prov.motor_prov_obj.starter_status[starter_number]) {
            printf("ON==>OFF  11111\n");
            prov.motor_prov_obj.starter_status[starter_number] = starter_status = off; //TODO. Should we use motor oper status instead of prov status?
        } else {
            printf("OFF==>ON 111111\n");
            prov.motor_prov_obj.starter_status[starter_number] = starter_status = on;
        }
        break;
    case mcu_reboot:
        printf("motor starter will be turned off after boot up\n");
        prov.motor_prov_obj.starter_status[starter_number] = starter_status = off;
        break;
    default:
        printf("ERROR  234234\n");
        break;
    }
    switch(reason) {
    case new_phone_call_received:
    case new_text_command_received:
    case mcu_reboot:
        snprintf(temp_str_100, sizeof(temp_str_100), "MOTOR%hhu: %s\nMODE: TEMPORARY\nBY: %s\n",
                starter_number, bool_str(starter_status), phone_no);
        append_to_sms_memory(temp_str_100);
        copy_reason_to_sms1(reason);
        printf("  reason asd \n");
        set_starter_oper_status(starter_number, starter_status);
        activate_relays_based_on_oper_stat_flag = true;
        break;
    default:
        break;
    }
    printf("Return: handle_starter_status_temporary()\n");
}

void handle_manual_switch(new_event_t reason) {
    unsigned int relay_number;
	char temp_str_100[100];
    printf("Enter:handle_manual_switch()\n");
    append_to_sms_memory("MANUAL SWITCH \n");
    switch(reason) {
    case mcu_reboot:
        for (relay_number = 0; relay_number<MAX_RELAYS; relay_number++ ) {
            set_relay_oper_status(relay_number, (relay_status_t)prov.manual_switch_status[relay_number]);
            snprintf(temp_str_100, sizeof(temp_str_100), "Manual Switch-%hhu %s\n",
                    relay_number, bool_str((bool_t)prov.manual_switch_status[relay_number]));
            append_to_sms_memory(temp_str_100);
        }
        break;
    case new_text_command_received:
    case new_phone_call_received:
        printf("boot/new_text_command_received\n");
        for (relay_number = 0; relay_number<MAX_RELAYS; relay_number++ ) {
            if(on == (relay_status_t) prov.manual_switch_status[relay_number]) {
                prov.manual_switch_status[relay_number] = off;
                printf("ON=>OFF\n");
            } else {
                prov.manual_switch_status[relay_number] = on;
                printf("OFF=>ON\n");
            }
            set_relay_oper_status(relay_number, (relay_status_t)prov.manual_switch_status[relay_number]);
            /* store_to_dataflash(cmd_write_manual_switch_status_provision,
                    (void *) &relay_number, (void *) &prov.manual_switch_status[relay_number],
                    NULL, NULL, NULL); */
            snprintf(temp_str_100, sizeof(temp_str_100), "Manual Switch-%hhu %s\n",
                    relay_number, bool_str((bool_t)prov.manual_switch_status[relay_number]));
            append_to_sms_memory(temp_str_100);
        }
        break;
    default:
        break;
    }
    switch(reason) {
    case new_phone_call_received:
        append_to_sms_memory("By-CALL\n");
        break;
    case mcu_reboot:
        append_to_sms_memory("By-ReBOOT\n");
        break;
    case new_text_command_received:
        append_to_sms_memory("By-SMS\n");
        break;
    }

    switch(reason) {
    case new_phone_call_received:
    case mcu_reboot:
    case new_text_command_received:
        send_sms_flag = activate_relays_based_on_oper_stat_flag = true;
        break;
    }
    printf("Return:handle_manual_switch()\n");
}

void init_oper_data1(new_event_t reason) {
	char temp_str_100[100];
	text_command_t command;
	unsigned int starter_number = 0, relay_number;
	bool_t starter_status, manual_switch_status;
	printf("Enter: init_oper_data1()\n");
    switch(reason) {
    case mcu_reboot:
    	switch(prov.basic_prov.device_type) {
            case motor_starter:
                printf("motor_starter \n");
                printf("starter_status[%hhu]=%s\n", starter_number, bool_str((bool_t)starter_number));
                switch(prov.motor_prov_obj.starter_mode[starter_number]) {
                case starter_mode_automatic:
                    printf("starter_mode_automatic 234234\n");
                    handle_starter_status_mode_automatic(mcu_reboot, 0, 0, 0, "POWER UP");
                    break;
                case starter_mode_temporary:
                    printf("starter_mode_temporary 1231\n");
                    handle_starter_status_temporary(mcu_reboot, 0, 0, 0, "POWER UP");
                    break;
                default:
                    printf("ERROR  54654 \n");
                    break;
                }
                break;
            case fixed_timer_switch:
                append_to_sms_memory("FIXED TIMER SWITCH\n");
                printf("fixed_timer_switch \n");
                timer_processing();
                break;
            case manual_switch:
                handle_manual_switch(reason);
                break;
            case astro_timer_switch:

                break;
            default:
                printf("errror 5462...\n");
                break;
        }
        break;
    default:
        break;
    }
    printf("Return: init_oper_data \n");
}
