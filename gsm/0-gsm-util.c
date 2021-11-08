#include "0_all.h"

/* 0 if OK is found
 * 1 if ok is NOT found
*/
int look_for_ok_resp_timeout(char *buf, int ms)
{
	int i;
	int ret_val; // NO OK FOUND
	printf_uart_0_1 = UART_DEBUG;
    //printf("\rEnter:look_for_ok_resp_timeout(%d)", ms);

    for(i=0; i<ms; i+=300) {
    	//printf_uart_0_1 = UART_DEBUG;printf("\r i=%d", i);
    	if ( NULL == strstr((void *)buf, OK_RESP_STR)){
    		//printf("\rNO OK present");
    		ret_val = 1;
    	} else {
    		ret_val = 0;// FOUND OK
    	    break;
    	}
    	R_WDT_Restart(); sleep_ms_a(300);R_WDT_Restart();
    }

    if(0 == ret_val ) {
        printf("\rOK found");
    } else if(1 == ret_val ) {
        printf("\rOK _NOT_ Found");
    }
    //printf("\rReturn:look_for_ok_resp_timeout: ret_val=%d, ", ret_val);
    return ret_val;
}

void dump_at_rx_buf(void)
{
    int len, i;
    len = strlen(TYPE1 at_rx_buf);
	printf_uart_0_1 = UART_DEBUG;
	printf("\rat_rx_buf:");
    for(i = 0; i< len;i++){
    	if ( '\n' == at_rx_buf[i] ) {
    		printf("<LF>");
    	} else if ('\r' == at_rx_buf[i]) {
    		printf("<CR>");
    	} else {
    		printf("%c", at_rx_buf[i]);
    	}
    }
    printf("###\r");
}


void dump_at_rx_buf_len(void)
{
    printf_uart_0_1 = UART_DEBUG; printf(" at_rx_buf len=%d", strlen(TYPE1 at_rx_buf));
}

/*
 * 0 if modem reset completes
 * 1 if modem reset is NOT complete
 * */
int has_modem_reset_completed(char* buf)
{
	if (NULL != strstr(	buf, "\r\nCall Ready\r\n")) {
		if(NULL != strstr(buf, "\r\nSMS Ready\r\n")) {
			return 0; // reset completed
		}
	}
	return ~0; // reset is not complete
}

void reset_gsm_modem(void)
{
	int i=0;
	printf_uart_0_1 = UART_DEBUG; printf("\r reset_gsm_modem ");
	printf_uart_0_1 = UART_GSM; printf("\rAT+CFUN=1,1\r");
	printf_uart_0_1 = UART_DEBUG;
    while(i++ < MODEM_RESET_TIME ) {
    	dump_at_rx_buf();
    	 printf("\r i=%d, sleeping ", i);
    	if(0 == has_modem_reset_completed(TYPE2 at_rx_buf)) {
    		printf("\rReset has completed");
    		break;
    	}
    	R_WDT_Restart();sleep_ms_a(1000);
    }
}

void reset_at_rx_buf(void)
{
	//printf_uart_0_1 = UART_DEBUG;printf("\renter:reset_at_rx_buf");
    memset(TYPE_VOID_NP at_rx_buf, (int)NULL, AT_RX_BUF_SZ);
    R_UART1_Receive_non_blocking((uint8_t *)at_rx_buf, AT_RX_BUF_SZ - 2);
    /* TODO: reset the UART Interrupt */
}

void sms_list_cmgr(int index)
{
	printf_uart_0_1 = UART_DEBUG;  printf("\r %s(%d)", "sms_list_cmgr", index);
    reset_at_rx_buf();

	printf_uart_0_1 = UART_GSM; printf("\rAT+CMGR=%d,1\r", index);
   	look_for_ok_resp_timeout((char *)at_rx_buf, 2000);
}

#if 0
void sms_list_all(void)
{
	int ok_loop; // 10 seconds
	printf_uart_0_1 = UART_DEBUG;
    printf("\n%s", "sms_list_all");
    reset_at_rx_buf();

	printf_uart_0_1 = UART_GSM;
    printf("\nAT+CMGL=\"ALL\"\r"); /* delete all sms. This is safty case, what if sms remained... */

    for(ok_loop=0; ok_loop<1; ok_loop++) {
    	sleep_ms_a(1000);R_WDT_Restart();
    	sleep_ms_a(1000);R_WDT_Restart();
		printf_uart_0_1 = UART_DEBUG;
	    printf("\r ok_loop = %d, *=>%s<=*", ok_loop, at_rx_buf);
    	if ( 1 == look_for_ok_resp()){
    		printf_uart_0_1 = UART_DEBUG;
    	    printf("\r break");
    	    break;
    	} else {
    		printf_uart_0_1 = UART_DEBUG;
    	    printf("\r loop again");
    	}
    }

	printf("\nat_rx_buf=%s, len=%d", at_rx_buf, strlen(at_rx_buf));
}
#endif

void sms_list_all_cmgr(void)
{
	int index; // 10 seconds
	//printf_uart_0_1 = UART_DEBUG; printf("\r sms_list_all_cmgr");

    for(index = 1; index < 5; index++) {
		sms_list_cmgr(index);
	    dump_at_rx_buf();
    }
}
