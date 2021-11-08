objects = 0_main.o 0_astro_new.o 0_gsm0_ubuntu.o 0_relay.o 0_misc.o 0_gsm_util.o 0_serial.o 0_dataflash.o 0_sms_util.o 0_fixed_timer.o 0_tank_sensor.o 0_app_dump.o

all : 0_uart.o
	gcc -c -DMACHINE=UBUNTU 0_main.c -lm -o 0_main.o
	gcc -c -DMACHINE=UBUNTU 0_astro_new.c -lm -o 0_astro_new.o.o
	gcc -c -DMACHINE=UBUNTU 0_gsm.c -lm -o 0_gsm.o
	gcc -c -DMACHINE=UBUNTU 0_ubuntu.c -lm -o 0_ubuntu.o
	gcc -c -DMACHINE=UBUNTU 0_relay.c -lm -o 0_relay.o
	gcc -c -DMACHINE=UBUNTU 0_misc.c -lm -o 0_misc.o
	gcc -c -DMACHINE=UBUNTU 0_gsm_util.c -lm -o 0_gsm_util.o
	gcc -c -DMACHINE=UBUNTU 0_serial.c -lm -o 0_serial.o
	gcc -c -DMACHINE=UBUNTU 0_dataflash.c -lm -o 0_dataflash.o
	gcc -c -DMACHINE=UBUNTU 0_sms_util.c -lm -o 0_sms_util.o
	gcc -c -DMACHINE=UBUNTU 0_fixed_timer.c -lm -o 0_fixed_timer.o
	gcc -c -DMACHINE=UBUNTU 0_tank_sensor.c -lm -o 0_tank_sensor.o
	gcc -c -DMACHINE=UBUNTU 0_app_dump.c -lm -o 0_app_dump.o
	gcc 0_main.o 0_astro_new.o 0_gsm.o 0_uart.o 0_ubuntu.o 0_relay.o 0_misc.o 0_gsm_util.o 0_serial.o 0_dataflash.o 0_sms_util.o 0_fixed_timer.o 0_tank_sensor.o 0_app_dump.o -lm -pthread -o main.exe

0_uart.o: 0_uart.c
	gcc -c -DMACHINE=UBUNTU 0_uart.c -lm -o 0_uart.o



clean:
	rm -rf *.o main.exe
