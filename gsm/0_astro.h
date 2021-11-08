#ifndef _ASTRO_H
#define _ASTRO_H

#include "0_all.h"

#define M_PI   3.14159265358979323846
// Converts degrees to radians.
#define d2rad(angleDegrees) (angleDegrees * M_PI / 180.0)
// Converts radians to degrees.
#define r2deg(angleRadians) (angleRadians * 180.0 / M_PI)


typedef struct  {
	unsigned int year;
	unsigned int month;
	unsigned int day;
	float lattitude;
	float longitude;
	/* solar_altitude_angle= Solar elevation Angle=Solar Altitude=SA= geometric centre of sun below horizon.
	 * SA=-0.83 deg for sunrise/sunset,
	 * SA=-6.0 deg for civil twi-light aka Dawn, Dusk
	 * SA=-4.0 I choose this for after various trail and error for Rural/open environment
	 * SA=-3.0 I choose this for after various trail and error for City/congested/high rise building life
	 * */
	float solar_altitude_angle;

	signed int tz_sign;// sign of UTC offset
	unsigned int tz_hh;// UTC offset
	unsigned int tz_mm;//UTC offset
}sunrise_equation_params_t;

typedef struct {
	unsigned int ymd_rise[3];	// sun rise date ymd_rise[0]=yy ymd_rise[1]=mm ymd_rise[2]=dd on that location note: year 2013 should be written as 13
	unsigned int srise_hh, srise_mm, srise_ss;	// //sun set time on that location
	unsigned int ymd_set[3];	// sun set date yy, mm , dd on that location
	unsigned int sset_hh, sset_mm, sset_ss;	//sun set time on that location
    time_t_mcu rise_or_early_time; //TODO: replace above by these 
    time_t_mcu set_or_late_time;
}sunrise_sunset_time_t_mcu;

uint8_t process_astro(const time_t_mcu *present_time, prov_t *prov1, unsigned int *on_off);
void test_process_astro(void);

#endif

