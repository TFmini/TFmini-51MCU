#include <stdio.h>	//used printf
#include "stc15f2k60s2.h"	//from STC-ISP V6.86, Used for all STC15 MCUs.  

// #define FOSC 11059200L	//System frequency
#define FOSC 33177600L	//System frequency

#define RXB P54
#define TXB P55

typedef bit BOOL;
typedef unsigned char BYTE;
typedef unsigned int WORD;
BYTE TBUF,RBUF;
BYTE TDAT,RDAT;
BYTE TCNT,RCNT;
BYTE TBIT,RBIT;
BOOL TING,RING;
BOOL TEND,REND;

BYTE t, r;
BYTE buf[16];

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
	if(REND) {
		REND = 0;
		rx[i] = RBUF;
		//printf("%d ", rx[i]);
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
 Timer 0 and 2 can be used, STC15W204S don't have Timer 1.
 *****************************************************************************/
void softwareUart_Init(unsigned long baudrate) {
	TING = 0;
    RING = 0;
    TEND = 1;
    REND = 0;
    TCNT = 0;
    RCNT = 0;
	
	TMOD = 0x00;	//timer0 in 16-bit auto reload mode
    AUXR |= 0x80;	//timer0 working at 1T mode
	//BAUD = 65536 - FOSC/3/BAUDRATE/M (1T:M=1; 12T:M=12), 3 times sampling rate
	//NOTE: (FOSC/3/BAUDRATE) must be greater then 98, (RECOMMEND GREATER THAN 110)
    TL0 = (65536 - (FOSC/3/baudrate));
    TH0 = (65536 - (FOSC/3/baudrate))>>8;	//initial timer0 and set reload value
    TR0 = 1;	//tiemr0 start running
    ET0 = 1;	//enable timer0 interrupt
    PT0 = 1;	//improve timer0 interrupt priority
    EA = 1;		//open global interrupt switch
}

void Uart_Init(unsigned long baudrate) {
    SCON = 0x50;	//8-bit variable UART
	AUXR |= 0x14;	//set Timer2 1T Mode, and start Timer2
	AUXR |= 0x01;	//select Timer 2 as Uart 1 baud generator
	T2L = (65536 - (FOSC/4/baudrate));   //Set auto-reload vaule
    T2H = (65536 - (FOSC/4/baudrate))>>8;
	ES = 1;
	EA = 1;
	SBUF = '\n';	//It's needed to give SBUF a random character if printf is used.
}
	
void main() {
	Uart_Init(115200);
	softwareUart_Init(115200);
	
	printf("Hello, TFmini!\r\n");
	
	while(1) {
		// if (REND)
        // {
        //     REND = 0;
        //     buf[r++ & 0x0f] = RBUF;
		// 	printf("%c ", RBUF);
        // }
        // if (TEND)
        // {
        //     if (t != r)
        //     {
        //         TEND = 0;
        //         TBUF = buf[t++ & 0x0f];
        //         TING = 1;
        //     }
        // }

        getTFminiData(&tfmini);
		if(tfmini.receiveComplete == 1) {
			tfmini.receiveComplete = 0;
			printf("(%d, %d)\r\n", tfmini.distance, tfmini.strength);
		} 
	}
}

void Uart() interrupt 4 using 1 {	
}

//Timer interrupt routine for UART
void tm0() interrupt 1 using 1
{
    if (RING)
    {
        if (--RCNT == 0)
        {
            RCNT = 3;                   //reset send baudrate counter
            if (--RBIT == 0)
            {
                RBUF = RDAT;            //save the data to RBUF
                RING = 0;               //stop receive
                REND = 1;               //set receive completed flag
            }
            else
            {
                RDAT >>= 1;
                if (RXB) RDAT |= 0x80;  //shift RX data to RX buffer
            }
        }
    }
    else if (!RXB)
    {
        RING = 1;                       //set start receive flag
        RCNT = 4;                       //initial receive baudrate counter
        RBIT = 9;                       //initial receive bit number (8 data bits + 1 stop bit)
    }

    if (--TCNT == 0)
    {
        TCNT = 3;                       //reset send baudrate counter
        if (TING)                       //judge whether sending
        {
            if (TBIT == 0)
            {
                TXB = 0;                //send start bit
                TDAT = TBUF;            //load data from TBUF to TDAT
                TBIT = 9;               //initial send bit number (8 data bits + 1 stop bit)
            }
            else
            {
                TDAT >>= 1;             //shift data to CY
                if (--TBIT == 0)
                {
                    TXB = 1;
                    TING = 0;           //stop send
                    TEND = 1;           //set send completed flag
                }
                else
                {
                    TXB = CY;           //write CY to TX port
                }
            }
        }
    }
}

