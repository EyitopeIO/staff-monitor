#include <REG52.h>
#include <common.h>

#define NEXT //debug purposes only
/*
*** NOTES ***
(1) Modem data format \r\n+PBREADY\r\n
*/

extern volatile bit RXDcmpt;
extern volatile unsigned char num_CR_LF;
extern volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
extern volatile unsigned char occurred_cr_lf;
unsigned char *progress;

void main(){
	*progress = 0x00; 
	P1 = 0xFF; //step 0 
	serialSetup('t');
	serialAll("START"); //debug: see on serial monitor
	while(occurred_cr_lf != 2); //wait for the closing \r\n
	INC(&progress); //step 1
	if(confirmData(rcvd_serial_data, STARTUP_RESP, strlen(STARTUP_RESP))){    //check for +PBREADY in the RX buffer
		INC(progress); //step 2
	}
	P2 = 0xFF;
	while(1){
	}
	//check for the array here
	
	
}
