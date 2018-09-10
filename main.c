#include <REG52.h>
#include <common.h>

#define NEXT //debug purposes only
/*
*** NOTES ***
(1) Modem data format \r\n+PBREADY\r\n
*/

extern volatile unsigned char RXDcmpt;
extern volatile unsigned char num_CR_LF;
extern volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
extern volatile unsigned char occurred_cr_lf;
unsigned long count = 0;

void main(){
	P0 = 0xFF;
	P1 = 0xFF;
	P2 = 0xFF;
	delay(2);
	reset_serial_para(); 
	serialSetup('t');
	serialAll("START\r\n"); //debug: see on serial monitor
	//while(occurred_cr_lf != 2); //wait for valid data
	serialSetup('t');
	serialAll("Just before checking data\r\n");
	while(1){ //interrupt should increment this. Nothing happens!
		count++;
		if(confirmData(rcvd_serial_data,STARTUP_RESP,strlen(STARTUP_RESP))){
			break;
		}
		if(count == 0xFFF){
			count = 0;
			serialAll("your number is: ");
			send(occurred_cr_lf);
			send('\n');
		}
	}
	P2 = 0x00;
	while(1){
	}
}
	
