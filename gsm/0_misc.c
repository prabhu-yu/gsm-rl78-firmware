#include "0_all.h"

volatile unsigned int reset_source;

void print_float(float f)
{
    printf(" %d.%04d, ",
            (int) (trunc(f)),
            (int) fabs((int)((f - (trunc(f))) * 10000)));
    //printf("%d.%d",(int)(f), (int)(f -(int (f)))));
    //printf("%d.%d",(int)f, (int)(f -(int)(f)));
}

void diag_gsm(void) {
    printf("\n/gcDiag1");
	volatile uint8_t c;
	R_WDT_Restart();
	reset_at_rx_buf();
    R_UART0_Receive_non_blocking((uint8_t *)&c, 1);
    while(1){
    	dump_at_rx_buf();
        R_WDT_Restart();
        sleep_ms(1000);
        R_WDT_Restart();
    	if (1 == has_R_UART0_Receive_data()) {
            printf("\n/gcu entered = %c", c);
    		process_keyboard(c);
    		R_UART0_Receive_non_blocking((uint8_t *)&c, 1);
    	}
    }
}

char *relay_status_string[] = {
    "off",
    "on",
    "on_hold_off"
};

char *motor_oper_status_string[] = {
    "motor_oper_off",
    "motor_oper_on"
};

void lower_string(char s[], uint16_t len)
{
    int c = 0;
    while (c != len ) {
        if (s[c] >= 'A' && s[c] <= 'Z') {
            s[c] = s[c] + 32;
        }
        c++;
    }
}

volatile uint32_t timer_cnt_ms;
/*1 ms resolution busy sleep timer , this should not exceed 65K ms */
void sleep_ms(int ms)
{
	volatile int stop_val = ms;
#if MACHINE == RL78
	 R_TAU0_Channel0_Stop();
	 timer_cnt_ms = 0;
	 R_TAU0_Channel0_Start();
	 while(timer_cnt_ms < stop_val) {
        NOP();
        NOP();
        NOP();
        NOP();
		 //TODO do nothing. add Deep sleep here since INT will wake up anyway
	 }
	 R_TAU0_Channel0_Stop();
#elif MACHINE == UBUNTU
     usleep(ms * 1000);
#endif

}

#if MACHINE == RL78
void sleep(unsigned int seconds) {
    int i;
    for(i=0; i<seconds; i++) {
        sleep_ms(1000);
    }
}
#endif


#if 0
/*1 sec resolution busy sleep timer , this should not exceed */
void sleep(uint32_t sec)
{
	volatile int stop_val = ms;
	 R_TAU0_Channel1_Stop();
	 timer_cnt_sec = 0;
	 R_TAU0_Channel1_Start();
	 while(timer_cnt_sec < stop_val) {
        NOP();
        NOP();
        NOP();
        NOP();
		 //TODO do nothing. add Deep sleep here since INT will wake up anyway
	 }
	 R_TAU0_Channel0_Stop();
}
#endif

void refresh_gpio(void) {

}

void my_sleep(int seconds)
{
	int i;
	for(i=0;i<seconds;i++){
		R_WDT_Restart();
		sleep_ms(1000);
		R_WDT_Restart();
	}
}

void gprs_testing(void)
{
	printf("\n GPRS  testing");
#define GPRS_SLEEP 6
    reset_at_rx_buf();
    my_sleep(GPRS_SLEEP);

    write_to_modem("\n/gcAT\r");

    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    write_to_modem("\n/gcAT+CIPSHUT\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    write_to_modem("\n/gcAT+CGATT=1\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    char *buffer = "\n/gcAT+CSTT=\"airtelgprs.com\",\"\",\"\"\r";
    write_to_modem(buffer);

    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    write_to_modem("\n/gcAT+CIICR\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    write_to_modem("\n/gcAT+CIFSR\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
	//printf("\n/gc\nAT+CIPSTART=\"TCP\",\"10.235.99.90\",\"5600\"\r\n");my_sleep(GPRS_SLEEP);dump_at_rx_buf();
    write_to_modem("\n/gcAT+CIPSTART=\"TCP\",\"exploreembedded.com\",\"80\"\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    write_to_modem("\n/gcAT+CIPSEND=63\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();
    write_to_modem("\n/gcGET exploreembedded.com/wiki/images/1/15/Hello.txt HTTP/1.0\r");
    my_sleep(GPRS_SLEEP);
    dump_at_rx_buf();

    printf("\n/gc GPRS DONE");
}

void print_line_break(void) {
    printf("====================================================\n");
}

void print_line_star_break(void) {
    printf("****************************************************\n");
}

void dump_oper(void) {
	unsigned int i;
	printf("\n ############### dump oper ####################");
	for(i=0;i<MAX_RELAYS;i++){
        printf("\n       :relay_status[%hhu] = %hhu", i,
               oper.relay_status[i]);
	}

	for(i=0; i<MAX_STARTERS; i++){
        printf("\n/gc       :starter_status[%d] = %d", i,
               oper.starter_status[i]);
	}
    printf("\n        Date/Time : %d : %d : %d -  %d : %d : %d",
           oper.time_now.year, oper.time_now.month, oper.time_now.month_day,
           oper.time_now.hour, oper.time_now.minutes, oper.time_now.seconds);
    printf("\n Xcurrent_water_level = %u", oper.current_water_level);

#if MACHINE == RL78
	R_CGC_Get_ResetSource();
#endif
	printf("\nreset_source=%X", reset_source);

    printf("\n Astro oper");

    dump_date_time(&oper.astro_oper.time_now);
    dump_date_time(&oper.astro_oper.sunrise_time);
    dump_date_time(&oper.astro_oper.sunset_time);

    printf("\n END OF OPER DUMP");
}

void process_keyboard(uint8_t kb_input) {
	uint8_t temp_str_100[100];
	int cmd;
	int param1, param2, param3, param4, param5;
	uint8_t confirm_code=0, counter=1;
	char phone_no[15], msg_status[40];
    int index,yy,MM,dd,hh,mm,ss,tz;
    printf("process_keyboard( %c )\n", kb_input);

#if 0
#if MACHINE == RL78
    R_UART0_Receive_non_blocking(&confirm_code, 1);
    printf("enter confirm code now\n");
    while(counter++) {
    	printf("counter=%d\n", counter);
        R_WDT_Restart();
    	if(10 == counter){
    		return;
    	}
    	sleep_ms(1000);
    	if('y' == confirm_code){
    		printf("GOOD\n");
    		break;
    	} else {
    		printf("timed out now! counter=%d\n", counter);
    	}
    }
#endif
#endif
    switch(kb_input){
    case '0':
    	dump_conf_oper();
    	dump_at_rx_buf();
    	break;
    case '1': //simulate a incoming SMS- to simulate all SMS commands.
        printf("simulate SMS\n");
        strncpy(phone_no, DEBUG_NUMBER,sizeof(phone_no));
        strncpy(msg_status, "REC UNREAD",sizeof(msg_status));
        index=8;
        yy=21;
        MM=1;
        dd=27;
        hh=15;
        mm=5;
        ss=6;
        tz=7;
        printf("enter sms body: \n");
        read_uart0(temp_str_100, sizeof(temp_str_100), 20);
        sprintf((char __far *)at_rx_buf,
        	"+CMGL: %d,\"%s\",\"%s\",\"\",\"%d/%d/%d,%d:%d:%d+%d\"\n/gc\n%s\r\n\r\nOK\r\n",
            index, msg_status, phone_no, yy, MM, dd, hh, mm, ss, tz,
            temp_str_100);
        //+CMGL: 26,"REC UNREAD",DEBUG_NUMBER,"","20/11/15,22:49:23+22"<CR><LF>10<CR><LF><CR><LF>OK<CR><LF>##

        dump_at_rx_buf();
        printf("calling process_cmgl_output\n");
        process_cmgl_output();
        activate_relays_based_on_oper_stat();
        break;
    case '2':
    	printf("Simulate a incoming call\n");
    	strncpy( phone_no, DEBUG_NUMBER, sizeof(phone_no));
    	sprintf((char *)at_rx_buf,
    			"+CLCC: 1,1,4,0,0,\"%s\",145,\"\"\n/gc\n\r\nOK\r\n",
				phone_no);
    	dump_at_rx_buf();
    	process_clcc_output();
    	//activate_relays_based_on_oper_stat();
    	break;
    case '3':// write to modem directly - anything.

        printf("ENTER at CMD to send to GSM modem directly\n");
	    	    	scanf("%s", temp_str_100);
        printf("u entered %s\n", temp_str_100);
        printf("\n/gc\n%s\r\n", temp_str_100);

        break;
    case '4':
    	provision_to_defaults();
    	/* date time sync from GSM network;
    	 * This writes to eeprom of the SIMCOM modem.
    	 * So, call this once when you insert the new modem chip.
    	 * do this manually.
    	 * * */
    	enable_date_time_sync_and_sms_storage_area();
    	break;
    case '5': // broadcast
    	broadcast_sms("broadcast sms");
    	break;
    case '6':
#if 0
        printf("Enter the value for the water level sensor data 0 to 9");
        read_uart0(temp_str_100, 1, 20);
        sscanf(temp_str_100, "%d", &sensor_level);
        printf("sensor_level=%d\n", sensor_level);
#endif
    case '7':
        provision_to_defaults();
        break;
    case '8':
    case '9':
    	break;
    case 'k':// reset mode,
    	reset_gsm_modem();
    	break;
    case 'l':
    	issue_cmgl_all();
    	break;
    case 'm':
    	process_cmgl_output();
    	break;
    case 'p':
        printf("=================> start:issue CSQ <== ================\n");
        write_to_modem("\n/gcCSQ\r");
        R_WDT_Restart();
        sleep_ms(2000);
        R_WDT_Restart();
        dump_at_rx_buf();
        printf("=================> end:issue CSQ  <== =================\n");
        break;
    case 'q':
        printf("issue CCLK\n");
        issue_cclk();
        break;
    case 'r':

        printf("parse CCLK\n");
        process_cclk_output();
        break;
    case 's':

        printf("RL0 ON \n");
        set_relay(0, 1);
        break;
    case 't':

        printf("\n/gc RL0 OFF ");
        set_relay(0, 0);
        break;
    case 'u':

        printf("\n/gc RL1 ON ");
        set_relay(1, 1);
        break;
    case 'v':

        printf("\n/gc RL1 OFF ");
        set_relay(1, 0);
        break;
    case 'w':

    case 'x':

        printf("\n/gccall store_to_dataflash cmd,p1,p2,p3,p4,p5:  ");
        scanf("%d,%d,%d,%d,%d,%d", &cmd, &param1, &param2, &param3,
              &param4, &param5);
        store_to_dataflash((text_command_t) cmd, &param1, &param2,
        		&param3, &param4, &param5);
        break;
    case 'C':

    	printf("\n/gcAT sending to MODEM");
        write_to_modem("\n/gcAT\r");

    	break;
    case 'D':
    	//printf("AT+CIPSEND");
    	//printf("AT+CSQ");
    	//printf("AT+CIPSTATUS");
    	break;
    default:
        printf("gcinvalid kb command\n");
        break;
    }
    printf("Return :process_keyboard\n");
}

//TODO: reset by writing to reg or calling proper driver api. now doing by long sleep...
void reset_mcu(int reset_reason) {
	prov.basic_prov.mcu_reset_reason = reset_reason;
	sleep_ms(20000);
}

//void R_WDT_Restart(void) { }
#if 0
void msleep(int milli_seconds)
{
	//printf("fff");
	volatile long long int i, j;
	for(i=0;i< 0xfff-1; i++) {
		for(j=0;j< 0xff; j++) {
		}
	}
}
#endif
