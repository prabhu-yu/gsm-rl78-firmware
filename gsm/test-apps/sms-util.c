#include "0-sms-util.h"


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
    char  address_text[20], service_center_time_stamp[20], address_type[20], sms_message_body_length[20];
#define format_index "%d,"
#define format_message_status "\"%[ A-Z]\","
#define format_address "%[-\"+_0-9A-Za-z],"
#define format_address_text "%[-\"+_0-9A-Za-z],"
#define format_service_center_time_stamp "\"%[-/,+_0-9+:]\""
#define format_address_type "%[0-9],"
#define format_sms_message_body_length "%[0-9],"
#define format_sms_message_body "%[\a-~]"
#define CRLF "\r\n"
    ret_val = sscanf(ptr_cmgl,"+CMGL:"
            format_index
            format_message_status
            format_address
            format_address_text
            CRLF
            format_sms_message_body
            CRLF,

            ptr_index, 
            message_status,
            address,
            address_text,
            sms_message_body
            );

    if(ret_val >= 2) {
        printf("good sms");
    } else {
        printf("corrupted sms listing. Should delete all.");
        return 1;
    }

    printf("\n ret_val = %d", ret_val);
    printf("\n 1 ptr_index = %d", *ptr_index);
    printf("\n 2 msg_status = %s", message_status);    
    printf("\n 3 address = %s", address);
    printf("\n 4 address_text = %s", address_text);
    //printf("\n 5 service_center_time_stamp = %s", service_center_time_stamp);
    //printf("\n 6 address_type = %s", address_type);
    //printf("\n 7 sms_message_body_length = %s", sms_message_body_length);
    printf("\n 8 sms_message_body = %s", sms_message_body);
    printf("\n");

    if (0 == strncmp(message_status, "REC UNREAD", sizeof("REC UNREAD"))) {
        printf(" this is a unread msessage");
        ret_val = sscanf(ptr_cmgl,"+CMGL:"\
            format_index
            format_message_status
            format_address
            format_address_text
            format_service_center_time_stamp
            CRLF
            format_sms_message_body
            CRLF
            "OK"
            CRLF,
            ptr_index, 
            message_status,
            address,
            address_text,
            service_center_time_stamp,
            sms_message_body
            );
    printf("\n ret_val = %d", ret_val);
    printf("\n 1 index = %d", *ptr_index);
    printf("\n 2 msg_status = %s", message_status);
    printf("\n 3 address = %s", address);
    printf("\n 4 address_text = %s", address_text);
    printf("\n 5 service_center_time_stamp = %s", service_center_time_stamp);
    //printf("\n 6 address_type = %s", address_type);
    //printf("\n 7 sms_message_body_length = %s", sms_message_body_length);
    printf("\n 8 sms_message_body = %s", sms_message_body);
    printf("\n");

    } else {
        printf(" this is not a unread msg");
    }

    return 0;
}


