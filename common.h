

/*
*** ALGORITHM ***

(1)Command the RN4871 to enter command mode
(2) Tell it to scan for advertisment data and inteprete. In our case, we only need this for what's below only.
		BLE device appends "\r\n" for every BLE it finds

%<address>,<type>,<RSSI>,Brcst:<data>%
%DCF740B78604,1,C8,Brcst:0201041AFF590002150112233445566778899AABBCCDDEEFF040B78604BB%\r\n%FDE34...

(3) When you find it, exit command mode

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
#define RX_BUFFER_SIZE 25
#define TX_MODEM_BUFFER_SIZE 30 //for 5 times 6 bytes  
#define STARTUP_RESP "+PBREADY"
#define OK "OK"
#define NETWORK "+CREG: 0,1"



void send(unsigned char val);
void sendCommand(unsigned char *serial_data);
void serialRX(void); //serial interrupt function
void pirHandle(void); //external interrupt function
void serialSetup(unsigned char mode);
void reset_serial_para(void);

bit modemSetup(unsigned char trials);
bit bluetoothStart(unsigned char setup_para); //enter command mode and begin scanning

bit confirmData(unsigned char *var_unsure, unsigned char *var_sure, unsigned char len);
unsigned char strlen(unsigned char *string);
void delay(unsigned int time);
void writeToArray(unsigned char val, unsigned char array_lenght, unsigned char *array_address);
