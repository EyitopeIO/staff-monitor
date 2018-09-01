#include <functionDef2.h>
#include <bluetooth.h>

#ifndef REG52
#include <REG52.h>
#endif




//modemInit would use these below. I'm yet to edit.
unsigned char feb=1;
bit fl=0,RXDcmpt;
unsigned char SERIALcase= 1;
unsigned char receivedData[50], count = 0;

void main () {
	
	/* initialise serial port
	*  initialise modem
	*  initialise bluetooth
	*  wait for interrupt from PIR
	*  tell bluetooth to scan when you get interrupt
	*  send ID via text on modem
	*  repeat
	*/
	
	serialPortInit();
	modemInit();
	bluetoothInit();
}