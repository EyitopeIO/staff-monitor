#include <common.h>

volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
volatile unsigned char transmit_to_modem[TX_MODEM_BUFFER_SIZE];
volatile unsigned int index = 0;
volatile unsigned int got_cr_lf = 0;
unsigned char ble_or_modem; //'b' for bluetooth, 'm' for modem
volatile unsigned char ble_delim_num = 0;

volatile bit PIR = 0; //flipped by INT0
bit got_carriage_ret = 0;
bit got_line_feed = 0;
bit got_comma = 0;

sbit TX = P0^0;
sbit RX = P0^1;
bit done = 0; //debug

void pirHandle(void) interrupt 0{ //INT0
	//turn on an LED to show it has seen something
	//PIR is set to zero when scanning is complete
	EX0 = 0;
	PIR = 1;
	ble_or_modem = 'b';
	bluetoothStart('s');
	//send message
	EX0 = 1;
}

bit bluetoothStart(unsigned char setup_para){
	unsigned char trial;
	unsigned int time; //actual value doesn't matter much
	switch(setup_para){
		case 'i': //initialise
			time = 5000;
			trial = 3;
			reset_serial_para();
			while(trial--){
				sendCommand("$$$"); //enter command mode
				while(time--); //delay
				if(confirmData(rcvd_serial_data, "CMD>", strlen("CMD>"))) return 1;
			}
			break;
		case 's': //scan
			P1 = 0xF0; //debug
			reset_serial_para();
			sendCommand("F\r"); //begin scan
			while(!done); //debug, so it waits for me to type
			sendCommand(transmit_to_modem); //debug. Send to serial monitor
		//PIR is the interrupt 0 pin to connect PIR
			sendCommand("X\r"); //stop scan
			P1 = 0x00; //debug
			PIR = 0;
			P1 = 0xFF; //debug
			break;
	}
}
		
bit modemSetup(unsigned char trials){
	/* In auto baudrate mode, modem needs you to send something
	so it can detect baudrate */
	const unsigned char trial_lc = trials;
	EX0 = 0; //disable INT0 
	P2 = 0xFE; // turn on P0^0
	while(trials--){
		sendCommand("START\r\n");
		while(got_cr_lf != 2); //wait till its done receiving
		if(confirmData(rcvd_serial_data,STARTUP_RESP,strlen(STARTUP_RESP))){
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xFC;  //turn on P0^1
	while(trials--){
		sendCommand("ATE0\r"); //turn off echo
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,OK,strlen(OK))){
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xF8; //turn on P0^2
	while(trials--){
		sendCommand("AT+CREG?\r"); //is network ready>
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,NETWORK,strlen(NETWORK))){
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xF0; //turn on P0^3
	while(trials--){
		sendCommand("AT+CSCS=\"GSM\"\r"); //set GSM to text mode
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,OK, strlen(OK))){
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xE0; //turn on P0^4
	while(trials--){
		sendCommand("AT+CMGF=1\r");
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,OK, strlen(OK))){
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	/*
	set phone number to send text to, then send the text
	*/
	P2 = 0x00; //done
	return 1;
}
	
void send(unsigned char val){
	SBUF = val;
	while(!TI);
	TI=0;
}
void sendCommand(unsigned char *serial_data){
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
	unsigned char rcvd = SBUF;
	P1 = 0x00; //debug
	if(RI){
		switch(ble_or_modem){
			case 'm': //for modem
				if(rcvd == '\r') got_carriage_ret = 1;
				if(rcvd == '\n') got_line_feed = 1;
				rcvd_serial_data[index] = rcvd;
				if(rcvd_serial_data[index] == '\n' && rcvd_serial_data[index-1] == '\r'){
					got_cr_lf++;
				}
				index++;
				break;
			case 'b':
				/* In <common.h> is the data format. What's required here
						is a way of storing just the address. transmit_to_modem
						is an array to keep the addresses pending transmission.
						Happy coding!
				*/
				if(rcvd == ','){
					got_comma = 1;
					P0 = 0xFF; //debug
				}
				if(rcvd == '%'){
					ble_delim_num++; //opening '%'
					P0 = 0x0F; //debug
				}
				if(ble_delim_num == 2){
					ble_delim_num = 0; //closing '%'
					got_comma = 0; //prepare to get first comma after next opening of '%'
					done = 1;
				}
				
				//If opening '%' and you haven't seen a comma.
				//Save until you see a comma
				if(ble_delim_num == 1  && !got_comma){
					transmit_to_modem[index++] = rcvd;
				}
		}
	}
	P1 = 0xFF;
	RI = 0;
}

void serialSetup(unsigned char mode){
	TR0 = 0; //to be sure timer 0 goes offf
	SCON = 0x50;
	TMOD = 0x20;
	TH1 =  0xFD; // to set the baud rate to 9600
	TR1 = 1; //to start the timer 
	ES = 1; //enable serial port interrupt 7
	switch(mode){
		case 't':
			EA= 1;
			break;
		case 'f':
			EA = 0;
			break;
	}
}

void reset_serial_para(void){
	// Use this after sending serial data
	writeToArray(0x00, RX_BUFFER_SIZE, rcvd_serial_data);
	writeToArray(0x00, TX_MODEM_BUFFER_SIZE, transmit_to_modem);
	index = 0;
	got_cr_lf = 0;
	got_carriage_ret = 0;
	got_line_feed = 0;
	got_comma = 0;
}

void delay(unsigned int time){ //time in intervals of 71 ms.
	TMOD |= 0x01; //mode 1 of timer0
	TMOD &= ~(1<<1); 
	TMOD &= ~(1<<3); //use clock to count
	TH0 = 0x00;
	TL0 = 0x00;
	TF0 = 0;
	TR0 = 1; //start timer
	while(time--){
		while(1){
			if(TF0){
				TH0 = 0x00;
				TL0 = 0x00;
				TF0 = 0;
				TR0 = 1;
				break;
			}
		}
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
	/* This checks for your text within jargons */ 
	int i, j;
	for(i=0,j=0; i<strlen(var_unsure); i++){
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
