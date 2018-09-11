

/*
*** ALGORITHM ***

(1)Command the RN4871 to enter command mode
(2) Tell it to scan for advertisment data and inteprete. In our case, we only need this for what's below only.

%DCF740B78604,1,C8,Brcst:0201041AFF590002150112233445566778899AABBCCDDEEFF040B78604BB%

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



#include <REG52.h>
#define RX_BUFFER_SIZE 50
#define STARTUP_RESP "+PBREADY"
#define COMMON_H

void send(unsigned char val);
void sendCommand(unsigned char *serial_data);
void serialRX(void); //the interrupt function
void serialSetup(unsigned char mode);
void reset_serial_para(void);

bit confirmData(unsigned char *var_unsure, unsigned char *var_sure, unsigned char len);
unsigned char strlen(unsigned char *string);
void delay(unsigned int time);
void writeToArray(unsigned char val, unsigned char array_lenght, unsigned char *array_address);

