// New functions added from 31-08-18
#define REG52
#include <REG52.h>

void sendAT(unsigned char *buf, unsigned char val);
void BYTEwrite( unsigned char VAL, unsigned char buffLEN, unsigned char *buffADDR);
void serialPortInit();
int modemInit();
bit confirmData(char *var_unsure, char *var_sure, int len);

void sendCommand(unsigned char *buf, unsigned char val); //sendAT code is in here

#define pb "+PBREADY"
#define network "+CREG: 0,1"
#define YES "OK"

sbit pir=P2^0;

/* TASKS AND TO-LOOK
* 1. i think i'd make another interrupt code
* 2. use his check_buff to confirm response from modem
* 3. he uses BYTEwrite to clear array. you can use it for any array in your code here
*/
