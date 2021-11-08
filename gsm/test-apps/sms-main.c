#include<stdio.h>
#include <string.h>

#include "0-sms-util.h"

void sms_delete_index_based(int index)
{
    printf("deleting sms at index=%d ", index);
}


int main(void)
{
    char *ptr_cmgl="+CMGL: 1,\"STO UNSENT\",\"\",\"\",._Timer: Switch0 -> ON_Timer: Switch1 -> ON_POWER UP_FIXED_TIMER_TIME: 00:00:17_._._+CMGL:";
    ptr_cmgl="+CMGL: 26,\"REC UNREAD\",\"+DEBUG_NUMBER\",\"\",\"20/10/10,13:39:09+22\"\r\n1\r\n\r\nOK\r\n";
    int index;
    char message_status[20];
    char address[20];
    int yy,MM,dd,hh,mm,ss,tz;
    char sms_message_body[160];
    int ret_val;

    ret_val = sms_parse(
            ptr_cmgl,
            &index, 
            message_status,
            address,
            &yy, &MM, &dd, &hh ,&mm, &ss, &tz,
            sms_message_body
            );
    if(0 == ret_val) {
        sms_delete_index_based(index);
    }
}
