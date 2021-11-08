#ifndef _MY_GSM_UTIL_H
#define _MY_GSM_UTIL_H

#include "0_all.h"

//gsm modem reset time
#define MODEM_RESET_TIME 120
int look_for_ok_resp_timeout(char *buf, int ms);

#endif
