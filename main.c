#include <REG52.h>
#include <common.h>

/*
*** NOTES ***
(1) Modem data format \r\n<data>\r\n
(2) Use the global variable ble_or_modem to indicate
		MCU is working on the BLE device or MODEM. A multiplexer
		would be built to work with this variable.
*/

extern bit PIR; //set by interrupt.
extern unsigned char ble_or_modem;
extern volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
extern volatile unsigned int got_cr_lf;
extern volatile unsigned int index;
		 

void main(){
	P0 = 0xFF; 
	P1 = 0xFF;
	P2 = 0x00;
	delay(5); //parameter is multiples of 71ms. Use 14 for 1s
	P2 = 0xFF; //This and above is debug
	serialSetup('t'); //set timer for baudrate = 9600
	reset_serial_para();
	
	ble_or_modem = 'm'; 
	modemSetup((unsigned char)3); 

	ble_or_modem = 'b';  //don't forget to set CTRL accordingly.
	bluetoothStart('i'); //init
	P0 = 0x00; //debug
	EX0 = 1; //enable INT0
	while(1){
		P1 = 0x00;
		delay(6);
		P1 = 0xFF;
		delay(14);
	}
}
	
