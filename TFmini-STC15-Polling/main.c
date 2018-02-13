/******************************************************************************
Author: weifengdq@163.com
Date: 201802
Link: TFmini TX(Green) -- STC15 RXD(P30)
                          STC15 TXD(P31) -- PC USBtoUART RX
Note: When you download program, please break P30 with TFmini, 
      then Link P30 with PC USBtoUART TX
******************************************************************************/

#include "stc15f2k60s2.h"
#include <stdio.h>

#define FOSC 11059200L	//System frequency

void Uart_Init(unsigned long baudrate) {
    SCON = 0x50;	//8-bit variable UART
	AUXR = 0x40;	//set Timer1 1T Mode
    TMOD = 0x00;	//Set Timer1 as 16-bit auto reload mode
	TL1 = (65536 - (FOSC/4/baudrate));   //Set auto-reload vaule
    TH1 = (65536 - (FOSC/4/baudrate))>>8;
    TR1 = 1;	//Timer1 start run
	SBUF = '\n';	//It's needed to give SBUF a random character if printf is used.
}

/******************************************************************************
 TFmini 9 bytes output: 
 [0x59, 0x59, distanceL, distanceH, strengthL, strengthH, Mode, 0x00, checksum]
 *****************************************************************************/
unsigned int TFmini_GetValue() {
	static unsigned char i = 0;
	unsigned char j = 0;
	unsigned int checksum = 0; 
	static unsigned char rx[9];
	unsigned int distance = 0;
	if(RI) {	//serialport receive a character
		RI = 0;
		rx[i] = SBUF;
		if(rx[0] != 0x59) {
			i = 0;
		} else if(i == 1 && rx[1]!= 0x59) {
			i = 0;
		} else if(i == 8) {
			for(j = 0; j < 8; j++) {
				checksum += rx[j];
			}
			if(rx[8] == (checksum % 256)) {
				distance = rx[2] + rx[3] * 256;
			}
			i = 0;
		} else {
			i++;
		}	
	}
	return distance;
}

void main() {
	unsigned int distance = 0;
	
	Uart_Init(115200);	//baudrate: 115200; stop: 1; parity: 0.

	printf("Hello, TFmini!\r\n");
	
    while(1) {
		distance = TFmini_GetValue();
		while(!distance) {
			distance = TFmini_GetValue();
			if(distance) {
				printf("%d\r\n", distance);
			}
		}
	}
}
