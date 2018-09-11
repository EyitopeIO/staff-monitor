#include <REG52.h>
#include <common.h>

#define NEXT //debug purposes only
/*
*** NOTES ***
(1) Modem data format \r\n+PBREADY\r\n
*/

extern bit RXDcmpt;
extern volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
extern volatile unsigned int occurred_cr_lf;
extern volatile unsigned int index;
unsigned long count = 0;

void main(){
	P0 = 0xFF;
	P1 = 0xFF;
	P2 = 0xFF;
	delay(2); 
	serialSetup('t');
	reset_serial_para();
	sendCommand("START\r\n"); //debug: see on serial monitor
	sendCommand("Just before checking data\r\n");
	while(occurred_cr_lf != 2);
	if(confirmData(rcvd_serial_data,STARTUP_RESP,strlen(STARTUP_RESP))){
		P2 = 0x00;
	}
	P0 = 0x00;
	while(1);
}
	
