#include "0_all.h"
#include "0_main.h"

uint16_t i;
uint8_t kb_input, c;

#if MACHINE == UBUNTU
int main(void) {
    setvbuf (stdout, NULL, _IONBF, BUFSIZ);
	app_main();
}
#endif

void app_main(void) {
	uint8_t rx_buf_10[10];
	init_uart();
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> OM TAT SAT\n");
    printf("calling R_UART0_Receive_non_blocking\n");
    R_UART0_Receive_non_blocking(rx_buf_10, 1);
    //while(1);
//#define str ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>OM TAT SAT\n"
//    R_UART0_Send_blocking((uint8_t * __near)str, sizeof(str));
#if MACHINE == UBUNTU
    printf("\n sz=> prov=%lu, oper=%lu\n", sizeof(prov_t), sizeof(oper_t));
#else
    printf("\n sz=> prov=%u, oper=%u\n", sizeof(prov_t), sizeof(oper_t));
#endif
    //i2c_init(); diag_gsm();
    data_flash_driver_init();
#if 0
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! Only do at the time first time configuration !!!!!!!!!!!!!!!!!!
    //process_keyboard('7'); // provision to defulat + prov MODEM
    provision_to_defaults();
  	enable_date_time_sync_and_sms_storage_area();
    dataflash_to_ram();
    dump_conf_oper();
    while(1) {
        R_WDT_Restart();
        printf("reboot NOW");
        sleep_ms(1000);
    }
#endif
    dataflash_to_ram();
#if MACHINE == UBUNTU
    provision_to_defaults();
#endif
    power_up_sms_append();
    init_oper_data();
    dump_conf_oper();
    act_on_mcu_boot_delay(); // how much to sleep before we go to GSM modem?
    init_gsm1();
    R_WDT_Restart();
    rx_buf_10[0]='\0';
    //gprs_testing();
    printf("Almost everything is done 1\n");
	if(keyboard_enabled) {
		R_UART0_Receive_non_blocking(rx_buf_10, 1);
	}
    printf("Entering forever loop\n");
    while(1){
        printf("#############################################################################################################\n");
        R_WDT_Restart();sleep_ms(1000);R_WDT_Restart();
        if(keyboard_enabled) {
        	if (1 == has_R_UART0_Receive_data()) {
        		printf("u entered = <%c>\n", rx_buf_10[0]);
        		//process_keyboard(rx_buf_10[0]); do not call it in feild.
        		R_UART0_Receive_non_blocking(rx_buf_10, 1);
        	}
        }
        process_modem();
    }
}

void act_on_mcu_boot_delay(void) {
	unsigned int temp_uint;
    printf("\n >>> act_on_mcu_boot_delay() \n prov.basic_prov.mcu_boot_delay = %hhu",
            (unsigned int)prov.basic_prov.mcu_boot_delay);
	/* sleep to let the Network connection.
     * TODO poll the Modem for this so that we don't need to wait like this */
    temp_uint = prov.basic_prov.mcu_boot_delay;
    if(temp_uint > const_mcu_boot_delay_max){
    	temp_uint = const_mcu_boot_delay_min;
    }

	while(temp_uint--) {
		printf("%d, ", temp_uint);
		R_WDT_Restart(); sleep_ms(1000); R_WDT_Restart();
	}
}
