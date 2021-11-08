#include "0_all.h"

int sms_parse(
        char *ptr_cmgl,
        int *ptr_index,
        char message_status[],
        char address[],
        int *ptr_yy,int *ptr_MM,int *dd,int *hh,int *mm,int *ss,int *tz,
        char sms_message_body[]
        )
{
    int ret_val;
    char  address_text[20], service_center_time_stamp[20];
    printf_uart_0_1 = UART_DEBUG;
    printf("\nEnter: sms_parse()");

#define format_index "%d,"
#define format_message_status "\"%[ A-Z]\","
#define format_address "\"%[-+_0-9A-Za-z]\","
#define format_address_text "%[-\"+_0-9A-Za-z],"
#define format_service_center_time_stamp "\"%[-/,+_0-9+:]\""
#define format_address_type "%[0-9],"
#define format_sms_message_body_length "%[0-9],"
//#define format_sms_message_body "%[\a-~]"
//#define format_sms_message_body "%[-\"\\+*%/_ 0-9A-Za-z]"
#define format_sms_message_body "%[\x20-\x7F]"
#define CRLF "\r\n"
    // this is for SENT message
    ret_val = sscanf(ptr_cmgl,"+CMGL:"
            format_index
            format_message_status
            ,
            ptr_index,
            message_status);

    if(2 != ret_val ) {
        printf("\n invalid message");
        return 2;
    }
    
    printf("\n index = %d", *ptr_index);
    printf("\n msg_status = %s", message_status);

    if (NULL != strstr(message_status, "SENT")) {
        printf("\r this is a SENT message, so will delete this");
        sms_delete_index_based(*ptr_index);
        return 1;
    }

    if (NULL == strstr(message_status, "READ")) {
        printf("\r this is a not a READ msg.. unknown string.");
        return 4;
    } else {
        printf("\r this is a received message, so will delete this, again parse (not reading)");
        printf("\r scanf1 of >>>%s<<<", ptr_cmgl);
        ret_val = sscanf(ptr_cmgl,"+CMGL:"\
            format_index
            format_message_status
            format_address
            format_address_text
            format_service_center_time_stamp
            CRLF
            format_sms_message_body
            CRLF,
            ptr_index,
            message_status,
            address,
            address_text,
            service_center_time_stamp,
            sms_message_body
            );
        sms_delete_index_based(*ptr_index);
        printf("\r  after : scanf ..");
        printf("\r ret_val=%d", ret_val);
        if(ret_val == 6) {
            printf("good received sms");
        } else {
            printf("corrupted sms listing");
            return 3;
        }

        printf("\n address = %s", address);
        printf("\n address_text = %s", address_text);
        printf("\n service_center_time_stamp = %s", service_center_time_stamp);
        //printf("\n address_type = %s", address_type);
        //printf("\n sms_message_body_length = %s", sms_message_body_length);
        printf("\n sms_message_body = %s", sms_message_body);
    }
    return 0;
}

void issue_CMGDA(void)
{
    printf_uart_0_1 = UART_DEBUG;
    printf("\r--->issue_CMGDA");

    reset_at_rx_buf();
    printf_uart_0_1 = UART_GSM;
    //printf("\r\nAT+CMGD=0,4\r"); /* delete all sms. This is safty case, what if sms remained... */

    printf("\rAT+CMGDA=\"DEL ALL\"\r"); /* delete all sms. This is safty case, what if sms remained... */
    R_WDT_Restart();sleep_ms_a(2000);R_WDT_Restart();
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    reset_at_rx_buf();

    printf_uart_0_1 = UART_DEBUG;
    printf(" deleted the all sms, verifying now");

    issue_CPMS_Q();
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
}

void sms_delete_all(void)
{
    int used_sms, temp;
    char *ptr;
#define NEEDLE_ME  "+CPMS: \"ME\","
#define NEEDLE_SM  "+CPMS: \"SM\","

    printf_uart_0_1 = UART_DEBUG;
    printf("\r>>>>>>>>>sms_delete_all");
    //TODO: query for the outstadning SMS, if there is any, call delete all SMS.
    reset_at_rx_buf();
    //issue_CPMS_EQ_Q();
    issue_CPMS_Q();
    R_WDT_Restart();sleep_ms_a(1000);
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    // at_rx_buf:.AT+CPMS?.._+CPMS: "ME",1,50,"ME",1,50,"ME",1,50\r\n\r\nOK\r\n
    ptr = strstr(TYPE_CONST_CHAR_P at_rx_buf, NEEDLE_ME);
    if(NULL == ptr) {
        printf_uart_0_1 = UART_DEBUG;printf("\r it is not ME");
        ptr = strstr(TYPE_CONST_CHAR_P at_rx_buf, NEEDLE_SM);
    }
    if (NULL != ptr) {
        //printf_uart_0_1 = UART_DEBUG;printf("\r%s", ptr);
        temp = sscanf(ptr, NEEDLE_ME"%d", &used_sms);
        if(1 != temp) {
            temp = sscanf(ptr, NEEDLE_SM"%d", &used_sms);
        }

        if(1 == temp ) {
            if(0 != used_sms){
                printf_uart_0_1 = UART_DEBUG;printf("clear SMS Db ");
                issue_CMGDA();
            } else {
                printf_uart_0_1 = UART_DEBUG;printf("\r No need of deleting");
            }
        } else {
            printf_uart_0_1 = UART_DEBUG;printf("\r err in parsing, temp=%d", temp);
        }
    } else {
        printf_uart_0_1 = UART_DEBUG;printf("\r Not found string");
    }
    printf_uart_0_1 = UART_DEBUG;printf("\r<---sms_delete_all");
}

int parse_cmgw_index_timeout(void  *buf, unsigned int timeout)
{
    int index;
    char *ptr;
    unsigned int my_slept_time;
    printf_uart_0_1 = UART_DEBUG; printf("\r parse_cmgw_index");
#define MY_SLEEP_INTERVAL 100
    for(my_slept_time=0; my_slept_time < timeout; my_slept_time += MY_SLEEP_INTERVAL){
        R_WDT_Restart();sleep_ms_a(MY_SLEEP_INTERVAL);
        ptr = strstr((void  *)buf, "+CMGW:");dump_at_rx_buf();
        if (ptr){
            sscanf(ptr, "+CMGW:%d", &index);
            printf_uart_0_1 = UART_DEBUG; printf("\r index=%d", index);
            R_WDT_Restart();sleep_ms_a(MY_SLEEP_INTERVAL);// Let the OK gets printed...
            break;
        }else{
            ptr = strstr((void  *)buf, "ERROR");dump_at_rx_buf();
            if (ptr){
                printf_uart_0_1 = UART_DEBUG; printf("\r Error in CMGW");
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
int sms_write_to_modem( char *mob_no, char *sms )
{
    unsigned int slept_time;
    int index = 0;
    unsigned int str_lenght;
    printf_uart_0_1 = UART_DEBUG; printf("\r sms_write_to_modem (mob_no=%s)", mob_no);
    if(!prov.basic_prov.sms_reply_enabled) {
        printf("\r Return :>>> sms_write_to_modem since !sms_reply_enabled");
        return 0;
    }
    str_lenght = strlen(sms);
    if(str_lenght > 160) {
        printf_uart_0_1 = UART_DEBUG; printf("\r too long sms");
        return 0;
    }
    reset_at_rx_buf();
    printf_uart_0_1 = UART_GSM; printf("\rAT+CMGW\r");
#define MY_SLEEP_TIME 100
    for (slept_time=0; slept_time < 29000; slept_time+= MY_SLEEP_TIME) {
        printf_uart_0_1 = UART_DEBUG; printf("\r slept_time=%d", slept_time);
        R_WDT_Restart();sleep_ms_a(MY_SLEEP_TIME);
        if( NULL != strstr(TYPE_CONST_CHAR_P at_rx_buf, "\r\n> ")) {
            printf_uart_0_1 = UART_DEBUG; printf("\r Got rn>space");
            printf_uart_0_1 = UART_GSM; printf("%s\r\x1A", sms);
            printf_uart_0_1 = UART_DEBUG; printf("\r wrote SMS");
            index = parse_cmgw_index_timeout(TYPE_VOID_P at_rx_buf, 29000);
            printf_uart_0_1 = UART_DEBUG; printf("\r New cmgw index = %d",index);
            if(index < 0 ) {
                printf_uart_0_1 = UART_DEBUG; printf("Error in CMGW so , will not send sms");
                break;
            }

            #define TMP_STR "\r\nAT+CMSS=%d,\"%s\"\r\n"
            reset_at_rx_buf();
            printf_uart_0_1 = UART_GSM;   printf(TMP_STR, index, mob_no);
            printf_uart_0_1 = UART_DEBUG; printf(TMP_STR, index, mob_no);
            parse_cmss_timeout( TYPE_VOID_P at_rx_buf, 60*2);R_WDT_Restart();
            break;
        } else {
            printf_uart_0_1 = UART_DEBUG; printf("\r Not got >");
        }
    }
    return index;
}

void broadcast_sms(char *sms)
{
    int i;
    char *mob_no;
    char temp_str_30[30];
    printf_uart_0_1 = UART_DEBUG; printf("\rEnter:>>> broadcast_sms sms: %s", sms);
    // appending Time.
    snprintf(temp_str_30, sizeof(temp_str_30), "\rTIME: %02d:%02d:%02d", oper.time_now.hh, oper.time_now.mm, oper.time_now.ss);
    strncat(send_sms_body_130, temp_str_30, sizeof(temp_str_30));

#if 0
    i=strlen(sms);
    while(i--) {
        sleep_ms_a(5);R_WDT_Restart();
        if ('\r' == sms[i]) {
            printf("<R>");continue;
        }
        if ('\n' == sms[i]) {
            printf("<N>");continue;
        }
        printf("%c",sms[i]);
    }
#endif
    for(i=0; i < phone_no_list_sz; i++){
        mob_no = get_stored_ph_number(i);
        if(NULL != mob_no) {
            printf_uart_0_1 = UART_DEBUG; printf("\rwrite sms:%s", mob_no);
            sms_write_to_modem( mob_no, sms);
        } else {
            //printf_uart_0_1 = UART_DEBUG; printf("\r NO write sms");
        }
    }
    send_sms_flag = false;
}

void reset_send_sms_body_130(void)
{
    printf_uart_0_1 = UART_DEBUG;printf("\renter: reset_send_sms_body_130");
    memset(send_sms_body_130, (int)NULL, sizeof(send_sms_body_130));
}

void issue_CPMS_Q(void)
{
    printf_uart_0_1 = UART_GSM;
    printf(CPMS);
    printf_uart_0_1 = UART_DEBUG;
}

void issue_CPMS_EQ_Q(void)
{
    printf_uart_0_1 = UART_GSM;
    printf("AT+CPMS=?");
    printf_uart_0_1 = UART_DEBUG;
}

void sms_delete_index_based(int index)
{
    //char str[20];
    printf_uart_0_1 = UART_DEBUG;
    printf("\r enter: sms_delete_index_based(%d)", index);

    printf_uart_0_1 = UART_GSM; printf("AT+CMGD=%d,0\r\n", index);
    sleep_ms_a(1000);R_WDT_Restart(); // TODO replace by active ok
    sleep_ms_a(1000);R_WDT_Restart();
    sleep_ms_a(1000);R_WDT_Restart();

    printf_uart_0_1 = UART_DEBUG;printf("\r return: sms_delete_index_based");
}

void issue_cmgl_all(void)
{
    printf_uart_0_1 = UART_DEBUG;
    printf("\renter:issue_cmgl_all");
    reset_at_rx_buf();
    printf_uart_0_1 = UART_GSM; printf(AT_CMGL_ALL);

    printf_uart_0_1 = UART_DEBUG;/* delete all sms. This is safty case, what if sms remained... */
    look_for_ok_resp_timeout(TYPE_CHAR_P at_rx_buf, 10000);
    //dump_at_rx_buf();
    printf("\rreturn:issue_cmgl_all");
}

/*retrun 0 if all is ok
 * return non zero +ve number  if there is an error
 * */
int parse_cmss_timeout(void  * buf, unsigned int timeout_seconds )
{
    unsigned int i;
    char *ptr;
    int sleep_delay_1000_ms = 1000;
    printf_uart_0_1 = UART_DEBUG;printf("\r parse_cmss_timeout %u seconds", timeout_seconds);

    for(i=0; i < timeout_seconds ; i++) {
        printf_uart_0_1 = UART_DEBUG;printf("\r i=%d", i);
        R_WDT_Restart(); sleep_ms_a(sleep_delay_1000_ms);
        ptr = strstr((void  *)buf, "+CMSS:");dump_at_rx_buf();
        if (ptr){
            printf_uart_0_1 = UART_DEBUG;  printf("\rgot CMSS replay");
            ptr = strstr((void  *)buf, "OK");dump_at_rx_buf();
            if(ptr) {
                printf_uart_0_1 = UART_DEBUG;  printf("\rgot CMSS replay AND OK; SMS sent!");
                return 0;
            }
        }else{
            ptr = strstr((void  *)buf, "ERROR");dump_at_rx_buf();
            if (ptr){
                printf_uart_0_1 = UART_DEBUG;  printf("\r Err in CMSS");
                R_WDT_Restart(); sleep_ms_a(500); // simply wiat to clear remaing buffer
                dump_at_rx_buf();
                return 1;
            }
        }

    }
        return 2;
}

int process_cmgl_output(void )
{
   /* +CMGL: index,message_status,phone_no,[phone_no_text],[service_center_time_stamp][,address_type,sms_message_body_length]<CR><LF>sms_message_body[<CR><LF>+CMGL: ...] */
    int index=0;
    int temp1_int, temp2_int;
    text_command_t temp1_uint;
    unsigned int temp2_uint, temp3_uint, temp4_uint, temp5_uint, temp6_uint;
    char message_status[10]; 
    char phone_no[20];
#define temp_str_sz 40
    char temp_str[temp_str_sz];
#define temp_str_sz2 40
    char temp_str_30[30];
    char temp_str_60[60];
    int yy,MM,dd,hh,mm,ss,tz;
    char sms_message_body[160];
    int cmd;
    char *ptr_cmgl;
    int ret_val;
    int temp_result;
    int i;
    char* local_at_rx_buf = TYPE_CHAR_P at_rx_buf;

    printf_uart_0_1 = UART_DEBUG; printf("\renter:process_cmgl_output()");

#if 0
    ptr_cmgl = strstr(at_rx_buf, "+CMTI: \"SM\",");
    if(NULL == ptr_cmgl){
        printf_uart_0_1 = UART_DEBUG; printf("\r no sms present");
        return 0; /* no more sms */
    }

    index = 0;
    ret_val = sscanf(ptr_cmgl,"+CMTI: \"SM\",%d", &index);
    if(1 == ret_val){
        if(0 != index) {
            sms_list_cmgr(index);
            sms_delete_index_based(index);
        } else {
            printf_uart_0_1 = UART_DEBUG; printf("\r no index is zero");
            return 0;
        }
    } else {
        printf_uart_0_1 = UART_DEBUG; printf("\r could not parse CMTI");
        return 0;
    }
#endif

#define STRING_CMGL "+CMGL:"

    // TODO : need to add a loop so that all sms will be parsed in a single shot.
    ptr_cmgl = strstr(local_at_rx_buf, STRING_CMGL);
    if(NULL == ptr_cmgl){
        printf("\rreturn: process_cmgl_output(): NO SMS present");
        return 0; /* no more sms */    
    } else {
        local_at_rx_buf += (sizeof(STRING_CMGL) - 1); // in next iteration, we need to go beyond of this sms
    }
    dump_at_rx_buf();
    
    ret_val = sms_parse(
            ptr_cmgl,
            &index, 
            message_status,
            phone_no,
            &yy, &MM, &dd, &hh ,&mm, &ss, &tz,
            sms_message_body
            );
    
    if( 0 != ret_val ) {
        printf("\r sms parsing failed");
        return 2;
    }

     if ( (FALSE == is_it_stored_phone_no(phone_no)) &&
          (NULL == strstr(phone_no, DEBUG_NUMBER)) &&
          (NULL == strstr(phone_no, DEBUG_NUMBER)) &&
          (NULL == strstr(phone_no, DEBUG_NUMBER))
        ) {
         printf("\r sms:unknown PH_NO");
         return 3;
    } else {
        printf("\r sms: KNOWN PH_NO");
    }

    cmd = cmd_max;
    temp_result = sscanf(sms_message_body, CMD_PRE"%d", &cmd);
    printf_uart_0_1 = UART_DEBUG; printf("\n temp_result=%d, cmd=%d", temp_result, cmd );

    if( (EOF == temp_result) || (1 != temp_result) ) {
        printf_uart_0_1 = UART_DEBUG; printf("\n could not parse sms message body (not header)");
        return 4; // wrong command
    }

    if( 0 == is_valid_text_cmd(cmd)){
        printf_uart_0_1 = UART_DEBUG; printf("\r wrong command");
        return 5;
    }

    switch(cmd) {
    case cmd_write_add_phone_number:
        printf_uart_0_1 = UART_DEBUG; printf("\n ADD_USER");
        temp_result = sscanf(sms_message_body,CMD_PRE"%d*%d*%s", &cmd, &temp1_int, temp_str);
        if (3 == temp_result) {
            printf_uart_0_1 = UART_DEBUG; printf("\n index=%d, ph no=%s", temp1_int, temp_str);
            store_phone_no(temp1_int, temp_str);
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "ADDED USER%d\rNUMBER:%s\rBY:%s", temp1_int, temp_str, phone_no);
            send_sms_flag = true;
        } else {
            printf_uart_0_1 = UART_DEBUG; printf("\n invalid store msg ");
        }
        break;
    case cmd_write_delete_phone_number:
        printf_uart_0_1 = UART_DEBUG; printf("\n DELETE_USER");
        temp_result = sscanf(sms_message_body, CMD_PRE"%d*%d", &temp1_int, &temp2_int);
        if(2 == temp_result) {
            printf_uart_0_1 = UART_DEBUG; printf("\n cmd=%d,user idx=%d", temp1_int, temp2_int);
            delete_phone_no_at_index(temp2_int);
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "DELETED USER%d\rBY : %s", temp2_int, phone_no);
            send_sms_flag = true;
        }
        break;
    case cmd_get_status:
        printf("\r send me the present status mode = %hhu", prov.basic_prov.device_type);
        if (water_sensor_tanks_1 == prov.basic_prov.device_type){
            printf("\r111 %s","\r GET status of MOBIL_LEVEL_CONTROLLER");
            send_sms_body_130[0]='\0';
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "STATUS\nMODE: %s", device_type_str((device_type_t) prov.basic_prov.device_type));
            printf("\r222 %s",send_sms_body_130);
            if(prov.water_level_prov.max_level == oper.current_water_level) {
                strcat(send_sms_body_130, "\rTANK IS FULL"); // turn off command
            } else if(0 == oper.current_water_level) {
                strcat(send_sms_body_130, "\rTANK IS EMPTY");//turn on motor
            }
            printf("\r333 %s",send_sms_body_130);
            strcat(send_sms_body_130, "\rWATER LEVEL = ");
            snprintf(temp_str_60, sizeof(temp_str_60), "%2d\rTANK IS %d%% FULL\rTIME: %s\rBY:%s", \
                    oper.current_water_level, \
                    100*oper.current_water_level/prov.water_level_prov.max_level, \
                    oper.time_now.str_time, phone_no);
            printf("\r444.1 temp_str_60 = %s", temp_str_60);
            printf("\r444 send_sms_body_130 = %s", send_sms_body_130);
            strncat(send_sms_body_130, temp_str_60, sizeof(temp_str_60));
            printf("\r 555 %s",send_sms_body_130);
        } else {
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                "STATUS\nmode:%s\rRelay1:%s\rTIME:%s\rBY:%s",
                device_type_str((device_type_t)prov.basic_prov.device_type),
                oper.relay_status[0]?"ON":"OFF", oper.time_now.str_time,
                             phone_no);
        }
        send_sms_flag = true;
        break;
    case cmd_read_phone_numbers:
        printf_uart_0_1 = UART_DEBUG; printf("\rGET_PHONE_BOOK");
        strcat(send_sms_body_130, "USER_NUM\r");
        for(i=0; i < phone_no_list_sz; i++){
            if(1 == prov.phone_book[i].valid){
                snprintf(temp_str_60, sizeof(temp_str_60),
                		"%u,%s\n", i, prov.phone_book[i].phone_no);
            }else{
                snprintf(temp_str_60, sizeof(temp_str_60), "%d,NO_PHONE\n", i);
            }
            strncat(send_sms_body_130, temp_str_60, sizeof(temp_str_60));
        }
        printf("\nsend_sms_body_130 = %s", send_sms_body_130);
        send_sms_flag = true;
        break;
    case cmd_read_light_24h_timer_table:
        //TIME_TBL\rhh:mm:ss,1_0\rhh:mm:ss,1_0  ...this repeats 10 times
        printf_uart_0_1 = UART_DEBUG; printf("\rGET_TT");
        strcat(send_sms_body_130, "TIME_TBL\r");
        for(i=0; i < relay_time_table_sz; i++){
            printf_uart_0_1 = UART_DEBUG;printf("\rTT[%u]: %02u-%02u-%02u RL=%u", i, prov.rl_tt[i].hh, prov.rl_tt[i].mm, prov.rl_tt[i].ss, prov.rl_tt[i].relay_status);
            snprintf(temp_str_30, sizeof(temp_str_30), "%02u:%02u:%02u,%01u\n",prov.rl_tt[i].hh, prov.rl_tt[i].mm, prov.rl_tt[i].ss, prov.rl_tt[i].relay_status);
            strncat(send_sms_body_130, temp_str_30, sizeof(temp_str_30));
        }
        send_sms_flag = true;
        snprintf(temp_str_30, sizeof(temp_str_30), "max_tt = %02u\n", prov.basic_prov.max_rl_tt_provisioned);
        strncat(send_sms_body_130, temp_str_30, sizeof(temp_str_30));

        break;
    case cmd_write_light_24h_timer_table:
        printf_uart_0_1 = UART_DEBUG; printf("SET_FIXED_TIMER_TT");
        // cmd*index*hh*mm*ss*rly_status; example
        // 17*0*6*55*5*0  ==>  stored at index 0, turns off at 6:17:44
        // 17*1*1*18*50*1  ==>  stored at index 1, turns on at 6:17:44
        temp_result = sscanf(sms_message_body, CMD_PRE"%u*%u*%u*%u*%u*%u", 
                &temp1_uint, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint, &temp6_uint);
        
        printf_uart_0_1 = UART_DEBUG;
        printf("\n cmd=%u, index=%u, hh:mm:ss=%u:%u:%u rly=%u", 
                temp1_uint, temp2_uint, temp3_uint, temp4_uint, temp5_uint, temp6_uint);

        store_to_dataflash(temp1_uint, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint , &temp6_uint);
        snprintf(send_sms_body_130, sizeof(send_sms_body_130), "TimeTable UPDATE\rBy %s", phone_no);
        send_sms_flag = true;
        time_table_changed = true;
        reset_time_table_sent_sms();
        break;

    case cmd_write_light_24h_timer_table_prov_max:
        printf_uart_0_1 = UART_DEBUG; printf("\n cmd_write_light_24h_timer_table_prov_max");
        // cmd*timer_tt_prov_max ex:  25*2
        temp_result = sscanf(sms_message_body, CMD_PRE"%u*%u", &temp1_uint, &temp2_uint);
        printf_uart_0_1 = UART_DEBUG; printf("\n cmd=%u, max=%u", temp1_uint, temp2_uint, temp3_uint, temp4_uint, temp5_uint, temp6_uint);
        store_to_dataflash(temp1_uint, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint , &temp6_uint);
        snprintf(send_sms_body_130, sizeof(send_sms_body_130), "tt_prov_max \rBy %s", phone_no);
        send_sms_flag = true;
        time_table_changed = true;
        reset_time_table_sent_sms();
        break;

// 1 arg, but no data to be stored
    case cmd_reset_mcu:
        printf_uart_0_1 = UART_DEBUG; printf("\nSET_RESET_MCU");
        reset_mcu(0);
        break;
    case cmd_reset_prov:
        printf_uart_0_1 = UART_DEBUG; printf("\ncmd_reset_prov");
        //provision_to_defaults();
        break;
    case cmd_read_sw_hw_rev:
    	printf_uart_0_1 = UART_DEBUG; printf("\ncmd_read_sw_hw_rev");
        snprintf(send_sms_body_130, sizeof(send_sms_body_130),
                "\nHW:R1.2\nSW:1.1\nDT:2021-01-11\nBy:%s", phone_no);
    	break;
//###########################################################
// 2 args
    case cmd_write_starter_n_call_aware: // cmd*motor_N
    case cmd_set_mcu_boot_delay:
    case cmd_write_device_type: // cmd*device_type

    case cmd_write_water_level_soak_time:
    case cmd_write_water_level_max_level:

    case cmd_write_sms_reply_enabled:
        printf_uart_0_1 = UART_DEBUG; printf("\n 2 args processing");
        send_sms_flag = true;
        temp_result = sscanf(sms_message_body, CMD_PRE"%u*%u", &temp1_uint, &temp2_uint);
        if(2 != temp_result){
            printf_uart_0_1 = UART_DEBUG; printf("\nMSG is wrong 113121 ARGS=%d", temp_result);
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), "\nNOT ENOUGH ARGS\rBy %s", phone_no);
            break;
        }
        printf_uart_0_1 = UART_DEBUG; printf("\n cmd=%u, param=%u", temp1_uint, temp2_uint);
        store_to_dataflash(temp1_uint, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint, &temp6_uint);
        switch(cmd) {
        case cmd_set_mcu_boot_delay:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "\nset_mcu_boot_delay=%u WRITTEN \rBy %s", temp2_uint, phone_no);
            break;
        case cmd_write_starter_n_call_aware:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130),
                    "\nMOTOR_N_CALL_ACTIVE=%u WRITTEN \rBy %s", temp2_uint, phone_no);
            break;
        case cmd_write_device_type:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "\nDEVICE TYPE:%s\nWRITTEN BY:%s",  
                    device_type_str((device_type_t) temp2_uint), phone_no);
            change_relays_oper_status_dueto_event(new_text_command_received);
            break;
        case cmd_write_water_level_soak_time:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "\nUPDATED\nWater soak time\nBy %s", phone_no);
            break;
        case cmd_write_water_level_max_level:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), 
                    "\nwater_level_max_level\nWRITTEN By: %s", phone_no);
            break;
        case cmd_write_sms_reply_enabled:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130),
                    "\nsms_reply_enabled\nWRITTEN By %s", phone_no);
            break;
        default:
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), "UNKNOWN\rBy %s", phone_no);
            break;
        }
        break;
//###########################################################
// 3 args
    case cmd_write_starter_mode: //cmd*motor_N*mode(0|1) ;mode=> 0=temp, 1=auto
    case cmd_write_starter_status: //cmd*motor_N*status ;  0 or 1
    case cmd_write_starter_start_stop_relay_hold_time:
    case cmd_write_manual_switch_status_provision:
        send_sms_flag = true;
        temp_result = sscanf(sms_message_body, CMD_PRE"%u*%u*%u", &temp1_uint, &temp2_uint, &temp3_uint);
        if(3 != temp_result){
            printf_uart_0_1 = UART_DEBUG; printf("\nMSG is wrong 345235=%d", temp_result);
            snprintf(send_sms_body_130, sizeof(send_sms_body_130), "\nNOT ENOUGH ARGS\rBy %s", phone_no);
            break;
        }
        printf("\r Starter cmd=%u, %u, %u", temp1_uint, temp2_uint, temp3_uint);
        store_to_dataflash(temp1_uint, &temp2_uint, &temp3_uint, &temp4_uint, &temp5_uint , &temp6_uint);
        switch(temp1_uint){
            case cmd_write_starter_mode:
            case cmd_write_starter_status:
            case cmd_write_manual_switch_status_provision:
                change_relays_oper_status_dueto_event(new_text_command_received);
                break;
        }
        snprintf(send_sms_body_130, sizeof(send_sms_body_130), "\nAn UPDATE BY By %s", phone_no);
        break;
    default:
        printf_uart_0_1 = UART_DEBUG; printf("\r WRONG SMS");
        break;
    }
    printf_uart_0_1 = UART_DEBUG;printf("\rReturn :<<<< process_cmgl_output");
    return 1; // successfully parsed at least one sms
}
