#include <REG52.h>
#include <common.h>

#
/*
*** NOTES ***
(1) Modem data format \r\n<data>\r\n

*** ALGORITHM ***
1. Wait for detected motion from PIR
2. If motion, beginning scanning immediately
3. Note all visible bluetooth devices and send to server
4. Server notes the ID and takes attendance
5. For now, we'd just send via GSM
*/

extern bit RXDcmpt;
extern volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
extern volatile unsigned int occurred_cr_lf;
extern volatile unsigned int index;
		 

void main(){
	P0 = 0xFF;
	P1 = 0xFF;
	P2 = 0x00;
	delay(5);
	P2 = 0xFF;
	serialSetup('t'); //set for baudrate = 9600
	reset_serial_para();
	modemSetup((unsigned char)3); //repeat each command 3 times if failure
	bluetoothStart('i'); //bluetooth setup
	P0 = 0x00;
	
	while(1); //Pending overall logic
}
	
