#include "0_all.h"
void setup_pins(void) {
	// enable pull-up resistors on the pins
}

void print_pin_input_status(char *prefix, unsigned char no_x) {
	if(no_x) {
		printf("[S%s:1]  ", prefix);
	} else {
		printf("[S%s:0]  ", prefix);
	}
}

unsigned int sensor_level = 3;
#if MACHINE == UBUNTU
unsigned int get_water_sensor_data(int level) {
    if (level == sensor_level) {
        return 0; // touched water
    }
    return 1; // no water
}
#endif

// TODO Soaking.
// if no sensor indicates water, it is assumed to be zero level.- No water.
int read_sensor_val_from_hw(void) {
	printf("Enter : read_sensor_val_from_hw()\n ");
	print_pin_input_status("1", WS_LEVEL_1);
	print_pin_input_status("2", WS_LEVEL_2);
	print_pin_input_status("3", WS_LEVEL_3);
	print_pin_input_status("4", WS_LEVEL_4);
	print_pin_input_status("5", WS_LEVEL_5);
	print_pin_input_status("6", WS_LEVEL_6);
	print_pin_input_status("7", WS_LEVEL_7);
	print_pin_input_status("8", WS_LEVEL_8);
	print_pin_input_status("9", WS_LEVEL_9);

    if ( 0 == WS_LEVEL_9) {
		printf(" L9,");
        return 9;
	}
	if ( 0 == WS_LEVEL_8) {
		printf(" L8,");
		return 8;
	}
    if ( 0 == WS_LEVEL_7) {
		printf(" L7,");
		return 7;
	}
	if ( 0 == WS_LEVEL_6) {
		printf(" L6,");
		return 6;
	}
	if ( 0 == WS_LEVEL_5) {
		printf(" L5,");
		return 5;
	}
	if ( 0 == WS_LEVEL_4) {
		printf(" L4,");
		return 4;
	}
	if ( 0 == WS_LEVEL_3) {
		printf(" L3,");
		return 3;
	}
	if ( 0 == WS_LEVEL_2) {
		printf(" L2,");
		return 2;
	}
	if( 0 == WS_LEVEL_1) {
		printf(" L1,");
        return 1;
	}
    return 0; //empty Tank
}

void prepare_sms_water_level(unsigned int water_level) {
	char temp_str_130[130];
    printf("Enter: prepare_sms_water_level(water_level=%d)\n", water_level);
	printf("before : send_sms_body_160 = %s\n", send_sms_body_160);
	if(water_level >= prov.water_level_prov.max_level) {
		strcat(send_sms_body_160, "TANK : FULL\n"); // turn off command
		send_sms_flag = true;
	} else if(0 == water_level) {
		strcat(send_sms_body_160, "TANK : EMPTY\n");//turn on motor
		send_sms_flag = true;
	} else {
		/* if(water_level == (prov.water_level_prov.max_level - 1)/2) {
			strcat(send_sms_body_160, "TANK : HALF\n");//turn on motor
		} */
    }
    snprintf(temp_str_130, sizeof(temp_str_130),
            "PRESENT LEVEL: %u\nMAX PROV LEVEL: %u\nFULL%% : %u%%",
            water_level,
            prov.water_level_prov.max_level,
            100 * water_level/prov.water_level_prov.max_level);
    strncat(send_sms_body_160, temp_str_130, sizeof(temp_str_130));
	printf("After : send_sms_body_160 = %s\n", send_sms_body_160);
    printf("Return : prepare_sms_water_level\n");
	return;
}

void process_tank_levels(void) {
	printf("Enter: process_tank_levels\n");
    printf("Before: level previous/current = %u/%u\n", oper.previous_water_level, oper.current_water_level);
    oper.current_water_level = read_sensor_val_from_hw();
    printf("\n");
    printf("After: level previous/current =  %u/%u\n", oper.previous_water_level, oper.current_water_level);
    if(oper.previous_water_level != oper.current_water_level) {
        printf("Reset everything and Start soaking\n");
        oper.previous_water_level = oper.current_water_level;
        oper.water_level_soaked_seconds = 0;
        oper.water_level_sms_sent = 0;
    }
    if(oper.water_level_sms_sent) {
        printf("SMS already sent. so, return");
        return;
    }
    oper.water_level_soaked_seconds++;
    printf("oper.soak/prov.soak: %u/%u \n", oper.water_level_soaked_seconds,
	    prov.water_level_prov.water_level_soak_seconds);

    if(true == send_sms_flag) {
        printf("another sms is being planned , so skip this time and do it in next iteration \n");
		return;
    }

    if(oper.water_level_soaked_seconds > prov.water_level_prov.water_level_soak_seconds) {
        printf("Soaking Over:Sending Sms\n");
        prepare_sms_water_level(oper.current_water_level);
        printf("send_sms_body_160=%s\n", send_sms_body_160);
        broadcast_sms(send_sms_body_160);
        oper.water_level_sms_sent = 1;
    } else {
        printf("Soaking In progress\n");
    }
    return;
}
