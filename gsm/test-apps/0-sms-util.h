#include<stdio.h>
#include <string.h>

void sms_delete_index_based(int index);
int sms_parse(
        char *ptr_cmgl,
        int *ptr_index,
        char message_status[],
        char address[],
        int *ptr_yy,int *ptr_MM,int *dd,int *hh,int *mm,int *ss,int *tz,
        char sms_message_body[]
        );

