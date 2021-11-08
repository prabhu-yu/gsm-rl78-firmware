
#include "0_all.h"

uint8_t at_buf[at_buf_sz];
//const char __near *s1

/* * after 10 seconds of pwr up
 * 0xD CR
 * 0xA LF
 * * */

void init_sim(void)
{
	UART0_print_string((uint8_t * __near)"inside init_sim");
    UART0_print_string((uint8_t * __near)"sending ATE0");

    at_buf[299] = '\0';
    R_UART0_Receive_non_blocking(at_buf, 299);
#undef str
#define str "ATE0\n"
#define sz (sizeof(str)-1)
    R_UART1_Send_blocking((uint8_t * __near)str, sz); // sms text format
    sleep_ms(1000);

#ifdef D
#undef str
#define str "\r\nOK\r\n"
#define sz (sizeof(str)-1)
    memcpy(at_buf, str, sz);
#endif

    UART0_print_string((uint8_t * __near)"\n");
    R_UART0_Send_blocking((uint8_t * __near)at_buf, sz);

    if(0 == memcmp(str, at_buf, sz)) {
        UART0_print_string((uint8_t * __near) "\ngot OK 1");
    } else {
        UART0_print_string((uint8_t * __near) "\nNo OK 1");
    }
//------------------------------------------------------------
    UART0_print_string((uint8_t * __near) "\nsending AT");
    R_UART0_Receive_non_blocking(at_buf, 299);
#undef str
#define str "AT\r\n"
#define sz (sizeof(str)-1)
    R_UART0_Send_blocking((uint8_t * __near)str, sz);// sms text format
    sleep_ms(1000);

#ifdef D
#undef str
#define str "\r\nOK\r\n"
#define sz (sizeof(str)-1)
    memcpy(at_buf, str, sz);
#endif
    UART0_print_string((uint8_t * __near)"\n");
    R_UART0_Send_blocking(at_buf, sz);

    if(0 == memcmp(str, at_buf, sz)) {
        UART0_print_string((uint8_t * __near)"got OK 2");
    } else {
        UART0_print_string((uint8_t * __near)"No OK 2");
    }
//-------------------------------------------------------------------------
    UART0_print_string((uint8_t * __near)"\nsending +CGMF");
    R_UART0_Receive_non_blocking(at_buf, 299);
#undef str
#define str  "AT+CMGF=1\r\n"
#define sz (sizeof(str)-1)
    R_UART0_Send_blocking((uint8_t * __near)str, sz);
    sleep_ms(1000);
#ifdef D
#undef str
#define str "\r\nOK\r\n"
#define sz (sizeof(str)-1)
    memcpy(at_buf, str, sz);
#endif
    UART0_print_string((uint8_t * __near)"\n");
    R_UART0_Send_blocking(at_buf, sz);

    if(0 == memcmp(str, at_buf, sz)) {
        UART0_print_string((uint8_t * __near)"got OK 3");
    } else {
        UART0_print_string((uint8_t * __near)"No OK 3");
    }

#ifdef D
#undef str
#define str "\r\nOK\r\n"
#define sz (sizeof(str)-1)
    memcpy(at_buf, str, sz);
#endif

}


/* Seraches for first valid AT cmd header and moves all the content to the
 *  if found gives the pointer of the starning msg *
 * */

void parse_at_resp_buffer(void)
{
#undef str
#define str "handle the"
	R_UART0_Send_blocking((uint8_t *) "parse ", 5);
	R_UART0_Send_blocking(at_buf, sizeof(at_buf));
    if(0 == memcmp("+CMGL:", at_buf, sizeof("+CMGL:")-1)) {
        UART0_print_string((uint8_t * __near)"got +CMGL:");
    } else {
        UART0_print_string((uint8_t * __near)"No +CMGL:");
    }
    memset(at_buf, 0, sizeof(at_buf));
}

void issue_at_cmds(void )
{
	// issue sms read a1ll cmd
	// issue current call list
	R_UART0_Send_blocking((uint8_t *) "issue", 5 );
#undef str
#define str "AT+CMGL=\"ALL\""
	R_UART0_Send_blocking((uint8_t * __near)str, sizeof(str)); // read all sms
#undef str
#define str "AT+CMGL=\"ALL\"\n"
	R_UART0_Send_blocking((uint8_t * __near)str, sizeof(str)); // read all sms
}
