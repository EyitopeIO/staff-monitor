

/*
*** ALGORITHM ***
(1) Setup modem and BLE
(2) If PIR goes high, scan for bluetooth devices for as long as PIR is high, 
		then send to server. For now, this server is just a person's phone.
(3) Chill while PIR is low. 

Below is the format of the bluetooth data.

%<address>,<type>,<RSSI>,Brcst:<data>%
%DCF740B78604,1,C8,Brcst:0201041AFF590002150112233445566778899AABBCCDDEEFF040B78604BB%\r\n%FDE34...

*** SETTINGS ***
baud rate = 9600
SA,2  -> No security AOK/ERR
SB,09  -> baud rate 9600 AOK/ERR
SC,0  -> beacon disabled AOK/ERR
SF,1  -> factory reset ERR/it reboots
&C  -> use local MAC to advertise AOK
F[,<interval>,<window>]  -> scan
JA,0,<MAC> --> add to white list AOK/ERR
JC  -> clear whitelist AOK
X  -> stop scan AOK
*/

#define COMMON_H
#include <REG52.h>

void send(unsigned char val);
void sendCommand(const unsigned char *serial_data);
void serialRX(void); //serial interrupt function
void pirHandle();
void serialSetup(unsigned char mode);
void reset_serial_para(unsigned char mode);
void set_TX_channel(unsigned char mode);

bit sendSMS(unsigned char *message); //bit sendSMS(unsigned char message);
bit modemSetup(unsigned char trials);
bit bluetoothStart(unsigned char setup_para); //enter command mode and begin scanning

bit confirmData(unsigned char *var_unsure, const unsigned char *var_sure, unsigned char len);
unsigned char stringLen(unsigned char *string);
void delay(unsigned int time);
void writeToArray(unsigned char val, unsigned char array_lenght, unsigned char *array_address);
