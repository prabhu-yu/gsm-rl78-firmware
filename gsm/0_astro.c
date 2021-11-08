#if 0
#if 0
#include "ds3231m.h"
#include "astro.h"
#include "volt.h"
#include "lcd.h"
#include "eeprom-utils.h"
#include "uart-utils.h"
#include "relay-control.h"
#include "astro-basic-defs.h"
#endif

#include "0_all.h"

//#if 0
char on_date[17], off_date[17];
char on_time[17], off_time[17];
//struct sunrise_sunset_time_t_mcu sun_e_rise_set;

float calendar_to_jdn(uint32_t year, uint32_t month, uint32_t day)
{
    float a = floor((14 - month) / 12);
    float y = year + 4800 - a;
    float m = month + 12 * a - 3;
    return day + floor((153 * m + 2) / 5) + 365 * y + floor(y / 4) - floor(y / 100) + floor(y / 400) - 32045;
}

void jd_to_calendar(uint32_t j, unsigned int yyyymmdd_array[])
{
	//printf("\n %s  j = %u", __FUNCTION__,(unsigned int)j);
	j = j - 1721119;

	uint32_t y = (4 * j - 1) / 146097;
	j = 4 * j - 1 - 146097 * y;
	uint32_t d = j / 4;

	j = (4 * d + 3) / 1461;
	d = 4 * d + 3 - 1461 * j;
	d = (d + 4) / 4;

	uint32_t m = (5 * d - 3) / 153;
	d = 5 * d - 3 - 153 * m;
	d = (d + 5) / 5;

	y = 100 * y + j;

	if (m < 10) {
		m = m + 3;
	} else {
		m = m - 9;
		y = y + 1;
	}
	yyyymmdd_array[0] = y;
	yyyymmdd_array[1] = m;
	yyyymmdd_array[2] = d;
	//printf("\n jd_to_calendar => y=%d %d %d ",(unsigned int)y,(unsigned int)m,(unsigned int)d);
	return;
}

/* based on http://en.wikipedia.org/wiki/Sunrise_equation */
uint32_t sunrise_equation(sunrise_equation_params_t *sun_eq_params, sunrise_sunset_time_t_mcu *sun_e)
{
	double int_part, fract_part;
	//uint64_t xx;

    printf("\n Enter: sunrise_equation()"); 
	printf("\nlattitude=%f, \nlongitude=%f \nsa=%f \ntz_sign=%d \n tz_hh=%d tz_mm= %d ",
			sun_eq_params->lattitude,
			sun_eq_params->longitude,
            sun_eq_params->solar_altitude_angle,
            sun_eq_params->tz_sign,
            sun_eq_params->tz_hh,
            sun_eq_params->tz_mm
        );

	printf("\nDate: %u:%u:%u ",
			sun_eq_params->year,
			sun_eq_params->month,
			sun_eq_params->day);
	double jdn = calendar_to_jdn(
			sun_eq_params->year,
			sun_eq_params->month,
			sun_eq_params->day);
	printf("\njdn= %f", jdn );

	double julian_date = jdn - 2451545;
	double julian_cycle_nStar = julian_date - 0.0009 - (sun_eq_params->lattitude / 360);
	printf("\njulian_cycle_nStar= %f",julian_cycle_nStar);

	double julian_cycle_n = floor(julian_cycle_nStar + 1.0 / 2.0);
	printf("\njulian_cycle_n= %f",julian_cycle_n);

	double solar_noon_JStar = 0.0009 + sun_eq_params->longitude / 360.0 + julian_cycle_n;	// note A1: this J* is without 2451545
	printf("\n solar_noon_JStar = %f \n",solar_noon_JStar);
	double solar_mean_anamoly = fmod((357.5291 + 0.98560028 * solar_noon_JStar), 360);
	/*note A2: due to note A1, J* is without 2451545, so  I _SHOULD NOT_ substract 2451545 again.
	see wikipedia formula, where they are substracting 2451545.
	*/
	printf("\n solar_mean_anamoly = %f\n", solar_mean_anamoly);
	//  C1 * Math.sin(M) + C2 * Math.sin(2 * M) + C3 * Math.sin(3 * M);
	double center_C = 1.9148 * sin(d2rad(solar_mean_anamoly)) 
		+ 0.0200 * sin(2.0 * d2rad(solar_mean_anamoly)) 
		+ 0.0003 * sin(3.0 * d2rad(solar_mean_anamoly));

	printf("\n center_C = %f \n",center_C);

	double ecliptic_longitude_lambda = fmod(solar_mean_anamoly + 102.9372 + center_C + 180, 360);
	printf("\n ecliptic_longitude_lambda = %f \n",ecliptic_longitude_lambda);

	double sun_declinataion_delta = r2deg(asin(sin( d2rad(ecliptic_longitude_lambda)) * sin( d2rad(23.45))));
	printf("\n sun_declinataion_delta = %f \n",sun_declinataion_delta);

	double cosine_hour_angle_wNot_degrees = 
		(sin( d2rad(sun_eq_params->solar_altitude_angle)) 
		- sin(d2rad( sun_eq_params->lattitude)) * sin(d2rad(sun_declinataion_delta)))
		/ (cos(d2rad(sun_eq_params->lattitude)) * cos( d2rad(sun_declinataion_delta)));
	printf("\n cosine_hour_angle_wNot_degrees =%f ", cosine_hour_angle_wNot_degrees);

	double hour_angle_wNot_degees = r2deg(acos(cosine_hour_angle_wNot_degrees));
	printf("\n hour_angle_wNot_degees = %f \n",hour_angle_wNot_degees );

	double jSet = 0.0009 + (hour_angle_wNot_degees / 360) 
		+ (sun_eq_params->longitude) / 360 + 0.0053 * sin(d2rad(solar_mean_anamoly)) - 0.0069 * sin(2 * d2rad(ecliptic_longitude_lambda));	/* "2451545 + julian_cycle_n" not added to make room  for fraction precision. */
	printf("\n jSet = %f Vs 2456176.041313 ...\n", jSet);

	double sunset_gmt = jSet + 0.5;	// gmt is 0.5 days ahead of Julian format
	printf("\n sunset_gmt=%f \n",sunset_gmt);

	modf(2451545 + julian_cycle_n + trunc(sunset_gmt), &int_part);
	// no rounding needed? re-think? sun may never rise on that day in some parts of world!

	// printf("\n modf(arg) = %s ", float_str);
	// printf("\n int_part =%s ", float_str);
	jd_to_calendar(lround(int_part), sun_e->ymd_set);	// GMT date of sun rise
	printf("\n int_part=%ld",lround(int_part) );
	printf("Sunset@ %u %u %u ", (unsigned int)sun_e->ymd_set[0],(unsigned int)sun_e->ymd_set[1],(unsigned int)sun_e->ymd_set[2]);

	double fractional_day = modf(sunset_gmt, &int_part);
	// printf("\n fractional_day =%s ", float_str);
	double GMT = fractional_day * 24.0;
	// printf("\n GMT =%s ", float_str); //printf("GMT = %f \n ", GMT);
	//float local_timeZone_set = GMT + 5.5;	// for india, IST is ahead of GMT by 5.5 hours or  5HH:30MM
	printf("\n TZ -----------11 ");
	printf("\n something=%f", sun_eq_params->tz_sign * (sun_eq_params->tz_hh + (float)sun_eq_params->tz_mm/60));
	
	double local_timeZone_set = GMT + sun_eq_params->tz_sign * (sun_eq_params->tz_hh + (float)sun_eq_params->tz_mm/60) ;	// for india, IST is ahead of GMT by 5.5 hours or  5HH:30MM
	// printf("\n local_timeZone_set =%s ", float_str);//printf("IST (format= solar day)  = %f \n ", local_timeZone_set );
	fract_part = modf(local_timeZone_set, &int_part);
	// printf("\n local_timeZone_set =%s ", float_str);
	sun_e->sset_hh = lround(trunc(int_part));
	sun_e->sset_mm = lround(trunc(fract_part * 60));
	sun_e->sset_ss = lround(trunc((modf((fract_part * 60), &int_part) * 60)));
	printf("\n ( hh:mm:ss) %u:%u:%u \n", 
			sun_e->sset_hh, 
			sun_e->sset_mm, 
			sun_e->sset_ss);

	double solar_noon_JStar_1 = 0.0009 + sun_eq_params->longitude / 360.0;	/// julian_cycle_n not added note A1: this J* is without 2451545
	printf("\nsolar_noon_JStar_1 = %f \n",solar_noon_JStar_1);

	double solar_tansit_jTransit = solar_noon_JStar_1 + 0.0053 * sin(d2rad(solar_mean_anamoly)) - 0.0069 * sin(2 * d2rad(ecliptic_longitude_lambda));
	printf("\nsolar_tansit_jTransit = %f", solar_tansit_jTransit);

	double jRise = solar_tansit_jTransit - (jSet - solar_tansit_jTransit);
	printf("\njRise = %f  VS 2456175.527898 \n", jRise); /* -ve jRise does indicate that, time just behind start of the julian day.
	This time is relative to present julian date and not the previous julian date.
	Draw number line and visualize this! Don't jump to previous JD point, but just go in left
	direction from present julian day point !*/

	double rise_jd_gmt = jRise + 0.5;	// gmt is 0.5days ahead of Julian day format
	printf("\n rise_jd_gmt =%f ", rise_jd_gmt);
	int32_t lround_val = lround(trunc(rise_jd_gmt));
	//printf("\n lround_val =%ld ", (long int)lround_val);

	jd_to_calendar(lround(trunc(2451545 + julian_cycle_n + lround_val)), sun_e->ymd_rise);	// calender date is same as JDN
	//printf("Sunrise@ %ld:%ld:%ld \n", sun_e->ymd_rise[0],sun_e->ymd_rise[1],sun_e->ymd_rise[2]);

	//float fractional_day_gmt = rise_jd_gmt - intpart;
	double iptr;
	double fractional_day_gmt = modf(rise_jd_gmt, &iptr);
	// printf("\n fractional_day_gmt =%s ", float_str);//printf("\n fractional_day_gmt =%f",fractional_day_gmt);

	GMT = fractional_day_gmt * 24.0;
	// printf("\n GMT =%s ", float_str);//printf(" GMT =%f\n ", GMT );

	//float local_TimeZone_Rise = GMT + 5.5;	// for india, IST is ahead of GMT by 5.5 hours or  5HH:30MM
	/* for other countries, local time zone will be at offset of GMT by x.y hours or  x HH: y*60 MM ,
	 * for example for India ,
	 * IST is ahaed of GMT by 5.5 hours (or 5 hours 30 mins).
	 * and this 5.5 is called timeZone of india... hope u got it
	 * */
	double local_TimeZone_Rise = GMT + sun_eq_params->tz_sign * (sun_eq_params->tz_hh + (float)sun_eq_params->tz_mm/60);

	// printf("\n local_TimeZone_Rise =%s ", float_str);//printf("IST (format=julian)  = %f \n ", IST );

	//srise_hh, swrise_mm, srise_ss; // these will never over flow. max val= 60 < 255
	/* truncate becos fractions are invalide now. in above line, fractions r used */
	fract_part = modf(local_TimeZone_Rise, &int_part);
	// printf("\n fract_part =%s ", float_str);
	sun_e->srise_hh = lround(trunc(int_part));
	sun_e->srise_mm = lround(trunc(fract_part * 60.0));
	sun_e->srise_ss = lround(trunc((modf((fract_part * 60.0), &int_part) * 60.0)));

#if 0
	//printf(" %f |",jRise);
	//printf(" %f |",jSet);
	printf(" printing info ...\n");
	printf(" %u:%u:%u |", (unsigned int)sun_e->ymd_rise[0], (unsigned int)sun_e->ymd_rise[1], (unsigned int)sun_e->ymd_rise[2]);
	printf(" %u:%u:%u |", (unsigned int)sun_e->srise_hh, (unsigned int)sun_e->srise_mm, (unsigned int)sun_e->srise_ss);
	//mdelay(500);
	//exit(0);

	printf(" %u:%u:%u |", (unsigned int)sun_e->ymd_set[0], (unsigned int)sun_e->ymd_set[1], (unsigned int)sun_e->ymd_set[2]);
	printf(" %u:%u:%u |", (unsigned int)sun_e->sset_hh, (unsigned int)sun_e->sset_mm, (unsigned int)sun_e->sset_ss);

	printf("%13s|", float_str);
	printf("%13s|", float_str);
	printf("%13s|", float_str);
#endif
	return 0;
}


/*This tells if the lights to be turn  ON or OFF.
 * return 0 if there is no error
 *       1 if there is some error
 *args: time - input time
 *    : on_off - this is where result will be put. this tells if the lights to be turned on or off.
 * */

uint8_t process_astro(const time_t_mcu *present_time, prov_t *prov1, unsigned int *on_off)
{
    printf("\nenter: process_astro");
    sunrise_sunset_time_t_mcu sun_e_set_only;
    sunrise_sunset_time_t_mcu sun_e_rise_set;
    sunrise_equation_params_t sun_eq_parms = { 0, 0, 0, 0, 0, 0 };
#if 0
	if (memcmp(present_time->year, &end_date.year, 6 * 4) > 0) {
		printf(" Fatal Error in RTC chip @ln %d...\n",__LINE__);
		while (1);
	}
#endif
    if (sun_eq_parms.day != present_time->month_day) {
 	printf("\nDay is NOT same. Do calc");
        printf("\n[Dt: %u:%02u:%02u",
        		present_time->year,
				present_time->month,
				present_time->month_day);

        sun_eq_parms.year = present_time->year;	/* 2000 already added */
        sun_eq_parms.month = present_time->month;
        sun_eq_parms.day = present_time->month_day;
        sun_eq_parms.lattitude = prov1->astro_prov.lattitude;
        sun_eq_parms.longitude = prov1->astro_prov.longitude;

        sun_eq_parms.lattitude = prov1->astro_prov.lattitude = 15.183333;
        sun_eq_parms.longitude = prov1->astro_prov.longitude = 75.733333;

        sun_eq_parms.tz_sign = prov1->astro_prov.tz_sign;
        sun_eq_parms.tz_hh = prov1->astro_prov.tz_hh;
        sun_eq_parms.tz_mm = prov1->astro_prov.tz_mm;

	/* printf("\n input: %ld:%ld:%ld ", sun_eq_parms.year, sun_eq_parms.month, sun_eq_parms.day); */

// Find rise time
    sun_eq_parms.solar_altitude_angle =  prov1->astro_prov.solar_altitude_angle_dawn_generic_use;// this is actually user configured
	sunrise_equation(&sun_eq_parms, &sun_e_rise_set);//morning OFF time

#if 0
// Find set time
    sun_eq_parms.solar_altitude_angle = prov1->astro_prov.solar_altitude_angle_dusk_generic_use;// this is actually user configured
    sunrise_equation(&sun_eq_parms, &sun_e_set_only);//morning OFF time

	//copy the sunset time
    memcpy(sun_e_rise_set.ymd_set,
    		sun_e_set_only.ymd_set,
    		sizeof(sun_e_set_only.ymd_set) + sizeof(sun_e_set_only.sset_hh) + sizeof(sun_e_set_only.sset_mm) + sizeof(sun_e_set_only.sset_ss)
			);
#endif

        
	snprintf(&off_time[0],20,"%02u:%02u:%02u %s",
			(unsigned int)(((sun_e_rise_set.srise_hh + 11)%12)+1),
			(unsigned int)sun_e_rise_set.srise_mm,
			(unsigned int)sun_e_rise_set.srise_ss,
			sun_e_rise_set.srise_hh>=12?"PM":"AM");
        
	snprintf(&on_time[0],20,"%02u:%02u:%02u %s",
			(unsigned int)(((sun_e_rise_set.sset_hh + 11)%12)+1),
			(unsigned int)sun_e_rise_set.sset_mm,
			(unsigned int)sun_e_rise_set.sset_ss,
			sun_e_rise_set.sset_hh>=12?"PM":"AM");
        /* printf("all: %s %s %s %s  end\n",on_date,on_time,off_date, off_time); */
        printf("\n rise date  | rise time| set date   |  set time|\n");
        printf(" %u:%02u:%02u |",
        		(unsigned int)sun_e_rise_set.ymd_rise[0],
				(unsigned int)sun_e_rise_set.ymd_rise[1],
				(unsigned int)sun_e_rise_set.ymd_rise[2]);
        printf(" %02u:%02u:%02u |",
        		(unsigned int)sun_e_rise_set.srise_hh,
				(unsigned int)sun_e_rise_set.srise_mm,
				(unsigned int)sun_e_rise_set.srise_ss);
        printf(" %u:%02u:%02u |", (unsigned int)sun_e_rise_set.ymd_set[0],
        		(unsigned int)sun_e_rise_set.ymd_set[1],
				(unsigned int)sun_e_rise_set.ymd_set[2]);
        printf(" %02u:%02u:%02u |\n",
        		(unsigned int)sun_e_rise_set.sset_hh,
        		(unsigned int)sun_e_rise_set.sset_mm,
				(unsigned int)sun_e_rise_set.sset_ss);
    } else {
            /* printf("2Day is same. Skip computation\n");
             * UART_SendWait(UART1, "1Day is same. Skip computation\n", 10);*/
	}
	/*
	 *                   .....On......Rise......Off...........Set....On......
	 *         00:00:00  --------------|-----------------------|------------- 23:59:59
	 * */
    if ( (memcmp((const void *)(&present_time->year), &sun_e_rise_set.ymd_rise, 6 * 4) > 0) &&
         (memcmp((const void *)&present_time->year, &sun_e_rise_set.ymd_set, 6 * 4) < 0)) {
        //printf("ln=%d\n",__LINE__);
	if(0 != *on_off){
    	    *on_off = 0;
    	    printf("\nTurn OFF,in b/w rise-set");
    	    printf(" [Dt/Tm: %u:%02u:%02u : %02u:%02u:%02u\n",
    	    		(unsigned int)present_time->year,
					(unsigned int)present_time->month,
					(unsigned int)present_time->month_day,
					(unsigned int)present_time->hour,
					(unsigned int)present_time->minutes,
					(unsigned int)present_time->seconds);
	}
    } else if (1 != *on_off){
        printf("\nTurn On:bfore rise or after set");
        printf(" [Dt/Tm: %u:%02u:%02u : %02u:%02u:%02u\n",
        		(unsigned int)present_time->year,
				(unsigned int)present_time->month,
				(unsigned int)present_time->month_day,
				(unsigned int)present_time->hour,
				(unsigned int)present_time->minutes,
				(unsigned int)present_time->seconds);
        *on_off = 1;
    }
     return 0;
}

void test_process_astro(void)
{
    time_t_mcu present_time;
    unsigned int on_off;
    prov_t prov;

#if 1
    //Kurtakoti.
    //prov.astro_prov.lattitude= 15.4289;
    //prov.astro_prov.longitude= -75.6315;

    //Gadag
    prov.astro_prov.lattitude= 15.42;
    prov.astro_prov.longitude= -75.62;
#elif 1
    /* Domlur     *
     * 09.Dec.2020
     * as per suncalc  As per our data
     * Dawn:06.07.18  06.07.09
     * rise 06.30.01  06.30.02
     *
     * set 17.53.40   17.53.45
     * Dusk 18.16.24  18.16.29
     *
     * as per our calculation
     *
     * */

    //prov.astro_prov.lattitude= 12.9607;
    p//rov.astro_prov.longitude=-77.6411;
#endif

    prov.astro_prov.tz_sign = +1;
    prov.astro_prov.tz_hh= 5;
    prov.astro_prov.tz_mm= 30;

    present_time.year=2021;
    present_time.month=01;
    present_time.month_day=15;

    prov.astro_prov.solar_altitude_angle_dawn_generic_use= -1.225; //= -0.82 - 0.405
    prov.astro_prov.solar_altitude_angle_dusk_generic_use= -0.616; //= -0.82 + 0.204

    prov.astro_prov.solar_altitude_angle_dawn_generic_use= -0.83; //= -0.82 - 0.405
    prov.astro_prov.solar_altitude_angle_dusk_generic_use= -0.83; //= -0.82 + 0.204
    process_astro( &present_time, &prov, &on_off);
    while(1);

    //for Civil Twilight
    prov.astro_prov.solar_altitude_angle_dawn_generic_use= -6 - .375; // -6 -0.82 + .445
    prov.astro_prov.solar_altitude_angle_dusk_generic_use= -6 + .28;  // -6 -0.82 + 1.1

    prov.astro_prov.solar_altitude_angle_dawn_generic_use= 0; //= -0.82 - 0.405
    prov.astro_prov.solar_altitude_angle_dusk_generic_use= 0; //= -0.82 + 0.204
    process_astro( &present_time, &prov, &on_off);

    while(1);
}
#if 0
#ifdef DEBUG_ASTRO
int main1(void)
{
	struct sunrise_equation_params sun_eq_parms2 = { 0, 0, 0, 0, 0, 0 };
	struct sunrise_sunset_time_t_mcu sun_e_civil;
	printf("\n   %s:%d \n", __FILE__, __LINE__);
    	//printf("\nV0.0 AUMTatSat R2.3.0.3 20180704");
#if 0
	float jdn = calendar_to_jdn(2014, 12, 31);
	char float_str[10];
	printf(" | %s |", float_str);

	uint32_t ymd[10];
	jdn = jdn + 1.0;
	printf(" | %s |", float_str);

	jd_to_calendar((uint32_t) jdn, ymd);
	printf("%s %d %u %u %u ", __FILE__, __LINE__, (unsigned int)ymd[0], (unsigned int)ymd[1], (unsigned int)ymd[2]);
	//return 0;
#endif
	printf("\n rise date  |Tcivildawn| sun rise | set date   |  sun set |Tcivildusk| Center C    |\n");
	//while(1);
	uint32_t yyyy, mm, dd;
	for (yyyy = 2016; yyyy <= 2016; yyyy++) {
		for (mm = 1; mm <= 12; mm++) {
			for (dd = 1; dd <= 10; dd++) {
				WDOG_Feed();
				sun_eq_parms2.year = yyyy;
				sun_eq_parms2.month = mm;
				sun_eq_parms2.day = dd;
				//sun_eq_parms2.lattitude = prov.lattitude;
				//sun_eq_parms2.longitude = prov.longitude;
				sun_eq_parms2.lattitude = LAT_FLOAT;
				sun_eq_parms2.longitude = LONG_FLOAT;
				//sun_eq_parms2.solar_altitude_angle = prov.solar_altitude_angle; // this is actually user configured
				sun_eq_parms2.solar_altitude_angle = -0.83;	// the geo-metric center of sun exactly crossing Horizon
				sunrise_equation(&sun_eq_parms2, &sun_e_rise_set);
				//sun_eq_ parms2.solar_altitude_angle = get_dawn_gc_of_sun(sun_eq_parms2.month);	// for civil twilight calculation; sun rays bend due to refraction of light via air, hence gives light even 'after sunset' or 'before sunrise' on horizon
				sun_eq_parms2.solar_altitude_angle = -5;
				printf("  gc= ");
				printf("\nsun_eq_parms2.solar_altitude_angle=%f ", sun_eq_parms2.solar_altitude_angle);
				sunrise_equation(&sun_eq_parms2, &sun_e_civil);
#if 1
				//printf(" %f |",jRise);
				//printf(" %f |",jSet);
				printf(" %02u:%02u:%02u  ", (unsigned int)sun_e_rise_set.ymd_rise[0], (unsigned int)sun_e_rise_set.ymd_rise[1], (unsigned int)sun_e_rise_set.ymd_rise[2]);
				printf(" %02u:%02u:%02u |", (unsigned int)sun_e_civil.srise_hh, (unsigned int)sun_e_civil.srise_mm, (unsigned int)sun_e_civil.srise_ss);
				printf(" %02u:%0u:%02u |", (unsigned int)sun_e_rise_set.srise_hh, (unsigned int)sun_e_rise_set.srise_mm, (unsigned int)sun_e_rise_set.srise_ss);
				//mdelay(500);
				//exit(0);

				printf(" %02u:%02u:%02u |", (unsigned int)sun_e_rise_set.ymd_set[0], (unsigned int)sun_e_rise_set.ymd_set[1], (unsigned int)sun_e_rise_set.ymd_set[2]);
				printf(" %02u:%02u:%02u |", (unsigned int)sun_e_rise_set.sset_hh, (unsigned int)sun_e_rise_set.sset_mm, (unsigned int)sun_e_rise_set.sset_ss);
				printf(" %02u:%02u:%02u |", (unsigned int)sun_e_civil.sset_hh, (unsigned int)sun_e_civil.sset_mm, (unsigned int)sun_e_civil.sset_ss);
				printf("\n\n");
				//exit(0);
#if 0
				printf("%13s|", float_str);
				printf("%13s|", float_str);
				printf("%13s|", float_str);
#endif
#endif
				//exit(0);
				//printf("\n");
			}
		}
	}
	// reset this shared variable
	sun_eq_parms2.year = sun_eq_parms2.month = sun_eq_parms2.day = 0;
	return 0;
}
#endif


#if 0
/**********************************************************************
Function Name : RTC_ISR
Notes : Interrupt service routine for RTC module.
**********************************************************************/

volatile uint32_t sw_rtc_tick;
volatile uint32_t sw_rtc_tick;

void rtc_tick(void)
{
	//printf("\n sw_rtc_tick=%d", sw_rtc_tick);
    	sw_rtc_tick++;
	lcd_backlit_tick++;
	if(lcd_non_seq_wait_seconds){
		lcd_non_seq_wait_seconds--;
	}
    static const signed char dim[12][2] =
    { // Number of days in month for non-leap year and leap year
	{ 31, 31}, // January
        { 28, 29}, // February
        { 31, 31}, // March
        { 30, 30}, // April
        { 31, 31}, // May
        { 30, 30}, // June
        { 31, 31}, // July
        { 31, 31}, // August
        { 30, 30}, // September
        { 31, 31}, // October
        { 30, 30}, // November
        { 31, 31}  // December
    };

/* Clears the interrupt flag, RTIF, and interrupt request */
	RTC->SC |= RTC_SC_RTIF_MASK;
    if(++tm_sw_cal.sec > 59) {                              // Increment seconds, check for overflow
        tm_sw_cal.sec = 0;                                  // Reset seconds
        if(++tm_sw_cal.min > 59) {                          // Increment minutes, check for overflow
            tm_sw_cal.min = 0;                              // Reset minutes
            if(++tm_sw_cal.hour > 23) {                     // Increment hours, check for overflow
                tm_sw_cal.hour = 0;                         // Reset hours
#if 0
                ++tm_sw_cal.ul_yday;                           // Increment day of year
                if(++tm_sw_cal.ul_wday > 6)                    // Increment day of week, check for overflow
                    tm_sw_cal.ul_wday = 0;                     // Reset day of week
#endif                                                        // Increment day of month, check for overflow
                if(++tm_sw_cal.month_day > dim[tm_sw_cal.month][is_leap_year(tm_sw_cal.year + 1900)]) {
                    tm_sw_cal.month_day = 1;                     // Reset day of month
                    if(++tm_sw_cal.month > 11) {              // Increment month, check for overflow
                        tm_sw_cal.month = 0;                  // Reset month
                        //tm_sw_cal.ul_yday = 0;                 // Reset day of year
                        ++tm_sw_cal.year;                   // Increment year
                    }                                   // - year
                }                                       // - month
            }                                           // - day
        }                                               // - hour
    }                                                   // - minute
}

#endif
#if 0
static int is_leap_year(const int y)
{
    if(y & 3) return 0;         // Not divisible by 4
    switch(y % 400) {           // Divisible by 100, but not by 400 (1900, 2100, 2200, 2300, 2500, 2600)
        case 100: case 200: case 300: return 0;
    }
    return 1;                   // Divisible by 4 and !(100 and !400)
}
#endif

#if 0
int get_dusk_gc_of_sun(int month)
{
	//TODO: hardcoded now get the values from the eeprom
	switch(month){
		case 1:case 2:case 3:case 4:
			return -6;
			break;
		case 5:case 6:case 7:case 8:
			return -2; // rainy season, clouds blocks sunrays in eveneing
			break;
		case 9:case 10:case 11:case 12:
			return -6;
			break;
	}
	return 0;
}

int get_dawn_gc_of_sun(int month)
{
	//TODO: hardcoded now get the values from the eeprom
	switch(month){
		case 1:case 2:case 3:case 4:
			return -3;
			break;
		case 5:case 6:case 7:case 8:
			return -3;// at -6deg dawn is 5:50, at -5deg dawn is 5:55
			break;
		case 9:case 10:case 11:case 12:
			return -3;
			break;
	}
	return 0;
}

int get_default_gc_of_sun(void)
{
	return -3;
}



/*
 * Returns 0 if all is OK
 * Returns Non zero if something is bad
 * Returns 1 is DATE OR TIME IS wrong- typically due to i2c err
 * */
uint8_t astro(void)
{
    printf("enter: %s\n",__FUNCTION__);
    static struct sunrise_equation_params sun_eq_parms = { 0, 0, 0, 0, 0, 0 };
    uint8_t ret_val = get_rtc_date_time();
    /*print_tm(&tm); */
    if ( ((0 != ret_val ) ) ){
		printf(" invalid date @ ln=%d\n", __LINE__);
	    	snprintf((char *)&lcd_rows[0],17,"WRONG DATE TIME");
		snprintf((char *)&lcd_rows[1], 16, "Code %d",ret_val);
	    	lcd_update_two_rows();

		printf("ERR01: Date/Time is wrong!\n");
		set_relay(0); // turn off relay
		delay_ms(5000);

		print_tm_to_str((struct tm2 *)&tm,(char *)&lcd_rows[0],(char *)&lcd_rows[1]);
		printf("\n %s %s ", (char *)&lcd_rows[0], (char *)&lcd_rows[1]);
		lcd_update_two_rows();
		delay_ms(2000);
		return 1; // wrong date. return.
	}
#if 0
	if (memcmp(&tm.year, &end_date.year, 6 * 4) > 0) {
		printf(" Fatal Error in RTC chip @ln %d...\n",__LINE__);
		while (1) ;
	}
#endif
	//printf(" [Time/Date: %u:%02u:%02u : %02u:%02u:%02u\n", (unsigned int)tm.year, (unsigned int)tm.month, (unsigned int)tm.month_day, (unsigned int)tm.hour, (unsigned int)tm.min, (unsigned int)tm.sec);
	if (sun_eq_parms.day != tm.month_day) {
		printf("Day is NOT same. Do calc\n");
		printf(" [Dt/Tm: %u:%02u:%02u : %02u:%02u:%02u\n", (unsigned int)tm.year, (unsigned int)tm.month, (unsigned int)tm.month_day, (unsigned int)tm.hour, (unsigned int)tm.min, (unsigned int)tm.sec);
		sun_eq_parms.year = tm.year;	/* 2000 already added */
		sun_eq_parms.month = tm.month;
		sun_eq_parms.day = tm.month_day;
		sun_eq_parms.lattitude = prov.lattitude;
		sun_eq_parms.longitude = prov.longitude;
		sun_eq_parms.solar_altitude_angle = prov.solar_altitude_angle;// this is actually user configured

		sun_eq_parms.tz_sign = prov.tz_sign;
		sun_eq_parms.tz_hh = prov.tz_hh;
		sun_eq_parms.tz_mm = prov.tz_mm;

		//sun_eq_parms.solar_altitude_angle = get_default_gc_of_sun();	// this is actually user configured
		//printf("\n input: %ld:%ld:%ld ", sun_eq_parms.year, sun_eq_parms.month, sun_eq_parms.day);
		sunrise_equation(&sun_eq_parms, &sun_e_rise_set);//morning OFF time
		//sun_eq_parms.solar_altitude_angle = prov.solar_altitude_angle; //evening time. Normally it is early compared to morning time. may be clouds are more spread during day due to sun rays. so evening sunlight fades too quickly. since sun angle is negative, we need to add to bring sun nearer to our visible rays.
#if 0
		sunrise_equation(&sun_eq_parms, &sun_evt_set);
		sun_e_rise_set.sset_hh = sun_evt_set.sset_hh;
		sun_e_rise_set.sset_mm = sun_evt_set.sset_mm;
		sun_e_rise_set.sset_ss = sun_evt_set.sset_ss;
#endif

		memset(&on_date[0],' ',17);
		memset(&on_time[0],' ',17);
		memset(&off_date[0],' ',17);
		memset(&off_time[0],' ',17);

		//snprintf(&on_date_time[0],20,"%u-%02u-%02u %02u:%02u:%02u", (unsigned int)sun_e_rise_set.ymd_rise[0], (unsigned int)sun_e_rise_set.ymd_rise[1], (unsigned int)sun_e_rise_set.ymd_rise[2], (unsigned int)sun_e_rise_set.srise_hh, (unsigned int)sun_e_rise_set.srise_mm,(unsigned int)sun_e_rise_set.srise_ss);
		snprintf(&off_time[0],20,"%02u:%02u:%02u %s", (unsigned int)(((sun_e_rise_set.srise_hh + 11)%12)+1), (unsigned int)sun_e_rise_set.srise_mm,(unsigned int)sun_e_rise_set.srise_ss, sun_e_rise_set.srise_hh>=12?"PM":"AM");
		//snprintf(&off_date_time[0],20,"%u-%02u-%02u %02u:%02u:%02u", (unsigned int)sun_e_rise_set.ymd_set[0], (unsigned int)sun_e_rise_set.ymd_set[1], (unsigned int)sun_e_rise_set.ymd_set[2], (unsigned int)sun_e_rise_set.sset_hh, (unsigned int)sun_e_rise_set.sset_mm,(unsigned int)sun_e_rise_set.sset_ss);
		snprintf(&on_time[0],20,"%02u:%02u:%02u %s", (unsigned int)(((sun_e_rise_set.sset_hh + 11)%12)+1), (unsigned int)sun_e_rise_set.sset_mm,(unsigned int)sun_e_rise_set.sset_ss, sun_e_rise_set.sset_hh>=12?"PM":"AM");
		//printf("all: %s %s %s %s  end\n",on_date,on_time,off_date, off_time);
	} else {
		//printf("2Day is same. Skip computation\n");
		//UART_SendWait(UART1, "1Day is same. Skip computation\n", 10);
	}
	//printf(" hi \n");
	/*
	 *              .....ON......Rise......OFF...........set....ON......
	 *         0:0  ______________|_______________________|_____________ 23:59
	 * */
	//printf("ln=%d\n",__LINE__);
#if 0
	if (memcmp((const void *)&tm.year, &manuf_date.year, 6 * 4) < 0) {
		printf(" Fatal: curr<manuf @%d...\n",__LINE__);
		lcd_non_seq_wait_seconds = 3;
		Lcd_Out_two_rows("ERR01: DATE/TIME","IS WRONG!");
		printf("ERR01: Date/Time is wrong!\n");
		set_relay(0);
		delay_ms(3000);
		//print_tm(&tm);
		//printf("sw cal:\n");
		//print_tm(&tm_sw_cal);
		return;
	}
#endif
	if ( (memcmp((const void *)(&tm.year), &sun_e_rise_set.ymd_rise, 6 * 4) > 0) &&
	     (memcmp((const void *)&tm.year, &sun_e_rise_set.ymd_set, 6 * 4) < 0)) { // try to turn off
		printf("ln=%d\n",__LINE__);
		if (get_relay() != 0) {
			//printf("ln=%d\n",__LINE__);
			printf("Turn OFF,in b/w rise-set\n");
			if(0 == force_on) {
			    set_relay(0);
			} else if(1 == force_on) {
			    snprintf((char *)&lcd_rows[0],17,"FORCE ON");
			    printf("FORCE ON in EFFECT\n");
			} else {
			    printf("light off\n");
			    snprintf((char *)&lcd_rows[0],17,"LIGHTS OFF");
			}
			snprintf((char *)&lcd_rows[1], 16, " ");
			lcd_update_two_rows();
			delay_ms(1000);

			printf("rise date  | rise time| set date   |  set time|\n");
			printf("%u:%02u:%02u |", (unsigned int)sun_e_rise_set.ymd_rise[0], (unsigned int)sun_e_rise_set.ymd_rise[1], (unsigned int)sun_e_rise_set.ymd_rise[2]);
			printf(" %02u:%02u:%02u |", (unsigned int)sun_e_rise_set.srise_hh, (unsigned int)sun_e_rise_set.srise_mm, (unsigned int)sun_e_rise_set.srise_ss);
			printf(" %u:%02u:%02u |", (unsigned int)sun_e_rise_set.ymd_set[0], (unsigned int)sun_e_rise_set.ymd_set[1], (unsigned int)sun_e_rise_set.ymd_set[2]);
			printf(" %02u:%02u:%02u|", (unsigned int)sun_e_rise_set.sset_hh, (unsigned int)sun_e_rise_set.sset_mm, (unsigned int)sun_e_rise_set.sset_ss);
			printf("\n");
		}
	} else {//try to turn on ...
		//printf("ln=%d\n",__LINE__);
		if (get_relay() != 1) { // if lights are OFF
			//printf("ln=%d\n",__LINE__);
			if((0 == prov.voltage_monitor_enabled) || (NORMAL_VOLT == get_230VAC_line_voltage_status())) {
				printf(" Turn On:bfore rise or after set \n");
				set_relay(1);
				printf("init_lcd=%d\n",__LINE__);
				init_lcd_4bits_mode();
				snprintf((char *)&lcd_rows[0],17,"LIGHTS ON");
				lcd_update_one_row(0);

			printf("\n rise date  | rise time| set date   |  set time|\n");
			printf(" %u:%02u:%02u |", (unsigned int)sun_e_rise_set.ymd_rise[0], (unsigned int)sun_e_rise_set.ymd_rise[1], (unsigned int)sun_e_rise_set.ymd_rise[2]);
			printf(" %02u:%02u:%02u |", (unsigned int)sun_e_rise_set.srise_hh, (unsigned int)sun_e_rise_set.srise_mm, (unsigned int)sun_e_rise_set.srise_ss);
			printf(" %u:%02u:%02u |", (unsigned int)sun_e_rise_set.ymd_set[0], (unsigned int)sun_e_rise_set.ymd_set[1], (unsigned int)sun_e_rise_set.ymd_set[2]);
			printf(" %02u:%02u:%02u |", (unsigned int)sun_e_rise_set.sset_hh, (unsigned int)sun_e_rise_set.sset_mm, (unsigned int)sun_e_rise_set.sset_ss);
			printf("\n");
			} else {
				printf(" Lights will be off till voltage is normal!!!\n");
			}
		}
	}
#if 0
	http://en.wikipedia.org/wiki/Twilight
	6 for civil, 12 for nauticale twilight

	-0.... -6.......-10 .... -80(-10) .... -90 ..... -162.... -170(-10) ....-174(-6).......-0
	 set    civil    on        off        midnight    on      off           civil         rise

	 OR
	 ->simply turn on at gc=-10 for evening,
	 ->turn off gc=-10 for morning OR non peak hours 23hh 00mm to 4hh 30mm

	Add turn off feature for second relay. a subset of cluser lights in circles.
		if current time is > 23 hrs AND less than 4 AM {
			turn off // since it is non peak hours
		} else {

		}
#endif

    printf("exit: %s\n",__FUNCTION__);
	return 0;
}



#endif

#endif
#endif
