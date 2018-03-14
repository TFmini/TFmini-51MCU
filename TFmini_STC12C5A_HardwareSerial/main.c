#include <stdio.h>
#include "stc12c5a60s2.h"	//the file come from stc-sip V6.86

#define FOSC 11059200L	//System frequency

typedef struct {
	int distance;
	int strength;
	char receiveComplete;
}TFmini;

TFmini tfmini = {0, 0, 0}; 

/******************************************************************************
 TFmini 9 bytes output: 
 [0x59, 0x59, distanceL, distanceH, strengthL, strengthH, Mode, 0x00, checksum]
 *****************************************************************************/
void getTFminiData(TFmini *tfmini) {
	static unsigned char i = 0 ;
	unsigned char j = 0;
	unsigned int checksum = 0;
	static unsigned char rx[9];
	
	if(S2CON & 0x01) {	//uart2 receive interrupt flag 
		S2CON &= ~(0x01);	//clear uart2 receive interrupt flag
		rx[i] = S2BUF;	//uart2 buf
		if(rx[0] != 0x59) {
			i = 0;
		} else if(i == 1 && rx[1] != 0x59) {
			i = 0;
		} else if(i == 8) {
			tfmini->receiveComplete = 1;
			//printf("\r\n");
			for(j = 0; j < 8; j++) {
				checksum += rx[j];
			} 
			if(rx[8] == (checksum % 256)) {
				tfmini->distance = rx[2] + rx[3] * 256;
				tfmini->strength = rx[4] + rx[5] * 256;
				tfmini->receiveComplete = 1;
			}
			i = 0;
		} else {
			i++;
		}
	}
}

/******************************************************************************
 Timer2 as baudtare generator.
 When FOSC = 11059200L, baudrate <= 345600(11059200/32) 
 *****************************************************************************/
void Uart_Init(unsigned long baudrate1, unsigned long baudrate2) {
	
	//Uart1 Init. Uart1 can select Timer1 or BRT as baudrate generator
	SCON = 0x50;            //8-bit variable UART
	//AUXR |= 0x01;			//selectBRT as baudrate generator
	AUXR |= 0x40;			//Set Timer1 1T Mode
	TMOD |= 0x20;   		//Set Timer1 as 8-bit auto reload mode
	TH1 = TL1 = -(FOSC/32/baudrate1); //Set auto-reload vaule
	TR1 = 1;                //Timer1 start run
	//ES = 1;
	
	//Uart2 Init. Uart2 can only select BRT as baudrate generator
	S2CON = 0x50;           //8-bit variable UART
	BRT = -(FOSC/32/baudrate2);  //Set auto-reload vaule of baudrate generator
	AUXR |= 0x14;            //Baudrate generator work in 1T mode
	IE2 = 0x01;             //Enable UART2 interrupt
		
	EA = 1;
	
	SBUF = '\n';	//It's needed to give SBUF a random character if printf is used.
}

void main() {
	 
	Uart_Init(115200, 115200);
	
	printf("Hello, TFmini!\r\n");
	
	while(1) {
		if(tfmini.receiveComplete == 1) {
			tfmini.receiveComplete = 0;
			printf("(%d, %d)\r\n", tfmini.distance, tfmini.strength);
		} 
	}
}

//void Uart() interrupt 4 using 1 {	
//}

void Uart2() interrupt 8 using 1 {
	getTFminiData(&tfmini);
}



