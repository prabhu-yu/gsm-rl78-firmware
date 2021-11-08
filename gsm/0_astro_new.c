#include "0_all.h"

#define M_PI   3.14159265358979323846
// Converts degrees to radians.
#define radians(angleDegrees) (angleDegrees * M_PI / 180.0)
// Converts radians to degrees.
#define degrees(angleRadians) (angleRadians * 180.0 / M_PI)

/*
 gcc 0_astro_new.c -lm && -o ./astro.exe
 */

void __from_format(double jd, char *fmt, signed long *jday_int, double *jday_fract) {
    /*
    Converts a Julian Day format into the "standard" Julian
    day format.
    Parameters
    ----------
    jd
    fmt
    Returns
    -------
    (jd, fractional): (int, float)
         A tuple representing a Julian day.  The first number is the
         Julian Day Number, and the second is the fractional component of the
         day.  A fractional component of 0.5 represents noon.  Therefore
         the standard julian day would be (jd + fractional + 0.5)
    */
//    if fmt.lower() == 'jd':
        //If jd has a fractional component of 0, then we are 12 hours into
        // the day

    printf("Enter >>> __from_format(jd = %f)\n", jd);
    *jday_int = (signed long)floor(jd + 0.5);
#if DEBUG
    printf("jday_int: %ld , its double format= %f\n", *jday_int, floor(jd + 0.5));
#endif
    *jday_fract = jd + 0.5 - floor(jd + 0.5);
#if DEBUG
    printf("jday_fract=%f\n", *jday_fract);
#endif

#if 0
    elif fmt.lower() == 'mjd':
        return __from_format(jd + 2400000.5, 'jd')
    elif fmt.lower() == 'rjd':
        return __from_format(jd + 2400000, 'jd')
    else:
        raise ValueError('Invalid Format')
#endif
}

double from_jdate(double jdate, char *fmt, time_t_mcu *result_time) {

    signed long jdate_int_part;
    double jdate_fract_part;
#if DEBUG
    printf("Enter >>> from_jdate(jdate = %f)\n", jdate);
#endif
    __from_format(jdate, NULL, &jdate_int_part, &jdate_fract_part);

#if DEBUG
    printf("jdate_int_part= %ld  jdate_fract_part= %f\n",jdate_int_part, jdate_fract_part );
#endif
    signed long l = jdate_int_part + 68569L + 2451545L;

#if DEBUG
    printf("l = %ld \n", l);
#endif
    signed long n = floor(4*l / 146097.0);

#if DEBUG
    printf("n = %ld \n", n);
#endif

    l = floor(l- (146097L*n + 3) / 4.0);
#if DEBUG
    printf("l = %ld \n", l);
#endif

    signed long i = floor(4000*(l+1)/1461001.0);
#if DEBUG
    printf("i = %ld \n", i);
#endif

    l = floor(l - 1461*i/4.0 + 31);
#if DEBUG
    printf("l = %ld \n", l);
#endif

    signed long j = floor(80 * l/2447.0);
#if DEBUG
    printf("j = %ld\n", j);
#endif

    signed long k = l - floor(2447 * j/80.0);
#if DEBUG
    printf(" k = %ld \n", k);
#endif

    l = floor(j/11.0);
#if DEBUG
    printf(" l = %ld \n", l);
#endif

    j = j+2-12*l;
#if DEBUG
    printf(" j = %ld \n", j);
#endif

    i = 100*(n-49)+i+l;
#if DEBUG
    printf(" i = %ld \n", i);
#endif

    int year = (int)i;
    int month = (int)j;
    int day = (int)k;
#if DEBUG
    printf("year:month:day= %d: %d: %d\n", year, month, day);
#endif

// need micro seconda accuracy? not on micro controller!
//#define MS_ACC 1e6
#define MS_ACC 1L
#define HOURS_24 24L
#define MINUTES_60 60L
#define SECONDS_3600 3600L

    signed long frac_component = (signed long)(jdate_fract_part * (signed long)(MS_ACC * HOURS_24 * SECONDS_3600));
#if DEBUG
    printf("frac_component = %ld\n", frac_component);
#endif

    int hours = floor( frac_component / (MS_ACC * SECONDS_3600));
#if DEBUG
    printf("hours = %d \n", hours);
#endif

    frac_component -= hours * MS_ACC * SECONDS_3600;
#if DEBUG
    printf("frac_component = %ld \n", frac_component);
#endif

    int minutes = floor(frac_component / (MS_ACC * 60L));
    frac_component -= minutes * MS_ACC * MINUTES_60;

    int seconds = floor(frac_component / MS_ACC);
    frac_component -= seconds * MS_ACC;

    frac_component = (signed long)(frac_component);

#if DEBUG
    printf("yyyy-mm-dd hh:mm:ss  %d-%d-%d T %d:%d:%d \n",
            year, month, day,
            hours, minutes, seconds );
#endif

    result_time->year = year;
    result_time->month = month;
    result_time->month_day = day;
    result_time->hour = hours;
    result_time->minutes = minutes;
    result_time->seconds = seconds;

    return 0;
}

double to_std_jdate(time_t_mcu *time_now) {
	/*
	   Converts a given datetime object to Julian date.
	   Algorithm is copied from https://en.wikipedia.org/wiki/Julian_day
	   All variable names are consistent with the notation on the wiki page.
	   Parameters
	   ----------
	   fmt
	   date_time->: datetime
	   Datetime object to convert to MJD
	   Returns
	   -------
	   jd: float
	 */
	double a = floor((14 - time_now->month) / 12.0);
	double y = time_now->year + 4800 - a;
	double m = time_now->month + 12 * a - 3;

	double jdn =
	    time_now->month_day + floor((153 * m + 2) / 5.0) + 365 * y +
	    floor(y / 4.0) - floor(y / 100.0) + floor(y / 400.0) - 32045;
	//printf("time_now: %d:%d:%d", time_now->tm_hour, time_now->tm_min, time_now->tm_sec);
	double fract =
	    (time_now->hour - 12) / 24.0 + time_now->minutes / 1440.0 +
	    time_now->seconds / 86400.0;
	double jdate = jdn + fract;

	//printf("\n jdn=%f, fract=%f, jd=%f", jdn, fract, jd);
	return jdate;
}

void sunrise_sunset_calc(astro_prov_t const *sun, astro_oper_t *astro_oper) {
    printf("sunrise_sunset_calc() \n");
	double jdate = to_std_jdate(&astro_oper->time_now);
#if DEBUG
	printf("jdate = %f \n", jdate);
#endif
	//double n = jdate - 2451545 + .0008;
	signed long n = floor(jdate - 2451545);
#if DEBUG
	printf(" n=%ld \n", n);
#endif
	double mean_solar_time_jstar = n - (sun->longitude / 360.0);
#if DEBUG
	printf(" mean_solar_time_jstar=%f \n", mean_solar_time_jstar);
#endif
	double solar_mean_anamoly_M =
	    fmod(357.5291 + 0.98560028 * mean_solar_time_jstar, 360.0);
#if DEBUG
	printf("solar_mean_anamoly_M=%f \n", solar_mean_anamoly_M);
#endif
	double equation_of_center =
	    1.9148 * sin(radians(solar_mean_anamoly_M)) +
	    0.0200 * sin(radians(2 * solar_mean_anamoly_M)) +
	    0.0003 * sin(radians(3 * solar_mean_anamoly_M));
#if DEBUG
	printf("equation_of_center= %f \n", equation_of_center);
#endif
    double ecliptic_langitude_lambda = fmod(solar_mean_anamoly_M + equation_of_center + 180 +102.9372, 360);
#if DEBUG
    printf(" ecliptic_langitude_lambda=%f \n",ecliptic_langitude_lambda);
#endif
    double solar_transit_j_transit = /*2451545.0*/0 + mean_solar_time_jstar + \
        0.0053*sin(radians(solar_mean_anamoly_M)) - \
        0.0069*sin(radians(2*ecliptic_langitude_lambda));
#if DEBUG
    printf(" solar_transit_j_transit=%f \n",solar_transit_j_transit);
#endif
    double declination_of_sun = degrees(asin (
        sin(radians(ecliptic_langitude_lambda)) * \
        sin(radians(23.44))));
#if DEBUG
    printf(" declination_of_sun= %f \n",declination_of_sun);
#endif
    double hour_angle_w_sunrise = degrees(acos(
        (sin(radians(sun->sunrise_solar_elevation_angle)) - sin(radians(sun->lattitude)) * sin(radians(declination_of_sun)))/\
        (cos(radians(sun->lattitude)) * cos(radians(declination_of_sun)))));
#if DEBUG
    printf(" sunrise hour_angle_w_sunrise = %f \n", hour_angle_w_sunrise);
#endif
    double jdate_rise = solar_transit_j_transit - hour_angle_w_sunrise/360.0;
#if DEBUG
    printf(" jdate_rise = %f \n",jdate_rise);
#endif
    // ADD INDIA TIME Zone 5 hrs 30 mins = 5.5 part of a day
    from_jdate(jdate_rise + 5.5/24.0 + /*2451545L*/0L, "", &astro_oper->sunrise_time);

    double hour_angle_w_sunset = degrees(acos(
        (sin(radians(sun->sunset_solar_elevation_angle)) - sin(radians(sun->lattitude)) * sin(radians(declination_of_sun)))/\
        (cos(radians(sun->lattitude)) * cos(radians(declination_of_sun)))));
#if DEBUG
    printf(" sunset hour_angle_w_sunset = %f \n", hour_angle_w_sunset);
#endif
    double jdate_set = solar_transit_j_transit + hour_angle_w_sunset/360.0;
#if DEBUG
    printf(" jdate_set = %f \n", jdate_set);
#endif
    // ADD INDIA TIME Zone 5 hrs 30 mins = 5.5 part of a day
    from_jdate(jdate_set + 5.5/24,  "", &astro_oper->sunset_time);
}

void astro_prov_copy(astro_oper_t *dest, astro_oper_t *src) {
    //dest->sunrise_time.tm_yday = src->sunrise_time.tm_yday;
    return;
}

/* returns 1 if changed and Copy to local memory before returning
 * returns 0 if NOT changed */
unsigned int are_astro_parameters_changed(
        astro_prov_t *p_local_prov, astro_prov_t *p_global_prov,
		time_t_mcu *p_local_time,       time_t_mcu *p_global_time) {
    //printf("lat = %f and %f\n", p_local_prov->lattitude, p_global_prov->lattitude);
    if (0 == memcmp( p_local_prov, p_global_prov, sizeof(astro_prov_t))) {
        printf("No prov changed\n");
    } else {
        printf("prov changed, we copied it \n");
        memcpy(p_local_prov, p_global_prov, sizeof(astro_prov_t));
        return 1;
    }
    //dump_date_time(p_local_time);
    //dump_date_time(p_global_time);
    if (0 != memcmp( &p_local_time->year, &p_global_time->year,	sizeof(p_global_time->year) + sizeof(p_global_time->year) + sizeof(p_global_time->year))){
        printf("yyyy or mm or dd changed \n");
        return 1;
    } else {
        printf("yyyy or mm or dd NOT changed \n");
    }
    //printf("\n Nothing changed");
    return 0;
}

static void set_relay_oper_status_due_astro_timer(unsigned int relay, relay_status_t relay_status) {
	char temp_str[50];
    printf("Enter: set_relay_oper_status_due_astro_timer(relay=%d, relay, relay_status=%d)\n", relay, relay_status);
	snprintf(temp_str, sizeof(temp_str)-1, "AstroTimer\nSwitch-%d : %s\n", relay, relay_status?"ON":"OFF");
    //TODO lennght should not exceed 130 bytes ...
    //printf("\nA.===> sms= temp_str=%s\nsend_sms_body_160=%s<===", temp_str, send_sms_body_160);
    //printf("send_sms_body_160.a=%s", send_sms_body_160);
    send_sms_flag = true;
    strncat(send_sms_body_160, temp_str, sizeof(temp_str));
    printf("send_sms_body_160.b=%s", send_sms_body_160);
    //printf("\nB.===>  sms= temp_str=%s\nsend_sms_body_160=%s<===", temp_str, send_sms_body_160);
	send_sms_flag = activate_relays_based_on_oper_stat_flag = true;
	set_relay_oper_status(relay, relay_status);
	printf("Return :set_relay_oper_status_due_astro_timer()\n");
    return;
}

void append_on_off_time(prov_t *p_prov, oper_t *p_oper, char *local_send_sms_body_160) {
    printf("Enter: append_on_off_time\n");
    char temp_str[100];
    printf("1. local_send_sms_body_160=%s", local_send_sms_body_160);

    time_t_mcu *my_time = &p_oper->astro_oper.sunset_time;
    snprintf(temp_str, sizeof(temp_str), "ON TIME: %02u:%02u:%02u\n",
        my_time->hour, my_time->minutes, my_time->seconds);
    strncat(local_send_sms_body_160, temp_str, sizeof(temp_str));

    my_time = &p_oper->astro_oper.sunrise_time;
    snprintf(temp_str, sizeof(temp_str), "OFF TIME: %02u:%02u:%02u\n",
        my_time->hour, my_time->minutes, my_time->seconds);
    strncat(local_send_sms_body_160, temp_str, sizeof(temp_str));

    printf("2. local_send_sms_body_160=%s", local_send_sms_body_160);
    printf("Return : append_on_off_time\n");
}


int process_astro1(prov_t *p_prov, oper_t *p_oper) {
    static int astro_first_power_up = 1; // during power up, all realys are off, so  just to disting wish this.
    printf("Enter: process_astro1 \n");
    static astro_prov_t astro_prov_local;
    int changed = are_astro_parameters_changed (
    		&astro_prov_local, &p_prov->astro_prov,
			&p_oper->astro_oper.time_now, &p_oper->time_now);
    switch(changed) {
        case 0: // nothing changed; do nothing.
            printf("\n No Re-caclulate sunrise/sunset ");
            break;
        default: // changed
            printf(" copy + Re-caclulate sunrise/sunset \n");
            //memcpy(&astro_prov_local, &p_prov->astro_prov, sizeof(astro_prov_t));
            printf("Now time is \n");
            dump_date_time(&p_oper->time_now);
            p_oper->astro_oper.time_now.year = p_oper->time_now.year;
            p_oper->astro_oper.time_now.month = p_oper->time_now.month;
            p_oper->astro_oper.time_now.month_day = p_oper->time_now.month_day;
            p_oper->astro_oper.time_now.hour = 12; // why 12 hrs?
            p_oper->astro_oper.time_now.minutes = 0;
            p_oper->astro_oper.time_now.seconds = 0;
            sunrise_sunset_calc(&p_prov->astro_prov, &p_oper->astro_oper);
            dump_date_time(&p_oper->astro_oper.sunrise_time);
            dump_date_time(&p_oper->astro_oper.sunset_time);
            break;
    }

    printf("Present Time: "); dump_date_time(&p_oper->time_now);
    printf("sunrise Time: "); dump_date_time(&p_oper->astro_oper.sunrise_time);
    printf("sunset Time: ");  dump_date_time(&p_oper->astro_oper.sunset_time);

    if (memcmp(&p_oper->time_now, &p_oper->astro_oper.sunrise_time, sizeof(time_t_mcu)) < 0) {
        printf("before sunrise\n");
        if(relay_on != get_relay_oper_status(0)) {
            printf("turn ON \n");
            set_relay_oper_status_due_astro_timer(0, relay_on);
            append_on_off_time(p_prov, p_oper, send_sms_body_160);
        } else {
            printf("already ON\n");
        }
        printf("Return: process_astro1 B\n");
        return 0;
    } else {
        printf("Not before sunrise \n");
    }

    if (memcmp(&p_oper->time_now, &p_oper->astro_oper.sunset_time, sizeof(time_t_mcu)) < 0)  {
        printf("after sun rise but before sunset so \n");
        if ((astro_first_power_up == 1) || (relay_off != get_relay_oper_status(0))) {
            astro_first_power_up = 0;
            printf("turn OFF \n");
            set_relay_oper_status_due_astro_timer(0, relay_off);
            append_on_off_time(p_prov, p_oper, send_sms_body_160);
        } else {
            printf("already OFF \n");
        }
        printf("Return: process_astro1 H\n");
        return 0;
    } else {
        printf(" not after sunrise \n");
    }

    printf(" after sun set so \n");
    if(relay_on != get_relay_oper_status(0)) {
        printf("turn ON\n");
        set_relay_oper_status_due_astro_timer(0, relay_on);
        append_on_off_time(p_prov, p_oper, send_sms_body_160);
    }else {
        printf("already ON\n");
    }

    printf("Return: process_astro1 A\n");
    return 0;
}

void astro_test(void) {
    printf(" Enter: astro_test \n");
    process_astro1(&prov, &oper);
    printf(" Return: astro_test \n");
    return;

    unsigned int hour, month_day;
    for (month_day=1; month_day<2; month_day++) {
        for(hour = 0; hour < 1; hour++) {
            printf("\n _____________________________");
            printf("\n day = %u, hour = %u", month_day, hour);
            oper.time_now.hour = hour;
            oper.time_now.month_day = month_day;
            snprintf(oper.time_now.str_time, sizeof(oper.time_now.str_time)+1,
                    "%04u-%02u-%02uT%02u:%02u:%02u+05:30",
                    oper.time_now.year, oper.time_now.month,   oper.time_now.month_day,
                    oper.time_now.hour, oper.time_now.minutes, oper.time_now.seconds);
            process_astro1(&prov, &oper);
            R_WDT_Restart();
#if MACHINE == RL78
            sleep_ms(100);
#endif
        }
    }
    printf(" Return: astro_test \n");
}

