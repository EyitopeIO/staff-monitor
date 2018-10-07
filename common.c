#include <common.h>

bit PIR = 0; //set by interrupt on P3^2
sbit CTRL = P3^3;

code const unsigned char *OK = "\r\nOK\r\n";
code const unsigned char *MODEM_TEST_STRING_OUT = "START\r";
code const unsigned char *MODEM_ECHO_OUT = "ATE0\r";
code const unsigned char *MODEM_NETWORK_IN = "\r\n+CREG: 0,1\r\n";
code const unsigned char *MODEM_NETWORK_OUT = "AT+CREG\r";
code const unsigned char *MODEM_STARTUP_RESP_IN = "\r\n+PBREADY\r\n";
code const unsigned char *MODEM_GSM_MODE_OUT = "AT+CSCS=\"GSM\"\r";
code const unsigned char *MODEM_TEXT_MODE_OUT = "AT+CMGF=1\r";
code const unsigned char *MODEM_PHONE_NUMBER_SELECT = "AT+CMGS=";
code const unsigned char *BLE_COMMAND_MODE_START_OUT = "$$$";
code const unsigned char *BLE_COMMAND_MODE_STOP_OUT = "---\r";
code const unsigned char *BLE_START_SCAN_OUT = "F\r";
code const unsigned char *BLE_STOP_SCAN_OUT = "X\r";
code const unsigned char *BLE_COMMAND_READY_IN = "CMD>";

code const unsigned char *phone = "\"+2348142357637\"\r";
code const unsigned char rand ='z'; //default char for reset_serial_para()

code const unsigned char HIGH = 0;
code const unsigned char PHIGH = 0x00;
code const unsigned char LOW = 1;
code const unsigned char PLOW = 0xFF;

const volatile unsigned char TX_SIZE = TX_MODEM_BUFFER_SIZE;
volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
volatile unsigned char transmit_to_modem[TX_MODEM_BUFFER_SIZE];
volatile unsigned char index = 0;
volatile unsigned char index_trans = 0; //reset this to zero after sending to modem
volatile unsigned char got_cr_lf = 0;
unsigned char ble_or_modem; //'b' for bluetooth, 'm' for modem
volatile unsigned char ble_case = 1;

bit got_carriage_ret = 0;
bit got_line_feed = 0;

//Set to 1 by default.
//Whoever doesnt't want incoming data to dusturb...
//should turn it off.
bit ready_for_data = 1;

sbit TX = P0^0;
sbit RX = P0^1;

void pir_interrupt(void) interrupt 0 
{
	PIR = 1;
}
bit confirmData(unsigned char *var_unsure,const unsigned char *var_sure,unsigned char len)
{
	/* This checks for your text within jargons */
	unsigned char i, j;
	ready_for_data = 0;
	for(i=0,j=0; i<length(var_unsure); i++)
	{
		if(var_unsure[i] == var_sure[j])
		{
			while(len--)
			{
				if(var_unsure[i++] != var_sure[j++])
				{
					ready_for_data = 1;
					return 0;
				}
			}
			ready_for_data = 1;
			return 1;
			//break; // successful comparison
		}
	}
	ready_for_data = 1;
	return 0;
}
bit bluetoothStart(unsigned char setup_para)
{
	unsigned char trial;
	unsigned char time; //actual value doesn't matter much
	set_TX_channel('b');
	switch(setup_para)
	{
		case 'i': //initialise
			time = 255;
			trial = 3;
			while(trial--)
			{
				sendCommand(BLE_COMMAND_MODE_START_OUT); //enter command mode
				while(time--); //delay
				if(confirmData(rcvd_serial_data, BLE_COMMAND_READY_IN, length(BLE_COMMAND_READY_IN))) return 1;
			}
			break;
		case 's': //scan
			P1 = PHIGH; //debug
			sendCommand(BLE_START_SCAN_OUT); //begin scan
			while(index_trans != TX_SIZE); //take only as much as array can hold
			//PIR is the interrupt 0 pin to connect PIR
			sendCommand(BLE_STOP_SCAN_OUT); //stop scan
			P1 = 0x00; //debug
			break;
	}
}
bit sendSMS(unsigned char *message)
{
	unsigned char count = 255; //actual number doesn't matter.
	unsigned char *last_element;
	ready_for_data = 0; //do not serial disturb me.
	set_TX_channel('m');
	last_element = copy(rcvd_serial_data, MODEM_PHONE_NUMBER_SELECT, length(MODEM_PHONE_NUMBER_SELECT));
	copy(last_element, phone, length(phone));
//	strcpy(rcvd_serial_data,MODEM_PHONE_NUMBER_SELECT);
//	strcat(rcvd_serial_data,phone);
	sendCommand(rcvd_serial_data);
	while(count--); //enough time to wait for '>'
	sendCommand(message);
	send(0x1A);
	ready_for_data = 1;
	return 1;
}
	
bit modemSetup(unsigned char trials)
{
	/* In auto baudrate mode, modem needs you to send something
	so it can detect baudrate */
	const unsigned char trial_lc = trials;
	set_TX_channel('m');
	P2 = 0xFE; // turn on P0^0
	while(trials--)
	{
		sendCommand(MODEM_TEST_STRING_OUT);
		while(got_cr_lf != 2); //wait till its done receiving
		if(confirmData(rcvd_serial_data,MODEM_STARTUP_RESP_IN,length(MODEM_STARTUP_RESP_IN)))
		{
			reset_serial_para(); //only 't' has effect
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xFC;  //turn on P0^1
	while(trials--)
	{
		sendCommand(MODEM_ECHO_OUT); //turn off echo
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,OK,length(OK)))
		{
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xF8; //turn on P0^2
	while(trials--){
		sendCommand(MODEM_NETWORK_OUT); //is network ready>
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,MODEM_NETWORK_IN,length(MODEM_NETWORK_IN)))
		{
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xF0; //turn on P0^3
	while(trials--)
	{
		sendCommand(MODEM_GSM_MODE_OUT); //set GSM to text mode
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,OK, length(OK)))
		{
			reset_serial_para();
			break;
		}
		reset_serial_para();
	}
	
	trials = trial_lc;
	P2 = 0xE0; //turn on P0^4
	while(trials--){
		sendCommand(MODEM_TEXT_MODE_OUT);
		while(got_cr_lf != 2);
		if(confirmData(rcvd_serial_data,OK, length(OK)))
		{
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
	
void send(unsigned char val)
{
	SBUF = val;
	while(!TI);
	TI=0;
}
void sendCommand(unsigned char *serial_data)
{
	const unsigned char len = length(serial_data);
	unsigned char counter = 0;
	//while(*serial_data != 0x00){ //null character is 0x00;
	while(counter != len)
	{
		TX = 0; //debug
		send(*serial_data); //SBUF = *serial_data; 
		serial_data++;
		counter++;
		TX = 1; //debug
	}
	TI = 0;
}
void set_TX_channel(unsigned char mode)
{
	switch(mode)
	{
		case 'm':
			sendCommand(BLE_COMMAND_MODE_STOP_OUT); //tell BLE to stop listening to commands
			ble_or_modem = 'm';
			CTRL = HIGH;
			break;
		case 'b':
			ble_or_modem = 'b';
			CTRL = LOW;
	}
}
void serialRX(void) interrupt 4
{
	/*
	*** NOTES ***
	(1) Always set index to 0 when expecting serial data.
	(2) RXDcmpt is a flag used to know that you've received CR-LF.
			Always set this to 0 when expecting serial data.
	(3) RI is the receive interrupt flag from a register.
			Also set to zero.
	*/
	unsigned char rcvd = SBUF;
	if(RI && ready_for_data)
	{
		switch(ble_or_modem)
		{
			case 'm': //for modem
				if(rcvd == '\r') got_carriage_ret = 1;
				if(rcvd == '\n') got_line_feed = 1;
				rcvd_serial_data[index] = rcvd;
				if(rcvd_serial_data[index] == '\n' && rcvd_serial_data[index-1] == '\r')
				{
					got_cr_lf++;
				}
				index++;
				break;
					
			case 'b':
				switch(ble_case)
				{
					case 1:
						if(rcvd == '%')
						{
							ble_case = 2;
						}
						break;
						
					case 2:
						if(rcvd == '\r' || rcvd == '\n')
						{
							ble_case =1;
						}
						else{
							if(rcvd != ',')
							{
								if(index_trans != TX_SIZE)
								{
									transmit_to_modem[index_trans++] = rcvd;
								}
							}
							else
							{
								ble_case = 1;
							}
						}
						break;
					}
				}
			}
	RI = 0;
}

void serialSetup(unsigned char mode)
{
	switch(mode)
	{
		case 't':
			TR0 = 0; //to be sure timer 0 goes offf
			SCON = 0x50;
			TMOD = 0x20;
			TH1 =  0xFD; // to set the baud rate to 9600
			ES = 1; //enable serial port interrupt 7
			EA= 1; //enable global interrupt
			TR1 = 1; //to start the timer 
			break;
		case 'f':
			EA = 0;
			break;
	}
}

void reset_serial_para()
{
	writeToArray(0x00, TX_MODEM_BUFFER_SIZE, transmit_to_modem);
	index_trans = 0;
	writeToArray(0x00, RX_BUFFER_SIZE, rcvd_serial_data);
	index = 0;
	got_cr_lf = 0;
	got_carriage_ret = 0;
	got_line_feed = 0;
}

void delay(unsigned int time)
{	
	//time in intervals of 71 ms.
	TMOD |= 0x01; //mode 1 of timer0
	TMOD &= ~(1<<1); 
	TMOD &= ~(1<<3); //use clock to count
	TH0 = 0x00;
	TL0 = 0x00;
	TF0 = 0;
	TR0 = 1; //start timer
	while(time--)
	{
		while(1)
		{
			if(TF0)
			{
				TH0 = 0x00;
				TL0 = 0x00;
				TF0 = 0;
				TR0 = 1;
				break;
			}
		}
	}
}
void writeToArray(unsigned char val, unsigned char array_lenght, unsigned char *array_address)
{
	while(array_lenght)
	{
		*array_address++ = val; 
		 array_lenght--;
	}
}
unsigned char length(unsigned char *string)
{
	unsigned char count = 0;
	while( *(string++) != 0x00)
	{
		count++;
	}
	return count;
}

unsigned char* copy(unsigned char *dest, unsigned char *source, unsigned char len)
{
	while(len--)
	{
		*dest = *source;
		dest++;
		source++;
	}
	return dest; //pointer to last element
}
