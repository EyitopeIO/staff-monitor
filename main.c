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
(4) Ensure serialSetup() is run at least once after every use of delay()
(5) ready_for_data is a variable cleared by a function that doesn't want to
	be disturbed with incoming serial data.
*/

sbit CTRL = P3^3; //for the multiplexer

extern bit PIR;
extern bit ready_for_data;
extern code unsigned char rand;  
extern code const unsigned char HIGH;
extern code const unsigned char PHIGH;
extern code const unsigned char LOW;
extern code const unsigned char PLOW;
extern volatile unsigned char transmit_to_modem[TX_MODEM_BUFFER_SIZE];
extern volatile unsigned char ble_case;

void main()
{
	serialSetup('t'); //set timer for baudrate = 9600
	P3 |= (1<<2); //set P3.2 as input
	P3 &= ~(1<<3); //set P3.3 as output;
	ready_for_data = 1;
	modemSetup1();
	P2 = PLOW;
	bluetoothStart('i'); //init
	EX0 = 1; //enable INT0 interrupt
	while(1)
	{
		P1 = PHIGH;
		P2 = PLOW;
		if(PIR)
 
		{ //0 for high; 1 for low
			EX0 = 0; //disable INT0
			PIR = 0; //clear PIR flag; set by interrupt
			ble_case = 1;
			serialSetup('t');
			reset_serial_para();
			bluetoothStart('s');
			sendSMS(transmit_to_modem);
			reset_serial_para();
			EX0 = 1; //enable INT0	
		}
		delay(1);
		P1 = PLOW;
		delay(1);	
	}
}
	
