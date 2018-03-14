#include <stdio.h>
#include <reg52.h>

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
	static unsigned int rx[9];
	if(RI) {
		RI = 0;
		rx[i] = SBUF;
		if(rx[0] != 0x59) {
			i = 0;
		} else if(i == 1 && rx[1] != 0x59) {
			i = 0;
		} else if(i == 8) {
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
void Uart_Init(unsigned long baudrate) {
    SCON = 0x50;	//8-bit variable UART
	TL2 = RCAP2L  = (65536 - (FOSC/32/baudrate));   //Set auto-reload vaule
    TH2 = RCAP2H  = (65536 - (FOSC/32/baudrate))>>8;
	T2CON = 0x34;	//Timer2 as baudtare generator, Timer2 start run
	ES = 1;
	EA = 1;
	
	SBUF = '\n';	//It's needed to give SBUF a random character if printf is used.
}

void main() {
	 
	Uart_Init(115200);
	printf("Hello, TFmini!\r\n");
	
	while(1) {
		if(tfmini.receiveComplete == 1) {
			tfmini.receiveComplete = 0;
			printf("(%d, %d)\r\n", tfmini.distance, tfmini.strength);
		} 
	}
}

void Uart() interrupt 4 using 1 {	
	getTFminiData(&tfmini);
}

