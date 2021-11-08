#ifndef _ALL_H
#define _ALL_H

#define RL78 1
#define UBUNTU 2

#define DEBUG_NUMBER "YOUR MOB NO"

#if MACHINE == RL78
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_pfdl.h"
#include "r_cg_port.h"
#include "r_cg_serial.h"
#include "r_cg_wdt.h"
#include "r_cg_timer.h"

#elif MACHINE == UBUNTU

#define __near 
#define __far

typedef unsigned short MD_STATUS;
typedef void pfdl_status_t;
/*==============================================================================================*/
/* unsigned type definitions                                                                    */
/*==============================================================================================*/
typedef unsigned char                       pfdl_u08;
typedef unsigned int                        pfdl_u16;
typedef unsigned long int                   pfdl_u32;

#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <pthread.h> 
#include <unistd.h> 

#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

#include "0_macrodriver.h"
#include "0_serial.h"
#include "0_ubuntu.h"

#endif

#include "0_app.h"
#include "0_app_dump.h"
#include "0_gsm.h"
#include "0_uart.h"
#include "0_relay.h"
#include "0_misc.h"
#include "0_dataflash.h"
#include "0_gsm_util.h"
#include "0_astro_new.h"
#include "0_astro.h"
#include "0_at_commands.h"
#include "0_tank_sensor.h"
#include "0_fixed_timer.h"
#include "0_sms_util.h"
#include "0_i2c.h"

extern volatile unsigned int reset_source;
#endif
