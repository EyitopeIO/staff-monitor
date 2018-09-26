#include <REG52.h>
#include <common.h>

/*
*** NOTES ***
(1) Modem data format \r\n<data>\r\n
(2) Global variable ble_or_modem tells if
		MCU is working on the BLE device or MODEM. A multiplexer
		would be built to work with this variable.
(3) In the function rset_serial_para(), only using 't' as a parameter
		has effect. Others are ignored; hence the purpose of rand.
*/

sbit PIR = P3^2; //PIR to be connected to this.
sbit CTRL = P3^3; //for the multiplexer

extern code unsigned char rand;  
extern code const unsigned char HIGH;
extern code const unsigned char PHIGH;
extern code const unsigned char LOW;
extern code const unsigned char PLOW;

void main(){
	P0 = PLOW; //port low
	P1 = PLOW; //port high
	P2 = PHIGH;
	delay(5); //parameter is multiples of 71ms. Use 14 for just about 1 sec
	P2 = PLOW; //This and above is debug
	serialSetup('t'); //set timer for baudrate = 9600
	reset_serial_para(rand); 
	P3 |= (1<<2); //set P3.2 as input
	P3 &= ~(1<<3); //set P3.3 as output;
	modemSetup((unsigned char)3);
	bluetoothStart('i'); //init
	P0 = PHIGH; //debug
	while(1){
		P1 = PHIGH;
		if(!PIR){ //0 for high; 1 for low
			pirHandle();
			reset_serial_para(rand);
		}
		delay(6);
		P1 = PLOW;
		delay(6);
		
	}
}
	
