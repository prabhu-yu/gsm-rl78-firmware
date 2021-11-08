/*
 * 0_app.h
 *
 *  Created on: Jan 16, 2021
 *      Author: prabhu
 */

#ifndef _0_APP_H_
#define _0_APP_H_

#include <stdio.h>
#include <math.h>
#include <string.h>

#if MACHINE == UBUNTU
#define AT_SERVER_UDP_PORT 5555
#endif

#undef  offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define TYPE1 (const char __near *)
#define TYPE2 (char __near *)

#define TYPE_CONST_CHAR_NP (const char __near *)
#define TYPE_CONST_CHAR_P (const char *)
#define TYPE_CHAR_NP (char __near *)
#define TYPE_CHAR_P (char *)
#define TYPE_VOID_NP (void __near *)
#define TYPE_VOID_P (void *)
#define FLOAT_TO_INT(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))
#define KEYBOARD_EN
//#define DISABLE_SMS

// TODO: remove all printf. instead use snprintf and dsend_uart. thus we are free from redirecting?
//TODO: Why two things below for same meaning?
#define at_buf_sz 300
#define AT_RX_BUF_SZ 1000

#define TRUE 1
#define FALSE 0
#define TRUE_A (~(FLASE))
#define CRLF "\r\n"

typedef enum{
	manual_switch        = 0,
	fixed_timer_switch   = 1,
	astro_timer_switch   = 2,
	motor_starter        = 3,
	water_sensor_tanks_1 = 4,
	motor_starter_water_sensor_tanks_1 = 5,
	motor_starter_water_sensor_tanks_2 = 6,
	school_timer                       = 7,
	invalid_max_device_mode            = 8
} device_type_t;

typedef enum {
	const_mcu_boot_delay_max = 60*2,
	const_mcu_boot_delay_min = 3,
	keyboard_enabled = 1,
	sms_reply_enabled = 1,
	sms_reply_disabled = 0,
    SMS_DEL_CNT = 60*30,
    GSM_RESET_CNT = 60*10,
    GSM_INIT_CNT = 20,
	MY_SMS_LENGHT = 50
} const_default_t;

typedef enum {
	starter_mode_automatic = 0,
	starter_mode_temporary = 1,
	starter_mode_timer     = 2,
	starter_mode_duration  = 3,
    starter_mode_invalid   = 4,
}starter_mode_t;

typedef enum {
    NONE,
    CMGL,
    CLIP
} resp_type_t;

typedef enum {
    off=0,
    on=1,
    false=0,
    true=1,
    bool_invalid = 2
} bool_t;

typedef enum {
    relay_off=0,
    relay_on=1,
    relay_on_hold_off=2}
relay_status_t;

typedef enum {
    mcu_reboot,
    new_text_command_received,
    new_phone_call_received
} new_event_t;

typedef enum {
    //off, on are implicit
    //Global settings
    cmd_write_device_type                          = 11,//cmd*device_type_t
    cmd_get_status                                 = 12,//dump all prov struct in 1 or two sms in NAME:VALUE pair
    cmd_reset_mcu                                  = 13,
    cmd_reset_prov                                 = 31985,
    cmd_set_mcu_boot_delay                         = 14,
    cmd_set_date_time                              = 15,
    cmd_write_time_zone                            = 16,
    cmd_write_password                             = 17,
    cmd_write_sms_reply_enabled                    = 18,
	cmd_read_sw_hw_rev                             = 19,
    // WATER LEVEL RELATED-------------------------------------------
    cmd_write_water_level_soak_time                = 30,// cmd*WATER_LEVEL_SOAK_SECONDS
    cmd_write_water_level_max_level                = 31,// cmd*WATER_LEVEL_MAX_LEVEL
    // phone number RELATED------------------------------------------
    cmd_write_add_phone_number                     = 50,//cmd*index*DEBUG_NUMBER
    cmd_write_delete_phone_number                  = 51,//cmd*index
    cmd_read_phone_numbers                         = 52,
    // Fixed Timer related------------------------------------------
    cmd_write_light_24h_timer_table                = 60,//cmd*index*hh*mm*ss*rly_status
    cmd_write_light_24h_timer_table_prov_max       = 62,//cmd*prov_max
    cmd_read_light_24h_timer_table                 = 63,
    // Motor Related-----------------------------------------------
    cmd_write_starter_mode                         = 70,// cmd*motor_N*starter_mode_t (For 0=auto, 1= temp)
    cmd_write_starter_status                       = 71,// cmd*motor_N*0|1  ( for on/off)
    cmd_write_starter_n_call_aware                 = 72,// cmd*motor_N
    cmd_write_starter_start_stop_relay_hold_time   = 73,// cmd*start_hold_sec*stop_hold_sec
    //Manual switch related------------------------------------------
    cmd_write_manual_switch_status_provision       = 80,//cmd*switch_No*off_on
    // Astro Timer related------------------------------------------
    cmd_write_lat_long                             = 90,//cmd*lattitude*longitude
    cmd_write_sun_elevation_rise_set               = 91,//cmd*sun_elevation_rise*sun_elevation_set
    cmd_read_astro_prov                            = 92,
    // Last value- keep updated------------------------------------------
    cmd_max                                        = 100
} text_command_t ;

typedef struct {
    unsigned int year; // year in yyyy 2020
    unsigned int month; //month of year [1,12]
    unsigned int month_day;//day of month [1-31]
    unsigned int hour; //hour [0-23]
    unsigned int minutes;//minutes [0-59]
    unsigned int seconds;//seconds [0-59]
    //yyyy-mm-ddThh:mm:ss+05:30 => 25 + null = 26 spaces
    char str_time[5 + 3 + 3 + 1 + 3 + 3 + 3 + 1+ 5 + 1 ];
} time_t_mcu;

typedef struct {
    uint8_t hour;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t relay_status; // on, off, no_action
} relay_time_t_mcuable_t;

//TODO make the following struct differently
#define MAX_RELAYS 2
#define MAX_STARTERS (MAX_RELAYS/2)

typedef  struct{
    uint8_t start_relay_hold_duration;
    uint8_t stop_relay_hold_duration;
    uint8_t starter_n_call_aware;
    uint8_t starter_mode[MAX_STARTERS]; //TEMP(0) always_on(1) always_off(2) motor_timer(3) motor_duration(3)
    uint8_t starter_status[MAX_STARTERS]; //TEMP(0) always_on(1) always_off(2) motor_timer(3) motor_duration(3)
} starter_prov_t;

// +4coutnry code + 10digit NULL = 15
#define phone_no_size 15
#define phone_user_name_size 15
#define phone_no_list_sz 10
#define relay_time_t_mcuable_sz 6
typedef  struct{
    uint8_t valid;
    char phone_no[phone_no_size];
    char name[phone_user_name_size];
    uint8_t notify_type;
    /* 0 - only sms.
     * 1 - sms + call/ring when needed.
     * 2 - No-notification
    */
    uint8_t user_type;
    /*0: read_write user
     *1: read_only user
     *2: it is a motor, so, we should send proper turn ON/OFF commands
    */
} phone_book_t;

typedef struct {
    uint16_t written_n;
    /* how many times the data flash is written?
     * Always keep this as first byte.
     * */
    uint8_t device_type;/* FixTimer(1), AstroTimer(2) Motor(3) water_sensor_tanks_1(4)
                           motor_WaterSensor_Tanks_1(5) motor_WaterSensor_Tanks_2(5) */
    uint8_t mcu_reset_reason;
    uint8_t mcu_boot_delay;
    uint8_t security;
    uint8_t ccode[4];// country code. +nnn
    //uint32_t devSign; // device signatures such as model, variant, revision all clubbed
    //uint32_t slNo; //serial number from my company
    //uint8_t key; // programmed key; read it in s/w to verify key is really programmed
    uint32_t voltage_monitor_enabled;
    uint16_t over_volt_mark;
    uint16_t under_volt_mark;
    uint16_t write_cnt;
    uint16_t banner_delay;
    uint8_t sms_reply_enabled;
    uint8_t max_rl_tt_provisioned;
} basic_prov_t;

typedef struct {
    uint16_t water_level_soak_seconds;
    uint8_t max_level;
    uint8_t turn_off_motor_at_this_level; // genrally turn on upon full
    uint8_t turn_on_motor_at_this_level; // turn off upon empty or some lower levels
    uint8_t only_turn_off;// I should not turn ON. i am allowed to turn on only
} water_level_prov_t;

typedef struct {
    double lattitude;
    double longitude;
    double sunrise_solar_elevation_angle;
    double sunset_solar_elevation_angle;

    //float solar_altitude_angle_dawn_generic_use;
    //float solar_altitude_angle_dusk_generic_use;
    //float solar_altitude_angle_2_relays_midnight_on[1+1];// 2 relays, [0]=>relay_1, [1]=>relay[2]
    //float solar_altitude_angle_2_relays_midnight_off[1+1+1+1];
    /* 2 relays, [0]=>relay_1,
     *           [1,2,3]=>relays2 on_at_dusk, off_at_late_night, on_at_earlymorning, off_at_dawn
     *           */

    /* Solar elevation Angle=Solar Altitude=SA= geometric centre of sun below horizon. Search google to know this more...
     * SA=-0.83 deg for sunrise/sunset,
     * SA=-6.0 deg for civil twi-light aka Dawn, Dusk
     * SA=-4.0 I choose this for after various trail and error for Rural/open environment
     * SA=-3.0 I choose this for after various trail and error for City/congested/high rise building life
     * */

    int8_t tz_sign; // sign of UTC offset +1 or -1
    int16_t tz_hh;// UTC offset
    //int16_t tz_mm;//UTC offset
    int16_t tz_mm;//UTC offset
    //float gc_sun_user; // user defined civil dawn and dusk
    //uint16_t yy,mm,dd; // Device Manufactured date. program will always verify that RTC can't have date before this.
} astro_prov_t;

typedef struct {
    basic_prov_t        basic_prov;
    water_level_prov_t  water_level_prov;
    phone_book_t        phone_book[phone_no_list_sz];
    relay_time_t_mcuable_t  rl_tt[relay_time_t_mcuable_sz];
    uint8_t             manual_switch_status[MAX_RELAYS];
    starter_prov_t      motor_prov_obj;
    astro_prov_t        astro_prov;
} prov_t;

#define DEBUG 0
typedef struct {
	time_t_mcu time_now;
	time_t_mcu sunrise_time;
	time_t_mcu sunset_time;
} astro_oper_t;

typedef struct {
    relay_status_t relay_status[MAX_RELAYS];
    bool_t starter_status[MAX_STARTERS];
    // Water level
    unsigned int current_water_level;
    unsigned int previous_water_level;
    unsigned int water_level_sms_sent;
    //unsigned int currently_soaked_level;
    unsigned int water_level_soaked_seconds;
    //time1_t time_now;
    time_t_mcu time_now;
    int manual_switch_status[MAX_RELAYS];
    astro_oper_t astro_oper;
} oper_t;

extern prov_t prov;
extern oper_t oper;

#endif /* 0_APP_H_ */
