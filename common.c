#include <common.h>

volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
volatile unsigned char index = 0;
volatile bit RXDcmpt = 0;
volatile unsigned char num_CR_LF = 0; //count number of times we see \r\n
volatile bit got_carriage_ret = 0;
volatile bit got_line_feed = 0;
volatile unsigned char occurred_cr_lf = 0;

sbit TX = P0^0;
sbit RX = P0^1;

void send(unsigned char val){
	SBUF = val;
	while(!TI);
	TI=0;
}
void serialAll(unsigned char *serial_data){
	while(*serial_data != 0x00){ //null character is 0x00;
		TX = 0; //debug
		send(*serial_data); //SBUF = *serial_data; 
		serial_data++;
		TX = 1; //debug
	}
	TI = 0;
}

void serialRX(void) interrupt 4{
	/*
	*** NOTES ***
	(1) Always set index to 0 when expecting serial data.
	(2) RXDcmpt is a flag used to know that you've received CR-LF.
			Always set this to 0 when expecting serial data.
	(3) RI is the receive interrupt flag from a register.
			Also set to zero.
	*/
	
	RX = 0; //debug
	if(RI){
		rcvd_serial_data[index] = SBUF;
		if(SBUF == '\r'){
			got_carriage_ret = 1;
		}
		if(SBUF == '\n'){
			got_line_feed = 1;
		}
		if(rcvd_serial_data[index] == '\n' && rcvd_serial_data[index-1] == '\r'){
			occurred_cr_lf++;
		}
		index++;
	}
	RI = 0;
	RX = 1;
}

void serialSetup(unsigned char global_interrupt){
	SCON = 0x50;
	TMOD = 0x20;
	TH1 =  0xFD; // to set the baud rate to 9600
	TR1 = 1; //to start the timer 
	ES = 1; //enable serial port interrupt 7
	switch(global_interrupt){
		case 't':
			EA = 1;
		case 'f':
			EA = 0;
	}
	index = 0; RXDcmpt = 0; got_carriage_ret = 0; got_line_feed = 0;
}



void reset_serial_para(void){
	// Use this after sending serial data
	writeToArray(0x00, RX_BUFFER_SIZE, rcvd_serial_data);
	RXDcmpt = 0;
	index = 0;
	num_CR_LF = 0;
	got_carriage_ret = 0;
	got_line_feed = 0;
}

void delay(unsigned int time){
	unsigned char num_of_times = 0;
	TMOD |= 0x01; //mode 1 of timer0
	TMOD &= ~(1<<1); 
	TMOD &= ~(1<<3); //use clock to count
	TH0 = 0x00;
	TL0 = 0x00;
	TF0 = 0;
	TR0 = 1; //start timer
	while(time--){
		while(num_of_times != 14){
			if(TF0){
				num_of_times++;
				TH0 = 0x00;
				TL0 = 0x00;
				TF0 = 0;
				TR0 = 1;
			}
		}
		num_of_times = 0;
		}
}

unsigned char strlen(unsigned char *string){
	unsigned char count = 0;
	while( *(string++) != 0x00){
		count++;
	}
	return count;
}
void writeToArray(unsigned char val, unsigned char array_lenght, unsigned char *array_address){
	while(array_lenght){
		*array_address++ = val; 
		 array_lenght--;
	}
}
bit confirmData(unsigned char *var_unsure,unsigned char *var_sure,unsigned char len){
	int i, j;
	for(i=0,j=0; i<len; i++){
		if(var_unsure[i] == var_sure[j]){
			while(len--){
				if(var_unsure[i++] != var_sure[j++]){
					return 0;
				}
			}
			return 1; // successful comparison
		}
	}
}
void INC(unsigned char *val){
	/* used to show where in the code is executing */
	(*val)++;
	P0 = *val;
}