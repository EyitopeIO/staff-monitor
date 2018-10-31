#include <common.h>
#include <string.h>

bit PIR = 0; //set by interrupt on P3^2
sbit CTRL = P3^3;

code const unsigned char *AT_OUT = "AT\r";
code const unsigned char *OK_IN = "OK";
code const unsigned char *MODEM_TEST_STRING_OUT = "START\r";
code const unsigned char *MODEM_ECHO_OUT = "ATE0\r";
code const unsigned char *MODEM_NETWORK_IN = "+CREG: 0,1";
code const unsigned char *MODEM_NETWORK_OUT = "AT+CREG?\r";
code const unsigned char *MODEM_STARTUP_RESP_IN = "MODEM:STARTUP";
code const unsigned char *PBREADY_IN = "+PBREADY";
code const unsigned char *MODEM_GSM_MODE_OUT = "AT+CSCS=\"GSM\"\r";
code const unsigned char *MODEM_TEXT_MODE_OUT = "AT+CMGF=1\r";
code const unsigned char *MODEM_PHONE_NUMBER_SELECT = "AT+CMGS=";
code const unsigned char *BLE_COMMAND_MODE_START_OUT = "$$$";
code const unsigned char *BLE_COMMAND_MODE_STOP_OUT = "---\r";
code const unsigned char *BLE_START_SCAN_OUT = "F\r";
code const unsigned char *BLE_STOP_SCAN_OUT = "X\r";
code const unsigned char *BLE_COMMAND_READY_IN = "CMD>";

code const unsigned char *phone = "\"+2348142357637\"\r";

code const unsigned char HIGH = 0; //debug!
code const unsigned char PHIGH = 0x00;
code const unsigned char LOW = 1;
code const unsigned char PLOW = 0xFF;

const volatile unsigned char TX_SIZE = TX_MODEM_BUFFER_SIZE;
volatile unsigned char rcvd_serial_data[RX_BUFFER_SIZE];
volatile unsigned char transmit_to_modem[TX_MODEM_BUFFER_SIZE];
volatile unsigned char index = 0;
volatile unsigned char index_trans = 0; //reset this to zero after sending to modem
volatile unsigned char ble_case = 1;
volatile unsigned char modem_case = 1;
volatile unsigned char modem_data_count = 0;
unsigned char ble_or_modem; //'b' for bluetooth, 'm' for modem

//Set to 1 by default.
//Whoever doesnt't want incoming data to dusturb...
//should turn it off.
bit ready_for_data = 1;

bit modem_data_complete = 0;

void pir_interrupt(void) interrupt 0 
{
	PIR = 1;
}

void bluetoothStart(unsigned char setup_para)
{
	unsigned char trial;
	unsigned char time; //actual value doesn't matter much
	set_TX_channel('b');
	ready_for_data = 1;
	switch(setup_para)
	{
		case 'i': //initialise
			time = 255;
			trial = 3;
			while(trial--)
			{
				sendCommand(BLE_COMMAND_MODE_START_OUT); //enter command mode
				
				//Enough time for data to enter
				while(time--);
				
				ready_for_data = 0;
				if(strstr(rcvd_serial_data, BLE_COMMAND_READY_IN) != NULL)
				{
					break;
				}
				ready_for_data = 1;
			}			
			break;
		case 's': //scan
			ready_for_data = 1;
			trial = 5;
			P1 = PHIGH; //debug
			sendCommand(BLE_START_SCAN_OUT);
			while(trial--)
			{	
				//ble_case=2 when opening flag
				//of BLE data is received
				if(ble_case >= 2)
				{
					break;
				}
				else
				{
					sendCommand(BLE_START_SCAN_OUT);
				}
			}
			while(index_trans != TX_SIZE); //take only as much as array can hold
			sendCommand(BLE_STOP_SCAN_OUT); //stop scan
			P1 = 0x00; //debug
	}
}
void sendSMS(unsigned char *message)
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
}
void modemSetup1(void)
{
	volatile unsigned char state = 2;
	
	//Enough time for modem 
	//to get itself ready (30s)
	delay(420); 
	
	serialSetup('t');
	reset_serial_para();
	set_TX_channel('m');
	while(1)
	{
		if(state > 6)
		{
			break;
		}
		switch(state)
		{
			case 2:
				P2 = 0xFC;  //turn on P0^1
				sendCommand(AT_OUT);
				while(!modem_data_complete);
				ready_for_data = 0;
				if((strstr(rcvd_serial_data,OK_IN) != NULL))
				{
					sendCommand("Pass 2.");
					state = 3;
				}
				else
				{
					sendCommand("Failed 2. ");
					sendCommand(rcvd_serial_data);
					state = 2;
				}
				delay(14);
				serialSetup('t');
				ready_for_data = 1;
				reset_serial_para();
				break;
			case 3:
				P2 = 0xF8; //turn on P0^2
				sendCommand(MODEM_ECHO_OUT); //turn off echo
				while(!modem_data_complete); //wait till its done receiving
				ready_for_data = 0;
				if(strstr(rcvd_serial_data, OK_IN) != NULL)
				//if(confirmData(rcvd_serial_data,OK_IN,length(OK_IN)))
				{
					sendCommand("Pass 3.");
					state = 4;
				}
				else
				{
					state = 3;
					sendCommand("Failed 3. ");
					sendCommand(rcvd_serial_data);
				}
				delay(14);
				serialSetup('t');
				ready_for_data = 1;
				reset_serial_para();
				break;
			case 4:
				P2 = 0xF0; //turn on P0^3
				sendCommand(MODEM_NETWORK_OUT); //is network ready?
				while(!modem_data_complete); //wait till its done receiving
				ready_for_data = 0;
				if(strstr(rcvd_serial_data,MODEM_NETWORK_IN) != NULL)
				/* If "+CREG:0, 3" after a couple of trials,
					you could signal the user that something
					is wrong with an indicator LED. It means
					network registration was denied. 
				*/
				//if(confirmData(rcvd_serial_data,MODEM_NETWORK_IN,length(MODEM_NETWORK_IN)))
				{
					sendCommand("Pass 4.");
					state = 5;	
				}
				else
				{
					sendCommand("Failed 4. ");
					state = 4;
					sendCommand(rcvd_serial_data);
				}
				delay(14);
				serialSetup('t');
				ready_for_data = 1;
				reset_serial_para();
				break;
			case 5:
				P2 = 0xE0; //turn on P0^4
				sendCommand(MODEM_GSM_MODE_OUT); //set GSM to text mode
				while(!modem_data_complete); //wait till its done receiving
				ready_for_data = 0;
				if(strstr(rcvd_serial_data,OK_IN) != NULL)
				//if(confirmData(rcvd_serial_data,OK_IN, length(OK_IN)))
				{
					sendCommand("Pass 5.");
					state = 6;
				}
				else
				{
					sendCommand("Failed 5. ");
					state = 5;
					sendCommand(rcvd_serial_data);
				}
				delay(14);
				serialSetup('t');
				ready_for_data = 1;
				reset_serial_para();
				break;
			case 6:
				P2 = 0xC0; //turn on P0^5
				sendCommand(MODEM_TEXT_MODE_OUT);
				while(!modem_data_complete); //wait till its done receiving
				ready_for_data = 0;
				if(strstr(rcvd_serial_data,OK_IN) != NULL)
				//if(confirmData(rcvd_serial_data,OK_IN, length(OK_IN)))
				{
					sendCommand("Pass 6. ");
					state = 7;
				}
				else
				{
					sendCommand("Failed 6. ");					
					state = 6;
					sendCommand(rcvd_serial_data);
				}
				delay(14);
				serialSetup('t');
				ready_for_data = 1;
				reset_serial_para();
				break;
		}
	}
	P2 = 0x00;
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
		send(*serial_data); //SBUF = *serial_data; 
		serial_data++;
		counter++;
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
			CTRL = LOW; //It is open drain. Keep it off to allow a voltage there.
			break;
		case 'b':
			ble_or_modem = 'b';
			CTRL = HIGH;
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
			case 'm':
				switch(modem_case) //initialised to 1 as global var.
				{
					case 1:
						if(rcvd == 0x0D) // '\r'
						{
							modem_case = 2;
						}
						break;
					case 2:
						if(rcvd == 0x0A) // '\n'
						{
							modem_case = 3;
						}
						break;
					case 3:
						rcvd_serial_data[index++] = rcvd;
						if(rcvd == 0x0D)
						{
							modem_case = 4;
						}
						break;
					case 4:
						if(rcvd == 0x0A)
						{
							//Last element in the array
							//assuming we've gotten all we need.
							index-=1;
							modem_case = 1;
							modem_data_complete = 1;
						}
						else //or else, it's useful data
						{
							rcvd_serial_data[index++] = rcvd;
							modem_case = 3;
						}
						break;
				}	
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
	//got_cr_lf = 0;
	//got_carriage_ret = 0;
	//got_line_feed = 0;
	modem_data_complete = 0;
}

void delay(unsigned int time)
{	
	//time=1 gives about 71 ms.
	TMOD |= 0x01; //mode 1 of timer0
	TMOD &= ~(1<<1); 
	TMOD &= ~(1<<3); //use clock to count
	TH0 = 0x00;
	TL0 = 0x00;
	TF0 = 0;
	TR0 = 1; //start timer
	while(time--)
	{
		//flash indicator here
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
