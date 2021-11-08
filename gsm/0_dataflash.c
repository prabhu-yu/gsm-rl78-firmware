#include "0_all.h"

char *text_command_t_str(text_command_t index) {
    char *x[] = {
        "cmd_write_device_type",
        "cmd_get_status",
        "cmd_reset_mcu",
        "cmd_reset_prov",
        "cmd_set_mcu_boot_delay",
        "cmd_set_date_time",
        "cmd_write_time_zone",
        "cmd_write_password",
        "cmd_write_sms_reply_enabled",
    // WATER LEVEL RELATED
        "cmd_write_water_level_soak_time",
    // phone number RELATED
        "cmd_write_add_phone_number",
        "cmd_write_delete_phone_number",
        "cmd_read_phone_number",
    // Switch Timer related
        "cmd_write_light_24h_timer_table",
        "cmd_write_light_24h_timer_table_prov_max",
        "cmd_read_light_24h_timer_table",
    // Motor Related
        "cmd_write_starter_mode",
        "cmd_write_starter_status",
        "cmd_write_starter_n_call_aware",
        "cmd_write_starter_start_stop_relay_hold_time",
    //Manual switch related
        "cmd_write_manual_switch_status_provision",
    // Last value- keep updated
        "cmd_max"
    };

    if ((uint8_t)index < (sizeof(x)/sizeof x[0])) {
        return x[index];
    } else {
        return "text_command_t_str";
    }
}

char *device_type_str(device_type_t index) {
    static char *device_type_str[] = {
        "MANUAL_SWITCH",
        "FIXED_TIMER_SWITCH",
        "ASTRO_TIMER_SWITCH",
        "MOTOR_STARTER",
        "WATER_SENSOR_TANKS_1",
        "MOTOR_STARTER_WATER_SENSOR_TANKS_1",
        "MOTOR_STARTER_WATER_SENSOR_TANKS_2",
        "SCHOOL_TIMER",
        "INVALID_MAX_DEVICE_MODE"
    };

#define MY_SIZE (sizeof(device_type_str) / sizeof(device_type_str[0]) )
    if(index < MY_SIZE ) {
        return device_type_str[index];
    } else {
        return "ERROR_device_type_str";
    }
}

int is_valid_device_type(device_type_t type) {
    if(type < invalid_max_device_mode) {
        return 1;
    }
    return 0;
}

char *starter_mode_str(starter_mode_t i) {
    static char *starter_mode[] = {
        "STARTER_MODE_AUTOMATIC",
        "STARTER_MODE_TEMPORARY",
        "STARTER_MODE_TIMER",
        "STARTER_MODE_DURATION",
    };
#undef MY_SIZE
#define MY_SIZE (sizeof(starter_mode) / sizeof(starter_mode[0]) )
    if(i < MY_SIZE ) {
        return starter_mode[i];
    } else {
        return "ERROR_starter_mode_str";
    }
}

char *bool_str(bool_t i) {
    static char *bool_str1[] = { "OFF", "ON" };
#undef MY_SIZE
#define MY_SIZE (sizeof(bool_str1)/sizeof(bool_str1[0]))
    if(i< MY_SIZE) {
        return bool_str1[i];
    } else {
        return "ERROR_bool_str1";
    }
}

char* new_event_t_str(new_event_t index) {
    static char *new_event_t_str[] = { "mcu_reboot", "new_text_command_received", "new_phone_call_received" };
#undef MY_SIZE
#define MY_SIZE (sizeof(new_event_t_str)/sizeof(new_event_t_str[0]))
    if(index< MY_SIZE) {
        return new_event_t_str[index];
    } else {
        return "ERROR_new_event_t_str";
    }
}

char* relay_status_t_str(relay_status_t index) {
    char *new_event_t_str[] = { "relay_off", "relay_on", "relay_on_hold_off" };
#undef MY_SIZE
#define MY_SIZE (sizeof(new_event_t_str)/sizeof(new_event_t_str[0]))
    if(index< MY_SIZE) {
        return new_event_t_str[index];
    } else {
        return "relay_status_t_str";
    }
}

void reset_prov_mem(void) {
	// we never reset the "written_n" since that is used to check the eeprom life.
    memset(&prov.basic_prov.device_type, 0, sizeof(prov) - sizeof(prov.basic_prov.written_n));
}

void data_flash_driver_init(void) {
    //load from data flash to ram.
    printf("data_flash_driver_init\n");
#if MACHINE == RL78
    R_FDL_Create();sleep_ms(1000);
    R_FDL_Open();
#endif
    printf(" Return << data_flash_driver_init\n");
}

void dump_conf_oper(void) {
	print_line_break();
	printf("Enter: dump_conf_oper()\n");
    dump_conf();
    dump_oper();
    print_line_break();
}

void dump_conf(void) {
    unsigned int i;
    printf(" ############### dump_conf STARTS ###################\n");
    print_line_star_break();//#####################
    printf(" basic_prov\n");
    printf(" written_n: %u\n", prov.basic_prov.written_n);
    printf(" device_type:%hhu (%s)\n", prov.basic_prov.device_type,
            device_type_str((device_type_t)prov.basic_prov.device_type));
    //printf(" security:%u", prov.basic_prov.security);
    printf(" mcu_reset_reason: %hhu\n",prov.basic_prov.mcu_reset_reason);
    printf(" mcu_boot_delay: %hhu\n",prov.basic_prov.mcu_boot_delay);
    printf(" sms_reply_enabled: %hhu\n",prov.basic_prov.sms_reply_enabled);

    print_line_star_break();//######################
    printf(" Water level\n");
    printf(" water_level_soak_seconds:%u\n", prov.water_level_prov.water_level_soak_seconds);
    printf(" water_level_max_level:%u\n", prov.water_level_prov.max_level);

    print_line_star_break();//##################
    printf(" Phone Book\n");
    for(i=0; i < phone_no_list_sz; i++){
        printf("[%d]: valid: %u, phone_no: %s\n",
                i,
                prov.phone_book[i].valid,
                prov.phone_book[i].valid?prov.phone_book[i].phone_no:"");
    }

    print_line_star_break();//#################
    printf("max_rl_tt_provisioned: %hhu\n", prov.basic_prov.max_rl_tt_provisioned);
    printf(" Timer: Time Table\n");
    printf(" Index: hh:mm:ss  relay_status\n");
    for(i=0; i < relay_time_t_mcuable_sz; i++){
        printf("[%u] %u:%u:%u %u\n", i, prov.rl_tt[i].hour,
            prov.rl_tt[i].minutes, prov.rl_tt[i].seconds, prov.rl_tt[i].relay_status);
    }

    print_line_star_break();//#######################
    printf(" Manual relay switch\n");
    for(i=0; i<MAX_RELAYS; i++){
        printf("manual_switch_status(%hhu): %hhu\n", i, prov.manual_switch_status[i]);
    }
    print_line_star_break();//######################

    printf(" MOTOR RELATED\n");
    for (i =0;i<MAX_STARTERS; i++){
        printf("   starter_mode(%hhu): %s\n", i, starter_mode_str( (starter_mode_t)prov.motor_prov_obj.starter_mode[i]) );
        printf("   starter_status(%hhu): %s\n", i, bool_str( (bool_t) prov.motor_prov_obj.starter_status[i]) );
    }
    printf(" start_relay_hold_duration:%u\n",prov.motor_prov_obj.start_relay_hold_duration);
    printf(" stop_relay_hold_duration:%u\n", prov.motor_prov_obj.stop_relay_hold_duration);
    print_line_star_break();//######################

    printf(" ASTRO RELATED\n");
    printf(" lattitude=%f\n longitude=%f\n sunrise_solar_elevation_angle=%f\n sunset_solar_elevation_angle=%f\n",
            prov.astro_prov.lattitude,
            prov.astro_prov.longitude,
            prov.astro_prov.sunrise_solar_elevation_angle,
            prov.astro_prov.sunset_solar_elevation_angle);

    printf("*************** dump_conf ENDS *******************\n");
}

void provision_to_defaults(void) {
    unsigned int i;
    printf("Enter: provision_to_defaults()\n");
    reset_prov_mem();
    // basic prov
    prov.basic_prov.device_type = manual_switch;
    prov.basic_prov.device_type = fixed_timer_switch;
    prov.basic_prov.device_type = astro_timer_switch;
    prov.basic_prov.device_type = water_sensor_tanks_1;
    prov.basic_prov.device_type = motor_starter;

    prov.basic_prov.max_rl_tt_provisioned = 2; // 1 based
    prov.basic_prov.mcu_boot_delay = 15;
    prov.basic_prov.sms_reply_enabled = sms_reply_enabled;

    //water level
    prov.water_level_prov.water_level_soak_seconds = 2;
    prov.water_level_prov.max_level = 3;

    // phone book prov
    for (i=0; i < phone_no_list_sz; i++) {
        prov.phone_book[i].valid = 0;
        prov.phone_book[i].phone_no[0]= '\0';
        prov.phone_book[i].name[0]= '\0';
    }

    prov.phone_book[0].valid = true;
    strncpy(prov.phone_book[0].phone_no, DEBUG_NUMBER, phone_no_size);
    strncpy(prov.phone_book[0].name, "prabhu", phone_user_name_size);

    //Timer prov
    prov.rl_tt[0].hour= 6;
    prov.rl_tt[0].minutes= 0;
    prov.rl_tt[0].seconds= 0;
    prov.rl_tt[0].relay_status= 0;

    prov.rl_tt[1].hour= 18;
    prov.rl_tt[1].minutes= 0;
    prov.rl_tt[1].seconds= 0;
    prov.rl_tt[1].relay_status= 1;

    //manual switch
    for(i=0; i<MAX_RELAYS; i++){
        prov.manual_switch_status[i]= off;
    }

    // motor prov
    for (i =0; i<MAX_STARTERS; i++){
        prov.motor_prov_obj.starter_mode[i]= starter_mode_temporary;
        prov.motor_prov_obj.starter_status[i]=  off;
    }

    prov.motor_prov_obj.start_relay_hold_duration= 5;
    prov.motor_prov_obj.stop_relay_hold_duration= 8;

    //Gadag
    //prov.astro_prov.lattitude = 15.0 + (11 / 60.0) + 0 / (60 * 60.0);// North +v; South -ve;
    //prov.astro_prov.longitude = 75.0 + (44 / 60.0) + 0 / (60 * 60.0);// East +v ; West -ve;

    /*
    //hubli
    prov.astro_prov.lattitude = 15.364678;// North +v; South -ve;
    prov.astro_prov.longitude = 75.0740425;// East +v ; West -ve;
    //Delhi
    prov.astro_prov.lattitude = 28.0 + (37 / 60.0) + 0 / (60 * 60.0);// North +v; South -ve;
    prov.astro_prov.longitude = 77.0 + (13 / 60.0) + 0 / (60 * 60.0);// East +v ; West -ve;

    prov.astro_prov.lattitude = 15.0 + (11 / 60.0) + 0 / (60 * 60.0);// North +v; South -ve;
    prov.astro_prov.longitude = 75.0 + (44 / 60.0) + 0 / (60 * 60.0);// East +v ; West -ve;
    //prov.astro_prov.sunrise_solar_elevation_angle = -0.83;
    //prov.astro_prov.sunset_solar_elevation_angle = -0.83;
    */

    //prov.astro_prov.sunrise_solar_elevation_angle = -6;
    //prov.astro_prov.sunset_solar_elevation_angle = -6;
    ram_to_dataflash();
    dump_conf_oper();
    printf("Return : provision_to_defaults()\n");
}

int get_device_type(void) {
    printf(" get_device_type =%d", prov.basic_prov.device_type);
    return prov.basic_prov.device_type;
}

char* get_stored_ph_number(int index) {
    printf("Enter: get_stored_ph_number(%d)\n", index);
    if( index >= phone_no_list_sz){
        return NULL;
    }
    if( 1 == prov.phone_book[index].valid) {
        printf(" full %s", prov.phone_book[index].phone_no);
        return (char *)prov.phone_book[index].phone_no;
    } else {
        printf(" empty ");
        return NULL;
    }
    printf(" Return: get_stored_ph_number\n");
}

/* if it is stored, TURE
 * if it is not  FALSE */
unsigned int is_it_stored_phone_no(const char *phone_no) {
    int i;
    printf("Enter: is_it_stored_phone_no(%s)\n", phone_no);
    for(i=0; i < phone_no_list_sz; i++){
        printf("\n i=%d", i);
        if( 1 == prov.phone_book[i].valid){
            printf(" full %s", prov.phone_book[i].phone_no);
            if( NULL == strstr(phone_no, prov.phone_book[i].phone_no)){
                printf(" Not matched ");
            } else {
                printf(" Matched; TRUE "); // found
                return TRUE;
            }
        } else {
            //printf(" empty ");
        }
    }

    printf("Return : is_it_stored_phone_no\n");
    return FALSE;
}

/*
 * 0-9 if stored successfully
 * 100 if not can not be stored.
 * */
int delete_phone_no(const char *phone_no)
{
    int i;
    int already_stored = FALSE;

    printf(" delete_phone_no(%s)\n",phone_no);

    // find the duplicates if any
    for(i=0; i < phone_no_list_sz; i++){
        printf(" prov.phone_book[%u].phone_book[i].phone_no:%s\n", i, prov.phone_book[i].phone_no);
        if( 1 == prov.phone_book[i].valid){
            printf(", full");
            if( NULL == strstr(prov.phone_book[i].phone_no, phone_no )){
                printf(", Not found");
            } else {
                printf(", Found");
                already_stored = TRUE;
                break;
            }
        }
    }
    if(TRUE == already_stored ) {
            printf("Deleting\n");
            // i has location where it found the number
            prov.phone_book[i].valid = 0;
            printf(" prov.phone_book[%u].valid=%u\n", i,prov.phone_book[i].valid);
            ram_to_dataflash();
    } else {
        printf(" can not remove since phone_no not exist in DB\n");
    }
    return 0;
}

unsigned int is_valid_text_cmd(unsigned int text_command) {
    switch(text_command){
    case cmd_write_device_type:
    case cmd_get_status:
    case cmd_reset_mcu:
    case cmd_set_mcu_boot_delay:
    case cmd_set_date_time:
    case cmd_write_time_zone:
    case cmd_write_password:
    case cmd_write_sms_reply_enabled:
    case cmd_read_sw_hw_rev:

    case cmd_write_water_level_soak_time:
    case cmd_write_water_level_max_level:

    case cmd_write_add_phone_number:
    case cmd_write_delete_phone_number:
    case cmd_read_phone_numbers:

    case cmd_write_light_24h_timer_table:
    case cmd_write_light_24h_timer_table_prov_max:
    case cmd_read_light_24h_timer_table:

    case cmd_write_starter_mode:
    case cmd_write_starter_status:
    case cmd_write_starter_n_call_aware:
    case cmd_write_starter_start_stop_relay_hold_time:

    case cmd_write_manual_switch_status_provision:
    case cmd_write_lat_long:
    case cmd_write_sun_elevation_rise_set:
    case cmd_read_astro_prov:
        return 1;
    default:
        return 0;
    }
}

/*
 * 0-9 if stored successfully
 * 100 if not can not be stored.
 * */
int store_to_dataflash(text_command_t arg1, const void *arg2, const void *arg3, const void *arg4, const void *arg5, const void *arg6) {
    printf("Enter: store_to_dataflash(%d)\n", arg1);
    is_valid_text_cmd(arg1);
    switch (arg1){
        case cmd_write_starter_start_stop_relay_hold_time:
            printf(" start/stop=%u/%u\n",*(uint8_t *)arg2, *(uint8_t *)arg3);
            prov.motor_prov_obj.start_relay_hold_duration = *(uint8_t *)arg2;
            prov.motor_prov_obj.stop_relay_hold_duration = *(uint8_t *)arg3;
            break;
        case cmd_write_water_level_soak_time:
            printf(" water_level_soak_seconds=%u\n",*(uint8_t *)arg2);
            prov.water_level_prov.water_level_soak_seconds = *(uint8_t *)arg2;
            break;
        case cmd_write_water_level_max_level:
            printf(" water_level_soak_seconds=%u\n",*(uint8_t *)arg2);
            prov.water_level_prov.max_level = *(uint8_t *)arg2;
            break;
        case cmd_write_light_24h_timer_table:
            printf(" index=%u, HH:MM:SS = %u:%u:%u, rly_stat=%u\n", \
                    *(unsigned int *)arg2, *(unsigned int *)arg3,*(unsigned int *)arg4, \
                    *(unsigned int *)arg5, *(int *)arg6);
            if( *(unsigned int *)arg2 < relay_time_t_mcuable_sz) {
                prov.rl_tt[*(unsigned int *)arg2].hour = *(unsigned int *)arg3;
                prov.rl_tt[*(unsigned int *)arg2].minutes = *(unsigned int *)arg4;
                prov.rl_tt[*(unsigned int *)arg2].seconds = *(unsigned int *)arg5;
                prov.rl_tt[*(unsigned int *)arg2].relay_status = *(unsigned int *)arg6;
            }
            break;
        case cmd_write_light_24h_timer_table_prov_max:
            printf("prov_tt_max=%u\n",*(uint8_t *)arg2);
            prov.basic_prov.max_rl_tt_provisioned = *(uint8_t *)arg2;
            break;
        case cmd_set_mcu_boot_delay:
            printf(" cmd_set_mcu_boot_delay=%u\n",*(uint8_t *)arg2);
            prov.basic_prov.mcu_boot_delay = *(uint8_t *)arg2;
            break;
        case cmd_write_starter_mode: //cmd*motor_N*mode(0|1) ;mode=> 0=temp, 1=auto
            printf("WRITE_MOTOR_MODE [ starter no = %u ] = %u\n",*(uint8_t *)arg2, *(uint8_t *)arg3);
            prov.motor_prov_obj.starter_mode[*(uint8_t *)arg2] = *(uint8_t *)arg3;
            break;
        case cmd_write_starter_status: //cmd*motor_N*mode(0|1) ;mode=> 0=temp, 1=auto
            printf("WRITE_MOTOR_STATUS=%hhu, %hhu\n",*(uint8_t *)arg2, *(uint8_t *)arg3);
            prov.motor_prov_obj.starter_status[*(uint8_t *)arg2] = *(uint8_t *)arg3;
            break;
        case cmd_write_manual_switch_status_provision: //cmd*motor_N*mode(0|1) ;mode=> 0=temp, 1=auto
            printf("nWRITE_MANUAL_SWITCH_STATUS=%u\n",*(uint8_t *)arg2);
            prov.manual_switch_status[*(uint8_t *)arg2] = *(uint8_t *)arg3;
            break;
#if 0
        case cmd_write_starter_n_call_aware:
            printf("nMOTOR_N_ON_OFF=%u",*(uint8_t *)arg2);
            prov.starter_n_call_aware = *(uint8_t *)arg2;
            break;
#endif
        case cmd_write_device_type:
            printf("cmd_write_device_type=%u\n",*(uint8_t *)arg2);
            if ( is_valid_device_type(*(uint8_t *)arg2)) {
                prov.basic_prov.device_type = *(uint8_t *)arg2;
            }
            break;
        case cmd_write_sms_reply_enabled:
            prov.basic_prov.sms_reply_enabled= *(uint8_t *)arg2;
            printf("nsms_reply_enabled=%u",*(uint8_t *)arg2);
            break;
        case cmd_write_lat_long:
            prov.astro_prov.lattitude = *(double *)arg2;
            prov.astro_prov.longitude = *(double*)arg3;
            printf(" lattitude=%f, longitude=%f\n", prov.astro_prov.lattitude, prov.astro_prov.longitude);
            break;
        case cmd_write_sun_elevation_rise_set:
            /*
             * prov.astro_prov.sunrise_solar_elevation_angle = -0.83;
             * prov.astro_prov.sunset_solar_elevation_angle = -0.83;
             * prov.astro_prov.sunrise_solar_elevation_angle = -6;
             * prov.astro_prov.sunset_solar_elevation_angle = -6; */
            prov.astro_prov.sunrise_solar_elevation_angle = *(double *)arg2;
            prov.astro_prov.sunset_solar_elevation_angle  = *(double *)arg3;
            printf("sun elevation => %f and %f", prov.astro_prov.sunrise_solar_elevation_angle, prov.astro_prov.sunset_solar_elevation_angle);
            break;
        default:
            printf("Error  default  ERR \n" );
    }
    ram_to_dataflash();
    dump_conf();
    printf("Return: store_to_dataflash(%d)\n", arg1);
    return 0;
}

int store_phone_no(int index, const char *phone_no) {
    printf(" store_phone_no (%d, %s)\n", index, phone_no);
    if(index < phone_no_list_sz ){
        printf(" copying");
        prov.phone_book[index].valid = 1;
        strcpy(prov.phone_book[index].phone_no, phone_no);
        printf(" prov.phone_book[%u].valid=%d \n", index, prov.phone_book[index].valid);
        printf(" prov.phone_book[%u].phone_no=%s \n", index,prov.phone_book[index].phone_no);
        //dump_conf();
        ram_to_dataflash();
        //dataflash_to_ram();
        //            //dump_conf();
    }
    return 0;
}

void delete_phone_no_at_index(int index) {
    printf(" delete_phone_no_at_index(%d)\n", index);
    if(index < phone_no_list_sz ) {
        printf(" deleting\n");
        prov.phone_book[index].valid = 0;
        ram_to_dataflash();
        dataflash_to_ram();
        dump_conf();
    }
}

int get_security_byte(void) {
    return prov.basic_prov.security;
}

void ram_to_dataflash(void) {
    printf(" enter ram_to_dataflash \n");
#if MACHINE == RL78
    R_WDT_Restart();
    R_FDL_Erase(0);
    R_FDL_Erase(1);
    R_FDL_Erase(2);
    R_FDL_Erase(3);
    prov.basic_prov.written_n++;
    R_FDL_Write(0, (uint8_t *)&prov.basic_prov.written_n, sizeof(prov));
#endif
    R_WDT_Restart();
     printf(" return ram_to_dataflash\n ");
}

void dataflash_to_ram(void) {
       printf(" enter dataflash_to_ram()\n ");
    R_WDT_Restart();
    //index = offsetof(prov_t, basic_prov.device_type);
#if MACHINE == RL78
    R_FDL_Read (0, (uint8_t *)&prov.basic_prov.written_n, sizeof(prov));
#endif
    R_WDT_Restart();
    printf(" return dataflash_to_ram()\n");
}
