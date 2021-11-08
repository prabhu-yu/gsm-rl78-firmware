#include "0_all.h"

int sms_parse( char *ptr_cmgl, int *ptr_index, char address[],
               int *ptr_yy,int *ptr_MM,int *dd,int *hh,int *mm,int *ss,int *tz,
               char received_sms_body[]) {
    char message_status[20];
    int ret_val;
    char  address_text[20], service_center_time_stamp[20];
    printf("Enter: sms_parse()\n");

#define format_index "%d,"
#define format_message_status "\"%[ A-Z]\","
#define format_address "\"%[-+_0-9A-Za-z]\","
#define format_address_text "%[-\"+_0-9A-Za-z],"
#define format_service_center_time_stamp "\"%[-/,+_0-9+:]\""
#define format_address_type "%[0-9],"
#define format_received_sms_body_length "%[0-9],"
//#define format_received_sms_body "%[\a-~]"
//#define format_received_sms_body "%[-\"\\+*%/_ 0-9A-Za-z]"
#define format_received_sms_body "%[\x20-\x7F]"
    // this is for SENT message
    ret_val = sscanf(ptr_cmgl,"+CMGL:"format_index format_message_status, ptr_index, message_status);
    if(2 != ret_val ) {
        printf(" invalid message; Return\n");
        return 2;
    }

    printf(" index = %d \n", *ptr_index);
    printf(" msg_status = %s \n", message_status);

    if (NULL != strstr(message_status, "SENT")) {
        printf(" this is a SENT message, so will delete this\n");
        sms_delete_index_based(*ptr_index);
        printf("Return\n");
        return 1;
    }

    if (NULL == strstr(message_status, "READ")) {
        printf("this is a not a READ msg.. unknown string. Returning\n");
        return 4;
    } else {
        printf("this is a received message, so will delete this, again parse (not reading)\n");
        printf("scanf1 of >>>%s<<<\n", ptr_cmgl);
        ret_val = sscanf(ptr_cmgl,"+CMGL:" format_index format_message_status format_address format_address_text format_service_center_time_stamp CRLF format_received_sms_body CRLF,
                ptr_index, message_status, address, address_text, service_center_time_stamp, received_sms_body);
        printf(" sscanf : ret_val=%d\n", ret_val);
        sms_delete_index_based(*ptr_index);
        if(ret_val == 6) {
            printf("good received sms\n");
            printf(" index  = %d\n", *ptr_index);
            printf(" address = %s\n", address);
            printf(" address_text = %s\n", address_text);
            printf(" service_center_time_stamp = %s\n", service_center_time_stamp);
            printf(" received_sms_body = %s\n", received_sms_body);
            //printf("\n address_type = %s", address_type); <address_type> and <body_length> are shown if AT+CSDH=1.
            //printf("\n received_sms_body_length = %s", received_sms_body_length);
        } else {
            printf("corrupted sms listing; Return\n");
            return 3;
        }
    }
    printf(" Return : sms_parse()\n");
    return 0;
}

void issue_CMGDA(void) {
    printf("Enter: issue_CMGDA()\n");
    reset_at_rx_buf();
    //printf("\r\nAT+CMGD=0,4\r"); /* delete all sms. This is safty case, what if sms remained... */
    write_to_modem("\rAT+CMGDA=\"DEL ALL\"\r"); /* delete all sms. This is safty case, what if sms remained... */
    R_WDT_Restart();sleep_ms(2000);R_WDT_Restart();
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    reset_at_rx_buf();
    printf(" deleted the all sms, verifying now\n");
    issue_CPMS_Q();
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    printf("Return: issue_CMGDA()\n");
}

void sms_delete_all(void) {
    printf("Enter: sms_delete_all()\n");
    int used_sms, temp;
    char *ptr;
#define NEEDLE_ME  "+CPMS: \"ME\","
#define NEEDLE_SM  "+CPMS: \"SM\","
     printf(" >>> sms_delete_all\n");
    //TODO: query for the outstadning SMS, if there is any, call delete all SMS.
    reset_at_rx_buf();
    //issue_CPMS_EQ_Q();
    issue_CPMS_Q();
    R_WDT_Restart();sleep_ms(1000);
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    // at_rx_buf:.AT+CPMS?.._+CPMS: "ME",1,50,"ME",1,50,"ME",1,50\r\n\r\nOK\r\n
    ptr = strstr(TYPE_CONST_CHAR_P at_rx_buf, NEEDLE_ME);
    if(NULL == ptr) {
        printf(" it is not ME\n");
        ptr = strstr(TYPE_CONST_CHAR_P at_rx_buf, NEEDLE_SM);
    }
    if (NULL != ptr) {
        //printf("\r%s", ptr);
        temp = sscanf(ptr, NEEDLE_ME"%d", &used_sms);
        if(1 != temp) {
            temp = sscanf(ptr, NEEDLE_SM"%d", &used_sms);
        }

        if(1 == temp ) {
            if(0 != used_sms){
                printf(" clear SMS Db \n");
                issue_CMGDA();
            } else {
                printf(" No need of deleting\n");
            }
        } else {
            printf(" err in parsing, temp=%d\n", temp);
        }
    } else {
        printf(" Not found string\n");
    }
    printf("Return :sms_delete_all()\n");
}

int parse_cmgw_index_timeout(void  *buf, unsigned int timeout) {
    int index;
    char *ptr;
    unsigned int my_slept_time;
    printf(" parse_cmgw_index_timeout() \n");
#define MY_SLEEP_INTERVAL 1000
    for( my_slept_time=0; my_slept_time < timeout; my_slept_time += MY_SLEEP_INTERVAL) {
        R_WDT_Restart();sleep_ms(MY_SLEEP_INTERVAL);
        ptr = strstr((void  *)buf, "+CMGW:");dump_at_rx_buf();
        if (ptr) {
            sscanf(ptr, "+CMGW:%d", &index);
            printf(" index=%d\n", index);
            R_WDT_Restart();sleep_ms(MY_SLEEP_INTERVAL);// Let the OK gets printed...
            break;
        }else{
            ptr = strstr((void  *)buf, "ERROR");dump_at_rx_buf();
            if (ptr){
                printf(" Error in CMGW\n");
                index = -1;
                break;
            }
        }
    }
    return index;
}

/* TODO add string length strictly.
 *  https://www.developershome.com/sms/cmgwCommand2.asp
 * */
//TODO: improve this function, remove delays as much as possible... make them deterministic..
int sms_write_to_modem( char *mob_no, char *sms ) {
    unsigned int slept_time;
    int index = 0;
    unsigned int str_lenght;
    printf("Enter: sms_write_to_modem (mob_no=%s)\n", mob_no);
    if(!prov.basic_prov.sms_reply_enabled) {
        printf("Return :>>> sms_write_to_modem since !sms_reply_enabled\n");
        return 0;
    }
    str_lenght = strlen(sms);
    if(str_lenght > 160) {
        printf("too long sms\n");
        return 0;
    }
    reset_at_rx_buf();
    write_to_modem("\rAT+CMGW\r");
#define MY_SLEEP_TIME 100
    char buffer[130];
    for (slept_time=0; slept_time < 29000; slept_time+= MY_SLEEP_TIME) {
        printf(" slept_time=%d\n", slept_time);
        R_WDT_Restart();sleep_ms(MY_SLEEP_TIME);
        if( NULL != strstr(TYPE_CONST_CHAR_P at_rx_buf, "\r\n> ")) {
            printf("\nGot rn>space\n");
            snprintf(buffer, sizeof(buffer), "\r%s\r\x1A\r", sms);
            write_to_modem(buffer);
            printf("wrote SMS\n");
            printf("writing the control + z");
            snprintf(buffer, sizeof(buffer), "\r\x1A\r");
            write_to_modem(buffer);

            index = parse_cmgw_index_timeout(TYPE_VOID_P at_rx_buf, 29000);
            printf("New cmgw index = %d\n",index);
            if(index < 0 ) {
                printf("Error in CMGW so , will not send sms\n");
                break;
            }
            reset_at_rx_buf();
            snprintf(buffer, sizeof(buffer), "\rAT+CMSS=%d,\"%s\"\r",index, mob_no);
            write_to_modem(buffer);
            parse_cmss_timeout( TYPE_VOID_P at_rx_buf, 60*2);R_WDT_Restart();
            break;
        } else {
             printf("Not got >\n");
        }
    }
    printf("Return: sms_write_to_modem()\n");
    return index;
}

void broadcast_sms(char *sms) {
    int i;
    char *mob_no;
#if 0
    char temp_str_100[100];
    printf("Enter: broadcast_sms( %s )\n", sms);
    snprintf(temp_str_100, sizeof(temp_str_100), "PT:%04u:%02u:%02u T %02u:%02u:%02u\n",
             oper.time_now.year, oper.time_now.month, oper.time_now.month_day,
             oper.time_now.hour, oper.time_now.minutes, oper.time_now.seconds);
    printf("temp_str_100 : %s", temp_str_100);
    strncat(send_sms_body_160, temp_str_100, sizeof(temp_str_100));
#endif
#if 0
    i=strlen(sms);
    while(i--) {
        sleep_ms(5);R_WDT_Restart();
        if ('\r' == sms[i]) {
            printf("<R>");continue;
        }
        if ('\n' == sms[i]) {
            printf("<N>");continue;
        }
        printf("%c",sms[i]);
    }
#endif
    for(i=0; i < phone_no_list_sz; i++) {
        mob_no = get_stored_ph_number(i);
        if(NULL != mob_no) {
            printf("write sms:%s\n", mob_no);
            sms_write_to_modem( mob_no, sms);
        } else {
            // printf("\r NO write sms");
        }
    }
    send_sms_flag = false;
    printf("Return : broadcast_sms\n");
}

void reset_send_sms_body_160(void) {
    printf("enter: reset_send_sms_body_160\n");
    memset(send_sms_body_160, 0, sizeof(send_sms_body_160));
}

void issue_CPMS_Q(void) {
    write_to_modem("\rAT+CPMS?\r");
}

void issue_CPMS_EQ_Q(void) {
    write_to_modem("\rAT+CPMS\r");
}

void sms_delete_index_based(int index) {
    char buffer[20];
    int ret_val;
    printf("enter: sms_delete_index_based(%d)\n", index);
    ret_val = snprintf(buffer, sizeof(buffer), "\rAT+CMGD=%d,0\r", index);
    reset_at_rx_buf();
    write_to_modem(buffer);
   	look_for_ok_resp_timeout((char *)at_rx_buf, 4000);
    printf("return: sms_delete_index_based\n");
}

void issue_cmgl_all(void) {
    printf(" Enter: issue_cmgl_all() \n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CMGL=\"ALL\"\r");
    /* delete all sms. This is safty case, what if sms remained... */
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 10000);
    //dump_at_rx_buf();
    printf("Return: issue_cmgl_all()\n");
}

/*retrun 0 if all is ok
 * return non zero +ve number  if there is an error
 * */
int parse_cmss_timeout(void *buf, unsigned int timeout_seconds ) {
    unsigned int i;
    char *ptr;
    int sleep_delay_1000_ms = 1000;
    printf("parse_cmss_timeout %u seconds\n", timeout_seconds);
    for(i=0; i < timeout_seconds ; i++) {
        printf("i=%d\n", i);
        R_WDT_Restart(); sleep_ms(sleep_delay_1000_ms);
        ptr = strstr((void  *)buf, "+CMSS:");dump_at_rx_buf();
        if (ptr){
            printf("got CMSS replay\n");
            ptr = strstr((void  *)buf, "OK");dump_at_rx_buf();
            if(ptr) {
                printf("got CMSS replay AND OK; SMS sent!\n");
                return 0;
            }
        } else {
            ptr = strstr((void  *)buf, "ERROR");
            dump_at_rx_buf();
            if (ptr){
                printf("Err in CMSS\n");
                R_WDT_Restart(); sleep_ms(500); // simply wiat to clear remaing buffer
                dump_at_rx_buf();
                return 1;
            }
        }
    }
    return 2;
}

// return 1 if things go bad!
unsigned int  parse_3_args_unsigned_ints(char *received_sms_body,
    unsigned int *temp2_uint, unsigned int *temp3_uint, char* phone_no) {
    printf("Enter: parse_3_args_unsigned_ints()\n");
    send_sms_flag = true;
    int temp_result = sscanf(received_sms_body, CMD_PRE"%*u*%u*%u", temp2_uint, temp3_uint);
    if(2 != temp_result) {
        printf("MSG is wrong 34523123=%d\n", temp_result);
        snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                "\nNOT ENOUGH ARGS\nBy %s", phone_no);
        return 1;
    }
    printf("arg0 = X, arg1 = %u, arg2 = %u\n", *temp2_uint, *temp3_uint);
    printf("Return  parse_3_args_unsigned_ints()");
    return 0;
}
int process_cmgl_output(void ) {
/* +CMGL: index,message_status,phone_no,[phone_no_text],[service_center_time_stamp][,address_type,received_sms_body_length]<CR><LF>received_sms_body[<CR><LF>+CMGL: ...]
AT+CMGL="all"
"+CMGL: 1,"REC READ","DEBUG_NUMBER","","21/01/25,16:03:45+22"
Hi 124
OK*/
    int index=0;
    int temp1_int, temp2_int;
    //text_command_t cmd;
    unsigned int temp2_uint, temp3_uint, temp4_uint, temp5_uint, temp6_uint;
    double temp1_double, temp2_double, temp3_double;
    char message_status[10];
    char phone_no[20];
    char temp_str_100[100];
    int yy,MM,dd,hh,mm,ss,tz;
    char received_sms_body[160];
    int cmd;
    char *ptr_cmgl;
    int ret_val;
    int temp_result;
    int i;
    char* local_at_rx_buf = TYPE_CHAR_P at_rx_buf;
    printf("Enter: process_cmgl_output()\n");
#if 0
    ptr_cmgl = strstr(at_rx_buf, "+CMTI: \"SM\",");
    if(NULL == ptr_cmgl){
         printf("\n no sms present");
        return 0; /* no more sms */
    }

    index = 0;
    ret_val = sscanf(ptr_cmgl,"+CMTI: \"SM\",%d", &index);
    if(1 == ret_val){
        if(0 != index) {
            sms_list_cmgr(index);
            sms_delete_index_based(index);
        } else {
            printf("\n no index is zero");
            return 0;
        }
    } else {
        printf("\n could not parse CMTI");
        return 0;
    }
#endif

    // TODO : need to add a loop so that all sms will be parsed in a single shot.
    ptr_cmgl = strstr(local_at_rx_buf, STRING_CMGL);
    if(NULL == ptr_cmgl){
        printf(" return: process_cmgl_output(): NO SMS present \n");
        return 0; /* no more sms */
    } else {
        local_at_rx_buf += (sizeof(STRING_CMGL) - 1); // in next iteration, we need to go beyond of this sms
    }
    dump_at_rx_buf();

    ret_val = sms_parse( ptr_cmgl, &index, phone_no, &yy, &MM, &dd, &hh ,&mm, &ss, &tz, received_sms_body );

    if( 0 != ret_val ) {
        printf("sms parsing failed\n");
        return 2;
    }

     if ( (FALSE == is_it_stored_phone_no(phone_no)) &&
          (NULL == strstr(phone_no, DEBUG_NUMBER)) &&
          (NULL == strstr(phone_no, DEBUG_NUMBER))
        ) {
         printf("sms:unknown PH_NO\n");
         return 3;
    } else {
        printf("sms: KNOWN PH_NO\n");
    }

    cmd = cmd_max;
    temp_result = sscanf(received_sms_body, CMD_PRE"%d", &cmd);
     printf("temp_result=%d, cmd=%d\n", temp_result, cmd );

    if( (EOF == temp_result) || (1 != temp_result) ) {
         printf("could not parse sms message body (not header)\n");
        return 4; // wrong command
    }

    if( 0 == is_valid_text_cmd(cmd)){
        printf("wrong command\n");
        return 5;
    }

    switch(cmd) {
    case cmd_write_add_phone_number:
         printf("ADD_USER\n");
        temp_result = sscanf(received_sms_body,CMD_PRE"%d*%d*%s", &cmd, &temp1_int, temp_str_100);
        if (3 == temp_result) {
             printf("index=%d, ph no=%s\n", temp1_int, temp_str_100);
            store_phone_no(temp1_int, temp_str_100);
            snprintf(send_sms_body_160, sizeof(send_sms_body_160), "WRITE: ADD USER%d\nNUMBER:%s\nBY:%s\n", temp1_int, temp_str_100, phone_no);
            send_sms_flag = true;
        } else {
             printf("invalid store msg \n");
        }
        break;
    case cmd_write_delete_phone_number:
         printf("DELETE_USER\n");
        temp_result = sscanf(received_sms_body, CMD_PRE"%d*%d", &temp1_int, &temp2_int);
        if(2 == temp_result) {
             printf("cmd=%d,user idx=%d\n", temp1_int, temp2_int);
            delete_phone_no_at_index(temp2_int);
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write: DELETE USER-%d\nBY: %s", temp2_int, phone_no);
            send_sms_flag = true;
        }
        break;
    case cmd_get_status:
        printf("send me the present status mode = %hhu\n", prov.basic_prov.device_type);
        if (water_sensor_tanks_1 == prov.basic_prov.device_type){
            printf("111 %s","\n GET status of MOBIL_LEVEL_CONTROLLER\n");
            send_sms_body_160[0]='\0';
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "READ: STATUS\nMODE: %s\n", device_type_str((device_type_t) prov.basic_prov.device_type));
            printf("send_sms_body_160: zxs %s\n",send_sms_body_160);

            unsigned int percent = 100*oper.current_water_level/prov.water_level_prov.max_level;
            char level_str_50[50];
            if(percent == 0) {
                strncat(send_sms_body_160, "EMPTY TANK\n", sizeof(level_str_50));
            } else if(percent == 100) {
                strncat(send_sms_body_160, "FULL TANK\n", sizeof(level_str_50));
            } else {
                strncat(send_sms_body_160, "HALF TANK\n", sizeof(level_str_50));
            }
            printf("send_sms_body_160 123=%s", send_sms_body_160);

            ret_val = snprintf(temp_str_100, sizeof(temp_str_100),\
                    "LEVEL : %d%% \nPresentSensor/TotalSensor: %2d/%2d\nTIME: %s\nBY:%s", \
                    100*oper.current_water_level/prov.water_level_prov.max_level,\
                    oper.current_water_level,\
                    prov.water_level_prov.max_level,\
                    oper.time_now.str_time,\
                    phone_no);
            if(ret_val < 0 ) {
                printf("ERROR 234234\n");
            }
            printf("444.1 temp_str_100 = %s\n", temp_str_100);
            strncat(send_sms_body_160, temp_str_100, sizeof(temp_str_100));
            printf("send_sms_body_160 sdfs: %s\n",send_sms_body_160);
        } else {
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                "STATUS\nmode:%s\nRelay1:%s\nTIME:%s\nBY:%s",
                device_type_str((device_type_t)prov.basic_prov.device_type),
                oper.relay_status[0]?"ON":"OFF", oper.time_now.str_time,
                             phone_no);
        }
        send_sms_flag = true;
        break;
        /* cmd_read */
    case cmd_read_phone_numbers:
        printf("GET_PHONE_BOOK\n");
        strcat(send_sms_body_160, "USER_NUM\n");
        for(i=0; i < phone_no_list_sz; i++){
            if(1 == prov.phone_book[i].valid){
                snprintf(temp_str_100, sizeof(temp_str_100),
                		"%u,%s\n", i, prov.phone_book[i].phone_no);
            }else{
                snprintf(temp_str_100, sizeof(temp_str_100), "%d,NO_PHONE\n", i);
            }
            strncat(send_sms_body_160, temp_str_100, sizeof(temp_str_100));
        }
        printf("send_sms_body_160 = %s\n", send_sms_body_160);
        send_sms_flag = true;
        break;
    case cmd_read_astro_prov:
        printf("cmd_read_astro_prov\n");
        strcat(send_sms_body_160, "astro_prov\n");
        snprintf(temp_str_100, sizeof(temp_str_100), "LAT: %f\nLONG: %f\nSUNRISE_SE: %f\nSUNSET_SE: %f\n",
                prov.astro_prov.lattitude,
                prov.astro_prov.longitude,
                prov.astro_prov.sunrise_solar_elevation_angle,
                prov.astro_prov.sunset_solar_elevation_angle);
        strncat(send_sms_body_160, temp_str_100, sizeof(temp_str_100));
        append_on_off_time(&prov, &oper, send_sms_body_160);
        printf("send_sms_body_160 = %s\n", send_sms_body_160);
        send_sms_flag = true;
        break;

    case cmd_read_light_24h_timer_table:
        //TIME_TBL\nhh:mm:ss,1_0\nhh:mm:ss,1_0  ...this repeats 10 times
         printf("GET_TT\n");
        strcat(send_sms_body_160, "TIME_TBL\n");
        for(i=0; i < relay_time_t_mcuable_sz; i++){
            printf("TT[%u]: %02u-%02u-%02u RL=%u\n", i, prov.rl_tt[i].hour, prov.rl_tt[i].minutes, prov.rl_tt[i].seconds, prov.rl_tt[i].relay_status);
            snprintf(temp_str_100, sizeof(temp_str_100), "%02u:%02u:%02u,%01u\n",prov.rl_tt[i].hour, prov.rl_tt[i].minutes, prov.rl_tt[i].seconds, prov.rl_tt[i].relay_status);
            strncat(send_sms_body_160, temp_str_100, sizeof(temp_str_100));
        }
        send_sms_flag = true;
        snprintf(temp_str_100, sizeof(temp_str_100), "max_tt = %02u\n", prov.basic_prov.max_rl_tt_provisioned);
        strncat(send_sms_body_160, temp_str_100, sizeof(temp_str_100));
        break;
    case cmd_write_light_24h_timer_table:
         printf("SET_FIXED_TIMER_TT\n");
        // cmd*index*hh*mm*ss*rly_status; example
        // 17*0*6*55*5*0  ==>  stored at index 0, turns off at 6:17:44
        // 17*1*1*18*50*1  ==>  stored at index 1, turns on at 6:17:44
        temp_result = sscanf(received_sms_body, CMD_PRE"%*u*%u*%u*%u*%u*%u",
            &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint, &temp6_uint);
        printf("cmd=%u, index=%u, hh:mm:ss=%u:%u:%u rly=%u\n",
                cmd, temp2_uint, temp3_uint, temp4_uint, temp5_uint, temp6_uint);
        store_to_dataflash(cmd, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint , &temp6_uint);
        snprintf(send_sms_body_160, sizeof(send_sms_body_160), "TimeTable UPDATE\nBy %s", phone_no);
        send_sms_flag = true;
        time_t_mcuable_changed = true;
        reset_time_t_mcuable_sent_sms();
        break;
    case cmd_write_light_24h_timer_table_prov_max:
         printf("cmd_write_light_24h_timer_table_prov_max\n");
        // cmd*timer_tt_prov_max ex:  25*2
        temp_result = sscanf(received_sms_body, CMD_PRE"%*u*%u", &temp2_uint);
         printf("cmd=%u, max=%u %u %u %u %u\n", cmd, temp2_uint, temp3_uint, temp4_uint, temp5_uint, temp6_uint);
        store_to_dataflash(cmd, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint , &temp6_uint);
        snprintf(send_sms_body_160, sizeof(send_sms_body_160), "tt_prov_max \nBy %s", phone_no);
        send_sms_flag = true;
        time_t_mcuable_changed = true;
        reset_time_t_mcuable_sent_sms();
        break;

/* cmd_write 1,2,3 args */
// 1 arg, but no data to be stored
    case cmd_reset_mcu:
         printf("SET_RESET_MCU\n");
        reset_mcu(0);
        break;
    case cmd_reset_prov:
         printf("cmd_reset_prov\n");
        //provision_to_defaults();
        break;
    case cmd_read_sw_hw_rev:
    	 printf("cmd_read_sw_hw_rev\n");
        snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                "\nHW:R1.2\nSW:1.1\nDT:2021-01-11\nBy:%s", phone_no);
    	break;
//###########################################################
// 2 args, both are unsigned integers
    case cmd_write_starter_n_call_aware: // cmd*motor_N
    case cmd_set_mcu_boot_delay:
    case cmd_write_device_type: // cmd*device_type
    case cmd_write_water_level_soak_time:
    case cmd_write_water_level_max_level:
    case cmd_write_sms_reply_enabled:
         printf(" 2 args processing\n");
        send_sms_flag = true;
        //temp_result = sscanf(received_sms_body, CMD_PRE"%u*%u", &cmd, &temp2_uint);
        temp_result = sscanf(received_sms_body, CMD_PRE"%*u*%u", &temp2_uint);
        if(1 != temp_result){
            printf("MSG is wrong 113121 ARGS=%d\n", temp_result);
            snprintf(send_sms_body_160, sizeof(send_sms_body_160), "NOT ENOUGH ARGS\nBy: %s", phone_no);
            break;
        }
        printf(" cmd=%u, param=%u\n", cmd, temp2_uint);
        store_to_dataflash(cmd, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint, &temp6_uint);
        switch(cmd) {
        case cmd_set_mcu_boot_delay:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write: set_mcu_boot_delay=%u\nBy: %s", temp2_uint, phone_no);
            break;
        case cmd_write_starter_n_call_aware:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write: write_starter_n_call_aware=%u\nBy: %s", temp2_uint, phone_no);
            break;
        case cmd_write_device_type:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "WRITE: DEVICE TYPE:%s\nBY: %s",
                    device_type_str((device_type_t) temp2_uint), phone_no);
            init_oper_data1(new_text_command_received);
            break;
        case cmd_write_water_level_soak_time:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write: Water soak time\nBy: %s", phone_no);
            break;
        case cmd_write_water_level_max_level:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write: water_level_max_level\nBy: %s", phone_no);
            break;
        case cmd_write_sms_reply_enabled:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write:sms_reply_enabled\nBy: %s", phone_no);
            break;
        default:
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "write: error\nBy: %s", phone_no);
            break;
        }
        break;
//###########################################################
// 3 args: all are unsigned ints
    case cmd_write_starter_mode: //cmd*motor_N*mode(0|1) ;mode=> 0=temp, 1=auto
        printf("cmd_write_starter_mode\n");
        if (parse_3_args_unsigned_ints(received_sms_body,  &temp2_uint, &temp3_uint, phone_no) ){
            break;
        }
        printf("Starter cmd=%u, starter_no=%u, mode=%u\n", cmd, temp2_uint, temp3_uint);
        handle_starter_mode(new_text_command_received, cmd_write_starter_mode, temp2_uint, temp3_uint, phone_no);
        break;
    case cmd_write_starter_status: //cmd*motor_N*status ;  0 or 1
        printf("cmd_write_starter_status\n");
        if (parse_3_args_unsigned_ints(received_sms_body,  &temp2_uint, &temp3_uint, phone_no) ){
            break;
        }
        printf("Starter cmd=%u, starter_no=%u, starter_status=%u\n", cmd, temp2_uint, temp3_uint);
        switch(prov.basic_prov.device_type) {
        case motor_starter:
            switch(prov.motor_prov_obj.starter_mode[0]) {
            case starter_mode_automatic:
                handle_starter_status_mode_automatic(new_text_command_received, cmd, temp2_uint, temp3_uint, phone_no);
                break;
            case starter_mode_temporary:
                handle_starter_status_temporary(new_text_command_received, cmd, temp2_uint, temp3_uint, phone_no);
                break;
            default:
                strncat(send_sms_body_160, "ERROR: STARTER MODE\n", MY_SMS_LENGHT);
                break;
            }
            break;
        }
    case cmd_write_starter_start_stop_relay_hold_time:
        if (parse_3_args_unsigned_ints(received_sms_body,  &temp2_uint, &temp3_uint, phone_no) ){
            break;
        }
        printf("relay_hold_time cmd=%u, start= %u, stop= %u\n", cmd, temp2_uint, temp3_uint);
        store_to_dataflash(cmd, &temp2_uint, &temp3_uint, NULL, NULL, NULL);
        snprintf(temp_str_100, sizeof(temp_str_100), "write:start/stop relay time\nBy: %s", phone_no);
        append_to_sms_memory(temp_str_100);
        break;
    case cmd_write_manual_switch_status_provision:
        if (parse_3_args_unsigned_ints(received_sms_body,  &temp2_uint, &temp3_uint, phone_no) ){
            break;
        }
        printf(" Starter cmd=%u, %u, %u\n", cmd, temp2_uint, temp3_uint);
        store_to_dataflash(cmd, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint , &temp6_uint);
        handle_manual_switch(new_text_command_received);
        snprintf(send_sms_body_160, sizeof(send_sms_body_160), "An UPDATE BY By %s\n", phone_no);
        break;
// 3 args: the last two are floats.
    case cmd_write_lat_long:
    case cmd_write_sun_elevation_rise_set:
        send_sms_flag = true;
        temp_result = sscanf(received_sms_body, CMD_PRE"%u*%lf*%lf", &cmd, &temp2_double, &temp3_double);
        if(3 != temp_result){
            printf("MSG is wrong 345236=%d\n", temp_result);
            snprintf(send_sms_body_160, sizeof(send_sms_body_160),
                    "NOT ENOUGH ARGS\nBy %s", phone_no);
            break;
        }
        printf(" astro params cmd=%u, %f, %f\n", cmd, temp2_double, temp3_double);
        store_to_dataflash(cmd, &temp2_double, &temp3_double, NULL, NULL, NULL);
        snprintf(send_sms_body_160, sizeof(send_sms_body_160), "WRITE for lat/long/SA_rise/set BY %s\n", phone_no);
        break;
    default:
         printf(" WRONG SMS\n");
        break;
    }
    printf(" Return : process_cmgl_output()\n");
    return 1; // successfully parsed at least one sms
}
