#include "0_all.h"

void dump_date_time(time_t_mcu *date_time) {
    printf("date_time = %u-%u-%u T %u-%u-%u\n",
            date_time->year,
            date_time->month,
            date_time->month_day,
            date_time->hour,
            date_time->minutes,
			date_time->seconds);
}

