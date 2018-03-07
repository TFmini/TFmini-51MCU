#include <stdio.h>	//used printf
#include "stc15f2k60s2.h"	//from STC-ISP V6.86, Used for all STC15 MCUs.  

#define FOSC 11059200L	//System frequency

typedef struct {
	int distance;
	int strength;
	char receiveComplete;
}TFmini;

/******************************************************************************
 TFmini 9 bytes output: 
 [0x59, 0x59, distanceL, distanceH, strengthL, strengthH, Mode, 0x00, checksum]
 *****************************************************************************/
void getTFminiData(TFmini *tfmini) {
	static unsigned char i = 0 ;
	unsigned char j = 0;
	unsigned int checksum = 0;
	static unsigned int rx[9];
	if(RI) {
		RI = 0;
		rx[i] = SBUF;
		if(rx[0] != 0x59) {
			i = 0;
		} else if(i == 1 && rx[1] != 0x59) {
			i = 0;
		} else if(i == 8) {
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
 Timer 0 and 2 can be used, STC15W204S don't have Timer 1.
 *****************************************************************************/
void Uart_Init(unsigned long baudrate) {
    SCON = 0x50;	//8-bit variable UART
	AUXR = 0x14;	//set Timer2 1T Mode, and start Timer2
	AUXR |= 0x01;	//select Timer 2 as Uart 1 baud generator
	T2L = (65536 - (FOSC/4/baudrate));   //Set auto-reload vaule
    T2H = (65536 - (FOSC/4/baudrate))>>8;
	ES = 1;
	EA = 1;
	SBUF = '\n';	//It's needed to give SBUF a random character if printf is used.
}
	
void main() {
	TFmini tfmini = {0, 0, 0};  
	
	Uart_Init(115200);
	printf("Hello, TFmini!\r\n");
	
	while(1) {
		getTFminiData(&tfmini);
		if(tfmini.receiveComplete == 1) {
			tfmini.receiveComplete = 0;
			printf("(%d, %d)\r\n", tfmini.distance, tfmini.strength);
		} 
	}
}