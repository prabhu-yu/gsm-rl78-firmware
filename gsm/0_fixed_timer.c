#include "0_all.h"
int sent_time_t_mcuable_sms[relay_time_t_mcuable_sz];

static void set_relay_oper_status_due_to_timer(unsigned int relay, relay_status_t relay_status) {
	char temp_str[50];
	printf("\nEnter: set_relay_oper_status_due_to_timer");
    printf("\nln=%d,set_relay_oper_status_due_to_timer(relay=%d, relay, relay_status=%d)",__LINE__,relay, relay_status);
	snprintf(temp_str, sizeof(temp_str)-1, "\rTimer: Switch%d -> %s",relay, relay_status?"ON":"OFF");
    //TODO lennght should not exceed 130 bytes ...
    //printf("\nA.===> sms= temp_str=%s\nsend_sms_body_160=%s<===", temp_str, send_sms_body_160);
    strncat(send_sms_body_160, temp_str, (sizeof(temp_str))-1);
    //printf("\nB.===>  sms= temp_str=%s\nsend_sms_body_160=%s<===", temp_str, send_sms_body_160);
	send_sms_flag = activate_relays_based_on_oper_stat_flag = true;
	set_relay_oper_status(relay, relay_status);
	printf("Return: set_relay_oper_status_due_to_timer");
    return;
}

void reset_time_t_mcuable_sent_sms(void) {
    memset(sent_time_t_mcuable_sms, 0, sizeof(sent_time_t_mcuable_sms));
}

void reset_and_update_time_t_mcuable_sent_sms(int time_t_mcuable_index) {
	printf("Enter : reset_and_update_time_t_mcuable_sent_sms(%d)", time_t_mcuable_index);
    memset(sent_time_t_mcuable_sms, 0, sizeof(sent_time_t_mcuable_sms));
    sent_time_t_mcuable_sms[time_t_mcuable_index] = 1;
    printf("Return : reset_and_update_time_t_mcuable_sent_sms");
    return;
}

/* returns
 * 0 if all is good.
 * 1 (a non zero) if time is not good
 * */

unsigned int validate_date_time(time_t_mcu *tm_obj) {
	if (	(tm_obj->year < 20) ||
			(tm_obj->year > 30) ||
			(tm_obj->month < 1) ||
			(tm_obj->month > 12) ||
			(tm_obj->month_day < 1) ||
			(tm_obj->month_day > 31) ||
			(tm_obj->hour < 1) ||
			(tm_obj->hour > 24) ||
			(tm_obj->minutes < 1) ||
			(tm_obj->minutes > 60) ||
			(tm_obj->seconds < 1) ||
			(tm_obj->seconds > 60)
	   ) {
		return 1;
	}
	return 0;
}

void timer_processing(void) {
    static unsigned int tt_current_index = 0;
    float time_now = oper.time_now.hour + (oper.time_now.minutes/60.0) + (oper.time_now.seconds/3600.0);
    float prov_t_i, prov_t_i_next;
    float prov_time_at_index_zero, prov_time_at_index_max;

    printf("\n >>> timer_processing ");
    if( validate_date_time(&oper.time_now) ) {
    	printf("\rn <<< Invalid Time; ");
    	return;
    }

    //printf("\rln=%d, %02d:%02d:%02d, float=%f", __LINE__,oper.time_now.hh, oper.time_now.mm, oper.time_now.ss, time_now);
    unsigned int max_rl_tt_provisioned_index = (prov.basic_prov.max_rl_tt_provisioned-1);
    if ( max_rl_tt_provisioned_index > (relay_time_t_mcuable_sz-1) ){
    	printf("\nSome thing wrong in max_rl_tt_provisioned %u", max_rl_tt_provisioned_index);
    	return;
    }

    while(1){
        printf("\nln=%d ____________________ tt_current_index=%d", __LINE__, tt_current_index);
        if ( tt_current_index >= max_rl_tt_provisioned_index) {
            printf("\nln=%d, error in TT?", __LINE__);
            tt_current_index = 0;
        }
        prov_time_at_index_zero = prov.rl_tt[0].hour + (prov.rl_tt[0].minutes/60.0) + (prov.rl_tt[0].seconds/3600.0);
        prov_time_at_index_max = prov.rl_tt[max_rl_tt_provisioned_index].hour + (prov.rl_tt[max_rl_tt_provisioned_index].minutes/60.0) + (prov.rl_tt[max_rl_tt_provisioned_index].seconds/3600.0);
        //very first case
        if( (time_now < prov_time_at_index_zero) || (time_now > prov_time_at_index_max) ) {
            printf("\nln=%d, spl case", __LINE__);
            if(0 == sent_time_t_mcuable_sms[max_rl_tt_provisioned_index]){
                printf("\nln=%d -> act at index max_rl_tt_provisioned_index = %d", __LINE__, max_rl_tt_provisioned_index );
                set_relay_oper_status_due_to_timer(0, (relay_status_t) prov.rl_tt[max_rl_tt_provisioned_index].relay_status);
                reset_and_update_time_t_mcuable_sent_sms(max_rl_tt_provisioned_index);
                tt_current_index = 0;
            } else {
                printf("\nln=%d, Already sent sms for max index spl case;", __LINE__);
            }
            break;
        } else {
            prov_t_i = prov.rl_tt[tt_current_index].hour + (prov.rl_tt[tt_current_index].minutes/60.0) + (prov.rl_tt[tt_current_index].seconds/3600.0);
            prov_t_i_next = prov.rl_tt[tt_current_index+1].hour + (prov.rl_tt[tt_current_index+1].minutes/60.0) + (prov.rl_tt[tt_current_index+1].seconds/3600.0);
            printf("\nln=%d prov_t_i(%d)=%f, prov_t_i_next=%f", __LINE__, tt_current_index, prov_t_i,  prov_t_i_next);
            if( (time_now > prov_t_i) && (time_now < prov_t_i_next) ){
                printf("\nln=%d", __LINE__);
                if(0 == sent_time_t_mcuable_sms[tt_current_index]) {
                    printf("\nln=%d prov_t_i(%d) = %f, prov_t_i_next=%f", __LINE__, tt_current_index, prov_t_i,  prov_t_i_next);
                    set_relay_oper_status_due_to_timer(0, (relay_status_t) prov.rl_tt[tt_current_index].relay_status);
                    reset_and_update_time_t_mcuable_sent_sms(tt_current_index);
                } else {
                    printf("\nln=%d, Already sent sms for index = %d", __LINE__, tt_current_index);
                }
                break;
            } else if (time_now >= prov_t_i_next){
                printf("\nln=%d, now is bigger than i=%d, i+1, so increament ", __LINE__, tt_current_index);
                tt_current_index++;
                if(tt_current_index >= (prov.basic_prov.max_rl_tt_provisioned-1)) {
                    printf("\nln=%d, DATA error TT?", __LINE__);
                    tt_current_index = 0;
                    break;
                }
            } else {
                printf("\nln=%d,  LOGIC error", __LINE__);
                break;
            }
        }
    }
     printf("\rreturn:timer_processing ");
}

int main1(void) {
    int hh,mm,ss,i;
    oper.time_now.hour=19;
    oper.time_now.minutes=31;
    oper.time_now.seconds=45;

    prov.rl_tt[0].hour=6;
	prov.rl_tt[0].minutes=30;
	prov.rl_tt[0].seconds=44;
	prov.rl_tt[0].relay_status=0;

    prov.rl_tt[1].hour=18;
	prov.rl_tt[1].minutes=30;
	prov.rl_tt[1].seconds=44;
	prov.rl_tt[1].relay_status=1;

    prov.rl_tt[2].hour=19;
	prov.rl_tt[2].minutes=30;
	prov.rl_tt[2].seconds=44;
	prov.rl_tt[2].relay_status=1;

    prov.rl_tt[3].hour=20;
	prov.rl_tt[3].minutes=30;
	prov.rl_tt[3].seconds=44;
	prov.rl_tt[3].relay_status=1;

    for (i=0;i<2;i++) {
        for(hh=6;hh<24;hh++){
            for(mm=0;mm<60;mm++){
                for(ss=0;ss<60;ss++){
                    oper.time_now.hour=hh;
                    oper.time_now.minutes=mm;
                    oper.time_now.seconds=ss;
                    timer_processing();
                }
            }
        }
    }
    printf("\n");
    return 0;
}
