#include "0_all.h"

int time_t_mcuable_changed = true;
volatile char at_rx_buf[AT_RX_BUF_SZ];
char send_sms_body_160[160];
bool_t send_sms_flag; // true because, at POWER UP, we need to send first message.
bool_t activate_relays_based_on_oper_stat_flag;

void detect_at_rx_buf_overflow_and_clear(void) {
    printf("detect_at_rx_buf_overflow_and_clear len=%lu \n",
            strlen((const char __near *)at_rx_buf));
    if(strlen((const char __near *)at_rx_buf) >  (AT_RX_BUF_SZ/2) ) {
        /* buffer is almost full. this means, there is garbage accumulated.
         * So far it is not processed, so, it will not be processed next also.
         * clear it all once..
         * it will not cause issue  due to half input since any input is so small, it will be smaller than
         * 100 bytes. so, we will almost never have a sms indication half during this api...
         */
        printf("RESETting BUF full\n");
        reset_at_rx_buf();
    } else {
        //printf("\nBUF not full %d / %d ", strlen(at_rx_buf),  (AT_RX_BUF_SZ/3));
    }
}

int read_unsolicited_result_code(void) {
    int ret_val=0;
     printf("-> read_unsolicited_result_code");
    if( (NULL != strstr((const char __near *)at_rx_buf, "+CMTI: \"SM\",")) || (NULL != strstr(TYPE1 at_rx_buf, "+CMGL:")) ) {
        printf("there is a sms \n");
        ret_val = 1;
    } else  if( NULL != strstr(TYPE1 at_rx_buf, "+CLIP:")) {
        printf("There is some incoming call\n");
        ret_val=2;
    } else {

        printf(" X ");
        ret_val =  3;
    }
    //
    //printf("\n<- read_unsolicited_result_code");
    return ret_val;
}

void hang_the_call(void) {
    R_WDT_Restart();
    printf("Hang the call\n");
    write_to_modem("\rATH0\r");
    R_WDT_Restart();sleep_ms(1000);R_WDT_Restart();
}

void release_all_calls(void) {
    R_WDT_Restart();
    printf("Enter: release_all_calls()\n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CHLD=0\r");
   	look_for_ok_resp_timeout((char *)at_rx_buf, 2000);
    R_WDT_Restart();
}

void reset_modem(void) {
    R_WDT_Restart();
    printf("reset_modem\n");
    write_to_modem("\rAT+CFUN=1,1\r");
    R_WDT_Restart();sleep_ms(1000);R_WDT_Restart();
}

void enable_date_time_sync_and_sms_storage_area(void) {
    printf(">>> enable_date_time_sync_and_sms_storage_are \n");
    printf("Start : Dumping profile\n");
    reset_at_rx_buf();
    write_to_modem("\rAT&V\r");// dump profile config
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    reset_at_rx_buf();
    printf("End: Dumping profile \n");

#if 1
    /* Enable the network date time sync so that CCLK works.*/
    printf("issue AT+CLTS_EQ_1 \n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CLTS=1\r");// call receive
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000);
    dump_at_rx_buf();
#endif
#if 1
    /* Enable the network date time sync so that CCLK works.*/
    printf("issue CTZU_EQ_1\n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CTZU=1\r");// call receive
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000);
    dump_at_rx_buf();
#endif

    printf("issue CPMS \n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CPMS?\r");// Wemy sms sotarge areaa
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000); //TODO
    dump_at_rx_buf();

    printf("issue CPMS_EQ_Q\n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CPMS=?\r");
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000); //TODO
    dump_at_rx_buf();

    printf("issue CPMS_EQ_ME_ME_ME\n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CPMS=\"ME\",\"ME\",\"ME\"\r");
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000); //TODO
    dump_at_rx_buf();

    R_WDT_Restart();sleep_ms(2000);
    gsm_config_write(); // Write to Modem
    R_WDT_Restart();sleep_ms(2000);

    printf("Start : Dumping profile\n");
    reset_at_rx_buf();
    write_to_modem("\rAT&V\r");// dump profile config
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000); //TODO
    dump_at_rx_buf();
    reset_at_rx_buf();
    printf("End : Dumping profile\n");

    // TODO : add the what ever done in init_gsm1
    //TODO add the 9600 default baud rate so that, it is hard wired into profile! no auto bauding...
    //TODO: ADD MODEM REBOOT  (CFUN pwoer reset?                           )
}

void init_gsm1(void) {
    printf(" >>> init_gsm1\n");
    R_WDT_Restart();
    reset_at_rx_buf();

    write_to_modem("\r\x1B\r");// ESCPE
    reset_at_rx_buf();
    write_to_modem("\rAT+CMGF=1\r");//sms format : //TODO : is this needed? can we write into Profile?
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 5000);
    //TODO : chcek for ret value

    reset_at_rx_buf();
    write_to_modem("\rAT+CLIP=0\r");// call receive : //TODO : is this needed? can we write into Profile?
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 5000);

    sms_delete_all();

    /*
    //sms_delete_all(); // delete all SMS at start . it is a stop gap measure. if there are too many sms, dump will over full the buffer. so best close it. No, donot do this... if too many message, find it by not getting OK and issue delete all. TODO
    // at last, call this for date and time sync. only once when u set new GSM chip, because, in this we write to eeprom of SIMCOM
    // Do something read from the eeprom status for this and enable it on own.
    //enable_date_time_sync_and_sms_storage_area(); //TODO
     * */
    //dump_at_rx_buf();
    printf("<<< init_gsm1()\n");
}

void gsm_config_write(void) {
    R_WDT_Restart();
    printf(" gsm_config_write\n");
    reset_at_rx_buf();
    write_to_modem("\rAT&W0\r");
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000);
    dump_at_rx_buf();
    printf(" <<< gsm_config_write()\n");
}

void remove_ok(char *buf  ) {
    // TODO: look fully into stream , tehre may be more than 1 OK strings....
    char *ptr;

    printf(" >>> remove ok()\n");
    while(1) {
        ptr = strstr(TYPE1 buf, OK_RESP_STR);
        if ( ptr ){
            printf("removing OK!!!!!!!\n");
            memset(ptr, 'Z', sizeof(OK_RESP_STR) - 1 );
        } else {
            break;
        }
    }
    printf("<<<  remove ok()\n");
}

void issue_AT_command(void) {
    printf(" Enter: issue_AT_command()\n");
    //write_to_modem("\r\x1A\r"); /* Ctrl-z is 0x1A SUBstitue char, to clear if modem is hanged in SMS sending... */
    write_to_modem("\r\x1A\r\rAT\r"); /* Ctrl-z is 0x1A SUBstitue char, to clear if modem is hanged in SMS sending... */
    printf(" Enter: issue_AT_command()\n");
}

#if 0
void process_sms_all(void)
{
    int temp=0;
    while(1){
        temp = process_sms();
        if(0 == temp) {

            printf("\n no more sms");
            break;
        } else {

            printf("\n let us see if there is one more sms");
        }
    }
}
#endif

void strip_country_code(char  *phone_no) {
    printf(" strip_country_code \n");
    printf(" before %s\n", phone_no);
    if(0 == strcmp("+91", phone_no)) {
        memmove(phone_no, &phone_no[3], 11); // 10 Digits of phone no + NULL
    }
    printf("after %s\n", phone_no);
}

void diagnose_modem(void) {
    int count = 5;
    printf(" Enter diagnose_modem()\n");
    reset_at_rx_buf();
    write_to_modem("\rAT+CSQ\r"); // query signal strength
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000);
    dump_at_rx_buf();
    reset_at_rx_buf();
    while(count) {
        issue_AT_command();
        if ( 0 == look_for_ok_resp_timeout(TYPE2 at_rx_buf, 10000)) {
            printf(" Diagnostic: all is good\n");
            break;
        } else {
            printf(" ERROR, reset mcu so that everythin will be reset.\n");
            dump_at_rx_buf();
            //reset_mcu(0);
        }
        count--;
    }
    if (0 == count){
        printf(" reset UART0/1 + modem; but not mcu!\n");
    }
    //reset_at_rx_buf();
    printf(" Return: diagnose_modem()\n");
}
void process_modem(void) {
    static unsigned int gsm_reset_cnt;
    printf(" Enter process_modem()\n");
    R_WDT_Restart();
    // we need to reset the outgoing sms buffer.
    if(false == send_sms_flag){
        printf("resetting the send_sms_body_160 \n");
    	send_sms_body_160[0]='\0';
    } else {
        printf("NOT resetting the send_sms_body_160! strange \n");
    }
    //dump_at_rx_buf();
    //dump_at_rx_buf_len();
#if 0
    //TODO: this is one more design. Instead of polling, we can read the sonlisiacted msg and take action based on it. ie, call and sms gives unsoliciated msg, we can take action based on taht. that would be a lot simple,
    //once things stablalises, i can go from present poll+issue at cmds to unsoliciated msg. taht would save issueing cmds every 2 seconds of poller

    temp = read_unsolicited_result_code();
    switch(temp)
    {
    case 1:// sms is present
         printf("\r process all SMS");
        //process_sms_cmti();
        break;
    case 2: // call is present
         printf("\r process call");
        process_call();
        break;
    default:
        // printf("*");
        break;
    }
#endif
    //detect_at_rx_buf_overflow_and_clear();
    //list all sms and process them: TODO first get sms count, then act... if no sms, ignore...
    printf("================================ step 0. diagnose_modem()\n");
    diagnose_modem();
    printf("================================ step 1.time fetching\n");
    reset_at_rx_buf();
    issue_cclk();
    process_cclk_output();
    R_WDT_Restart();
    printf("================================ step 2: sms related\n");
    reset_at_rx_buf();
    issue_cmgl_all();
    process_cmgl_output();
    R_WDT_Restart();
    printf("================================ step 3: call related\n");
    reset_at_rx_buf();
    issue_clcc();
    process_clcc_output();

    if (prov.basic_prov.device_type == fixed_timer_switch) {
        printf("================================ step 5: fixed timer related\n");
        timer_processing();
    } else if (prov.basic_prov.device_type == astro_timer_switch) {
        printf("================================ step 6: Astro timer related\n");
        astro_test();
    } else if (prov.basic_prov.device_type == water_sensor_tanks_1) {
        printf("================================ step 7 Water tank levels \n");
        process_tank_levels();
    }

    if(send_sms_flag) {
        printf("================================ step 10: sending sms \n");
        broadcast_sms(send_sms_body_160);
        reset_send_sms_body_160();
    }

    if(activate_relays_based_on_oper_stat_flag) {
        printf("============================= step 11: relay activation related \n");
        activate_relays_based_on_oper_stat();
    }

    gsm_reset_cnt++;
    printf("gsm_reset_cnt=%u \n", gsm_reset_cnt);
    if( 0 == (gsm_reset_cnt% SMS_DEL_CNT)){
        gsm_reset_cnt=1;
        printf("DEL ALL SMS \n");
        sms_delete_all();
    }
    if( 0 == (gsm_reset_cnt % (GSM_RESET_CNT))){
         printf("reset+init \n");
        reset_gsm_modem();
        init_gsm1();
    }
    if( 0 == (gsm_reset_cnt % (GSM_INIT_CNT))){
        printf("init_gsm \n");
        init_gsm1();
    }
    printf("================================ \n");
    printf("return: process_modem() \n");
}

/* For Date time syncing from GSM nw*/
void issue_cclk(void) {
    printf(" >>> issue_cclk()\n");
    write_to_modem("\rAT+CCLK?\r");
    if ( look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000) ) {
        printf(" OK not found reboot now 2 \n");
    }
}

/* For Date time syncing from GSM nw*/
void process_cclk_output(void) {
    char *ptr_cmgl;
    int temp_int1;
    int temp_int2;
    int temp_int3;
    int temp_int4;
    int temp_int5;
    int temp_int6;
    int temp_int7;
    int ret_val;
    printf(" Enter: process_cclk_output()\n");
    dump_at_rx_buf();
    ptr_cmgl = strstr(TYPE1 at_rx_buf, "+CCLK: ");
    if(NULL == ptr_cmgl){
         printf(" NO CCLK present\n");
        return; /* no call exists */
    } else {
        printf("Yes CCLK present\n");
    }
    R_WDT_Restart();
    //  printf("\n%s", ptr_cmgl);
    ret_val = sscanf(ptr_cmgl,"+CCLK: \"%d/%d/%d,%d:%d:%d+%d\"",
    		         &temp_int1, &temp_int2, &temp_int3,&temp_int4,
					 &temp_int5, &temp_int6, &temp_int7);
    // printf("\n ret_val = %d" , ret_val);
    if( 7 != ret_val) {
         printf("error in CCLK\n");
        return;
    }

    printf("yy:MM:dd:hh:mm:ss + tz=%d:%d:%d Time %d:%d:%d  TZ: %d \n",temp_int1, temp_int2,
    		temp_int3, temp_int4, temp_int5, temp_int6, temp_int7);
    oper.time_now.year = 2000 + temp_int1 ;
    oper.time_now.month = temp_int2;
    oper.time_now.month_day = temp_int3;
    oper.time_now.hour = temp_int4;
    oper.time_now.minutes = temp_int5;
    oper.time_now.seconds = temp_int6;
    snprintf(oper.time_now.str_time, sizeof(oper.time_now.str_time)+1,
            "%04d-%02d-%02dT%02d:%02d:%02d+05:30",
            oper.time_now.year,oper.time_now.month,oper.time_now.month_day,
            oper.time_now.hour,oper.time_now.minutes,oper.time_now.seconds);
    printf("oper.time_now.str_time=%s \n", oper.time_now.str_time);
    printf(" Return: process_cclk_output()\n");
}

void issue_clcc(void) { // Query call
    printf("Enter: Enter: issue_clcc\n");
    write_to_modem("\rAT+CLCC\r");
    look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000);
    printf("Return: issue_clcc\n");
}

// call processing
int process_clcc_output(void) {
    char phone_no[20]="", temp_str_50[50], *ptr_cmgl;
    int fun_ret_val = 0, temp_int1, ret_val;
    unsigned int motor_number = 0;
    unsigned int relay_number, starter_status;
    relay_status_t manual_switch_status;

    printf("Enter : process_clcc_output()\n");
    ptr_cmgl = strstr(TYPE_CONST_CHAR_P at_rx_buf, "+CLCC:");
    if(NULL == ptr_cmgl){
        printf("NO call present\n");
        return 0;
    } else {
        printf("Call exists\n");
    }
    printf("%s\n", ptr_cmgl);
    ret_val = sscanf(ptr_cmgl,"+CLCC: %d,%d,%d,%d,%d,\"%[+0-9]\",%d",
            &temp_int1, &temp_int1, &temp_int1,	&temp_int1,&temp_int1, phone_no, &temp_int1);
    // printf("\n ret_val = %d" , ret_val);
    release_all_calls();
    if( 7 != ret_val) {
         printf("error in Call parsing \n");
        return 2;
    }
    printf("incoming phone_no=%s\n", phone_no);
    if( (FALSE == is_it_stored_phone_no(phone_no)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER)) ) {
         printf("call: from UN-known phone number\n");
         return 0;
    }
    send_sms_flag = true;
    printf("call: KNOWN phone number: toggle now\n");
    switch(prov.basic_prov.device_type) {
    case motor_starter:
        switch(prov.motor_prov_obj.starter_mode[0]) {
        case starter_mode_automatic:
            handle_starter_status_mode_automatic(new_phone_call_received, cmd_write_starter_status, 0, 0, phone_no);
            break;
        case starter_mode_temporary:
            handle_starter_status_temporary(new_phone_call_received, cmd_write_starter_status, 0, 0, phone_no);
            break;
        default:
            strncat(send_sms_body_160, "ERROR: STARTER MODE\n", MY_SMS_LENGHT);
            break;
        }
        break;
    case manual_switch: //now change the provision
        printf("new_phone_call_received \n");
        activate_relays_based_on_oper_stat_flag = true;
        for (relay_number=0; relay_number<MAX_RELAYS; relay_number++ ) {
            if(on == prov.manual_switch_status[relay_number]) {
                printf("ON=>OFF\n");
                manual_switch_status = relay_off;
                set_relay_oper_status(relay_number, manual_switch_status);
            } else {
                printf("OFF=>ON\n");
                manual_switch_status = relay_on;
                set_relay_oper_status(relay_number, manual_switch_status );
            }
            store_to_dataflash(cmd_write_manual_switch_status_provision, (void *) &relay_number,
                (void *) &manual_switch_status, NULL, NULL, NULL);
        }
        snprintf(temp_str_50, sizeof(temp_str_50)-1, "Manual Switch-%hhu %s\n",
            relay_number, relay_status_t_str(manual_switch_status));
        strncat(send_sms_body_160, temp_str_50, MY_SMS_LENGHT);
        if(send_sms_flag) {
            strncat(send_sms_body_160, "CALL BY:", MY_SMS_LENGHT);
            strncat(send_sms_body_160, phone_no, MY_SMS_LENGHT);
            strncat(send_sms_body_160, "\n", MY_SMS_LENGHT);
            printf("send_sms_body_160===>%s<===\n", send_sms_body_160);
        }
        break;
    case water_sensor_tanks_1:
        printf("water_sensor_tanks_1 \n");
        prepare_sms_water_level(oper.current_water_level);
        if(send_sms_flag) {
            strncat(send_sms_body_160, "CALL BY:", MY_SMS_LENGHT);
            strncat(send_sms_body_160, phone_no, MY_SMS_LENGHT);
            strncat(send_sms_body_160, "\n", MY_SMS_LENGHT);
            printf("send_sms_body_160===>%s<===\n", send_sms_body_160);
        }
        break;
    case astro_timer_switch:
        printf("astro_timer_switch\n");
        strncat(send_sms_body_160, "astro_timer_switch\n", MY_SMS_LENGHT);
        append_on_off_time(&prov, &oper, send_sms_body_160);
        if(send_sms_flag) {
            strncat(send_sms_body_160, "CALL BY:", MY_SMS_LENGHT);
            strncat(send_sms_body_160, phone_no, MY_SMS_LENGHT);
            strncat(send_sms_body_160, "\n", MY_SMS_LENGHT);
            printf("send_sms_body_160===>%s<===\n", send_sms_body_160);
        }
        break;
    default:
        printf(" Error !!! 324234");
        break;
    }

    printf("Return:process_clcc_output()\n");
    return fun_ret_val;
}

#if 0
void process_all_gsm_input(void)
{
    static int gsm_reset_cnt=1;//avoid resetting first, 1 second counter
    int temp;
     printf("\n->process_all_gsm_input ");
    R_WDT_Restart();
    //dump_at_rx_buf();
    dump_at_rx_buf_len();

    temp = read_unsolicited_result_code();
    switch(temp)
    {
    case 1:// sms is present
         printf("\n process all SMS");
        //process_sms_cmti();
        break;
    case 2: // call is present
         printf("\n process call");
        process_call();
        break;
    default:
        // printf("*");
        break;
    }
    detect_at_rx_buf_overflow_and_clear();

    gsm_reset_cnt++;
     printf(" gsm_reset_cnt=%d", gsm_reset_cnt);

    if( 0 == (gsm_reset_cnt% SMS_DEL_CNT) ) {
        gsm_reset_cnt=1;
         printf("\r DEL ALL SMS ");
        sms_delete_all();
    }

    if( 0 == (gsm_reset_cnt % (GSM_RESET_CNT)) ) {
         printf("\r reset + init_gsm >>>");
        reset_gsm_modem();
        init_gsm();
    }
}
#endif

#if 0
int process_call(void)
{
    char phone_no[20];
    char temp_str1_50[50];
    char *ptr_cmgl;
    //int ret_val;
    int fun_ret_val = 0;
    int starter_status = 0;

    printf("\nEnter: process_call");
    dump_at_rx_buf();

    ptr_cmgl = strstr(TYPE1 at_rx_buf, "+CLIP:");
    if(NULL == ptr_cmgl){

        printf("\nReturn: process_call: no call present");
        return 0; /* no call exists */
    }

    R_WDT_Restart();sleep_ms(200);R_WDT_Restart();
      printf("\n%s", ptr_cmgl);
    sscanf(ptr_cmgl,"+CLIP: \"%[+0-9]\"", phone_no);
    printf("\n incoming phone_no=%s", phone_no);
    hang_the_call();

    if( (FALSE == is_it_stored_phone_no(phone_no)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER))
      ) {
         printf("\n call:UN-known phone number");
        //fun_ret_val = 3;
    } else {
        printf("\n call: KNOWN phone number: toggle now");
       starter_status = get_relay_oper_status(0);
       // if it is already OFF, turn it to AUTO-ON
       if(0 == starter_status) {
            printf("\n AUTO ON by call");
           set_relay(0,1);
           //store_to_dataflash(text_command_SET_MODE_LIGHTS_ON, &temp2_uint, &temp2_uint, &temp2_uint, &temp2_uint , &temp2_uint);
           snprintf(temp_str1_50, sizeof(temp_str1_50), "ON BY %s (CALL)", phone_no);
           broadcast_sms(temp_str1_50);
       } else {
           // turn it OFF
            printf("\n OFF by call");
           set_relay(0,0);
           //store_to_dataflash(text_command_SET_MODE_LIGHTS_OFF, &temp2_uint, &temp2_uint, &temp2_uint, &temp2_uint , &temp2_uint);
           snprintf(temp_str1_50, sizeof(temp_str1_50),"OFF BY %s (CALL)", phone_no);
           broadcast_sms(temp_str1_50);
       }
   }
    reset_at_rx_buf(); // clear the buffer ...
    printf("\nReturn: process_call()");
    return fun_ret_val;
}



int get_call_status_and_act(void)
{
    int fun_ret_val = 0;
    printf("\n issue AT+CLCC");
    printf("\rAT+CLCC\r");
    if ( look_for_ok_resp_timeout(TYPE2 at_rx_buf, 2000) ) {
        printf("\n OK not found reboot now 1 ");
        //sleep_ms(8000);
    }

    printf("\n process_call >>>>>");
    dump_at_rx_buf();
#if 0
    ptr_cmgl = strstr(at_rx_buf, "+CLIP:");
    if(NULL == ptr_cmgl){

        printf("\n no call present");
        return 0; /* no call exists */
    }

    R_WDT_Restart();sleep_ms(200);R_WDT_Restart();
      printf("\n%s", ptr_cmgl);
    sscanf(ptr_cmgl,"+CLIP: \"%[+0-9]\"", phone_no);
    printf("\n incoming phone_no=%s", phone_no);
    hang_the_call();

    if( (FALSE == is_it_stored_phone_no(phone_no)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER)) &&
        (0 != strcmp(phone_no, DEBUG_NUMBER))
      ) {
         printf("\n call:UN-known phone number");
        //fun_ret_val = 3;
    } else {
        printf("\n call: KNOWN phone number: toggle now");
       starter_status = get_relay(0);
       // if it is already OFF, turn it to AUTO-ON
       if(0 == starter_status) {
            printf("\n AUTO ON by call");
           set_relay(0,1);
           store_device1_mode(AUTO_ON);
           snprintf(temp_str1_50, sizeof(temp_str1_50), "AUTO_ON BY %s (CALL)", phone_no);
           broadcast_sms(temp_str1_50);
       } else {
           // turn it OFF
            printf("\n OFF by call");
           set_relay(0,0);
           store_device1_mode(OFF);
           snprintf(temp_str1_50, sizeof(temp_str1_50),"OFF BY %s (CALL)", phone_no);
           broadcast_sms(temp_str1_50);
       }
   }
#endif
    reset_at_rx_buf(); // clear the buffer ...
    return fun_ret_val;
}


#endif
