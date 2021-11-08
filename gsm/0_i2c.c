#include "0_all.h"

#define SLAVE_ADDRESS 0xd1

volatile unsigned int R_IICA0_Master_Send_end;
volatile unsigned int R_IICA0_Master_receiveend;

void wait_till_send_ends_timeout(void)
{
	while(R_IICA0_Master_Send_end) { //TODO rest the watch dog timer...
	}
}

void wait_till_receive_ends_timeout(void)
{
	while(R_IICA0_Master_receiveend) { //TODO rest the watch dog timer...
	}
}

void i2c_init(void)
{
	MD_STATUS status;
	unsigned int i;

	unsigned char Tx_Buffer[0x13]={0x00, 0x00,0x00,0x00,0x00,0x30,0x11,0x20};
	unsigned char rx_buf[0x13];
	//unsigned int Tx_end,Rx_end; TODO read the INT to see if writing completed

	R_IICA0_Master_Send(SLAVE_ADDRESS, Tx_Buffer, 8, 0xff);
	wait_till_send_ends_timeout();
	//R_WDT_Restart();sleep_ms(2000);R_WDT_Restart();

	R_IICA0_Master_Send(SLAVE_ADDRESS, Tx_Buffer, 1, 0xff);
	wait_till_receive_ends_timeout();
	//R_WDT_Restart();sleep_ms(2000);R_WDT_Restart();

  	while(1){
  		printf("\n\r");
  		status = R_IICA0_Master_Receive(SLAVE_ADDRESS, rx_buf, sizeof(rx_buf), 0xff);
  		R_WDT_Restart();sleep_ms(2000);R_WDT_Restart();
   		if(MD_OK == status) {
   			for (i=0; i<sizeof(rx_buf); i++) {
   				printf(" %x ", rx_buf[i]);
   			}
   		} else {
   			printf("\n ERROR ");
   		}
   	}
}
