#ifndef _0_TANK_SENSOR_H
#define _0_TANK_SENSOR_H

#include "0_all.h"

void process_tank_levels(void);
int read_sensor_val_from_hw(void);
void print_pin_input_status(char *prefix, unsigned char no_x);
void prepare_sms_water_level(unsigned int water_level);
extern unsigned int sensor_level;

#if MACHINE == RL78
#if 0
#define WS_LEVEL_1 P3_bit.no0
#define WS_LEVEL_2 P7_bit.no0
#define WS_LEVEL_3 P3_bit.no1
#define WS_LEVEL_4 P1_bit.no7
#else
#define WS_LEVEL_1 P14_bit.no7
#define WS_LEVEL_2 P1_bit.no0
#define WS_LEVEL_3 P1_bit.no1
#define WS_LEVEL_4 P1_bit.no2
#define WS_LEVEL_5 P1_bit.no3
#define WS_LEVEL_6 P1_bit.no4
#define WS_LEVEL_7 P1_bit.no5
#define WS_LEVEL_8 P1_bit.no6
#define WS_LEVEL_9 P1_bit.no7
#endif

#elif MACHINE == UBUNTU

#define WS_LEVEL_1 get_water_sensor_data(1)
#define WS_LEVEL_2 get_water_sensor_data(2)
#define WS_LEVEL_3 get_water_sensor_data(3)
#define WS_LEVEL_4 get_water_sensor_data(4)
#define WS_LEVEL_5 get_water_sensor_data(5)
#define WS_LEVEL_6 get_water_sensor_data(6)
#define WS_LEVEL_7 get_water_sensor_data(7)
#define WS_LEVEL_8 get_water_sensor_data(8)
#define WS_LEVEL_9 get_water_sensor_data(9)
#endif

#define ZERO 0
#define ONE 0
#define TWO 0
#define THREE 0
#define FOUR 4

#endif
