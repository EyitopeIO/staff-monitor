

/*
*** ALGORITHM ***
(1) Setup modem and BLE
(2) PIR going high triggers interrupt that sets a flag. MCU polls the flag.
(3) If high, turn off interrupt, scan, send to modem, resume. In future, 
		transmission would be to some server


Below is the format of the bluetooth data.

%<address>,<type>,<RSSI>,Brcst:<data>%
%DCF740B78604,1,C8,Brcst:0201041AFF590002150112233445566778899AABBCCDDEEFF040B78604BB%\r\n%BFF2140B33604,0,FF,Brcst:F2010317DA590002BA3812233445566778899AABB123AFEFF040B78604BB%\r\n
%DCF740B78604,1,C8,Brcst:0201041AFF590002150112233445566778899AABBCCDDEEFF040B78604BB%\r\n%BFF2140B33604,0,FF,Brcst:F2010317DA590002BA3812233445566778899AABB123AFEFF040B78604BB%\r\n
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

#define RX_BUFFER_SIZE 25
#define TX_MODEM_BUFFER_SIZE 25 //for 2 times 12 bytes plus two times '%' 


void send(unsigned char val);
void sendCommand(const unsigned char *serial_data);
void serialRX(void); //serial interrupt function
void serialSetup(unsigned char mode);
void reset_serial_para(void);
void set_TX_channel(unsigned char mode);

bit sendSMS(unsigned char *message); //bit sendSMS(unsigned char message);
bit modemSetup(unsigned char trials);
bit bluetoothStart(unsigned char setup_para); //enter command mode and begin scanning

bit confirmData(unsigned char *var_unsure, const unsigned char *var_sure, unsigned char len);
unsigned char length(unsigned char *string);
void delay(unsigned int time);
void writeToArray(unsigned char val, unsigned char array_lenght, unsigned char *array_address);
unsigned char* copy(unsigned char *dest, unsigned char *source, unsigned char len);
