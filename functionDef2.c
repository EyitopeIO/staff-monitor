//defined in main.c
#include <functionDef2.h>

#ifndef REG52
#include <REG52.h>
#endif

//defined in main.c
extern unsigned char feb;
extern bit fl ,RXDcmpt;
extern unsigned char SERIALcase;
extern unsigned char receivedData[50], count;

void sendCommand(unsigned char *data_source, unsigned char end_chr) {
	//build a circuit that selects whether MCU signal goes to RN4871 or GSM
	while(*data_source!=0x00) { //null character is 0x00;
		SBUF = *data_source;      
		while(!TI);
		TI=0;
		data_source++;
	}
	SBUF = end_chr;     //
	while(!TI); 
	TI = 0;
}

void serialPortInit() {
		SCON = 0x50;         
		TMOD = 0x20;          
		TH1 =  0xFD;  // to set the baud rate to 9600
		TR1 = 1;     //to start the timer 
		ES = 1;     //enable serial port interrupt    
		EA = 1;   // enable global interrupt 
}
			
void BYTEwrite( unsigned char VAL, unsigned char buffLEN, unsigned char *buffADDR) {
	while(buffLEN)
	{
		*buffADDR++ = VAL;      														//write the value into address.
		 buffLEN--;           													   //decrement the lenght
	}
	return;
}

int modemInit() {
	for(;;) {
		switch(feb)
		{
			case 1:			
				sendAT("Star", 0x0D);
				while(!RXDcmpt);                      // Wait till complete info has been receive
				if(confirmData(receivedData,pb, 8))          //CHECK FOR  +PBREADY                                                         
				{
					sendAT("+PBREADY RCX", 0x0D); // to send  pbready rcx once the result true 
					BYTEwrite(0x0,count,receivedData);    //clear buffer
					RXDcmpt =count = 0;
					feb=2;  // if true move to case 2
				}		
				else   // if false
				{
					BYTEwrite(0x00,count,receivedData);  // clear buffer   
					RXDcmpt =count =0;			
					feb=1; 										// repeat case 1
				}
				break;
			/*********************************************************/	
			case 2:
				sendAT("ATE0",0x0D);  // to switch off echo
				//delay(200);
				while(!RXDcmpt);
				if(confirmData(receivedData,"OK", 2)) // check for OK
				{
					BYTEwrite(0x00,count,receivedData);    //clear buffer
					RXDcmpt =count = 0;
					feb=3; 		//if true go to case  3
				}		
				else 
				{
					BYTEwrite(0x00,count,receivedData);  // clear buffer   
					RXDcmpt =count =0;			
					feb=2;  								//repeat case 2
				}
				break;
			/*********************************************************/	
			case 3:
				sendAT("AT+CREG?",0x0D); //check for network
				//delay(50)	
				while(!RXDcmpt);
				if(confirmData(receivedData,network,10)) // confirm if the reply is "+CREG 0,1"
				{
					sendAT("network set",0x0D);  // send network set once result is true
					BYTEwrite(0x00,count,receivedData);  // clear buffer   
					RXDcmpt=count =0;						
					feb= 4;				// if true goto case 4
				}
				else												//if false
				{
					BYTEwrite(0x0,count,receivedData);  // clear buffer   
					RXDcmpt =count =0;		
					feb=3;                    // repeat case 3
				}	
				break;
			/*********************************************************/	
			case 4:
				sendAT("AT+CSCS=\"GSM\"",0x0D); // set to gsm mode  
			  sendAT("AT+CMGF=1",0x0D);  // SET GSM TO TEXT MODE 
				while(!RXDcmpt);
				if(confirmData(receivedData,"OK",2)) // check for OK
				{
					BYTEwrite(0x0,count,receivedData);  // clear buffer   
					RXDcmpt =count =0;			
					sendAT("AT+CMGS =\"08174705046\"",0x0D);  // PHONE NUMBER TO SEND SMS TO
					while(!RXDcmpt);
					if(confirmData(receivedData,">",1))
					{
						BYTEwrite(0x0,count,receivedData);  // clear buffer   
						RXDcmpt =count =0;			
						sendAT("SOME ONE IS AT THE DOOR",0x0D); // THE MTEXT TO BE SEND 
						sendAT(0X1A,0X0D); 		// THE HEXADECIMAL VALUE FOR CTRLZ 
						feb=1;
					}
					else 
					{
						BYTEwrite(0x0,count,receivedData);  // clear buffer   
						RXDcmpt =count =0;	
						feb=4;
					}
				}		
				else 
				{
					BYTEwrite(0x0,count,receivedData);  // clear buffer   
					RXDcmpt =count =0;			
					feb=4;  								//repeat case 4
				}
				break;
		}
	}
}
bit confirmData (char *var_unsure, char *var_sure,int data_size) {
	while(data_size--) {
		if(*var_unsure++ != *var_sure++) {
			return 0;
		}
	}
	return 1;
}
void serial() interrupt 4 { //you need separa
	unsigned char rcv;
	rcv = SBUF;
	if(RI)
	{
		switch(SERIALcase)
		{
			case 1:     //check for the first byte of opening flag.
				if(rcv == 0x0D)
					{
					//clear the buffer if the value is 0x0D
					SERIALcase=2;
					}
				break;
			case 2:  //check for the second byte of opening flag
					if(rcv == 0x0A)
					{
					//clear the buffer if the value is 0x0A
					// buffer must receive 0x0D n 0x0A first 	
					SERIALcase = 3;  
					}
					break;
			case 3: 
				receivedData[count++] = rcv;
				if(rcv == 0x0D)
					{
					SERIALcase  = 4;        							//check for 0x0A.
					}
				break;
			case 4:
				if(rcv== 0x0A)
				{
					//If 0x0A is recieved after 0x0D, then this is the end of recieve frame.
					RXDcmpt = 1;    											//set recieve complete flag.
					SERIALcase  = 1;											//its not 0x0A, so go back to keep recieveing till you detect another 0x0D.
					count-=1;
				}
				else
				{
					receivedData[count++] = rcv;   					//save the value recieved in rcv buffer.
					SERIALcase = 3;
				}
				break;
		}
	}
	RI = 0;
}
void sendAT(unsigned char *buf, unsigned char val) { //modemInit needs this for now
	while(*buf!=0x00)
	{
		SBUF = *buf;      
		while(!TI);
		TI=0;
		buf++;
	}
	SBUF = val;     // PUT VAL IN SBUF
	while(!TI); 
	TI = 0;
}

