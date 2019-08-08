# TFmini-51MCU
TFmini some routines on the 51 MCU.

Taking STC's 51 MCU as an example, STC MCU has both classic STC89 and STC12, STC15, STC8, etc. There is at least one serial port. If TFmini is connected, TFmini only needs TX of TFmini to connect RX of SCM when sending data.The TX of the MCU can be freed to do other things, such as printing data to a computer or serial screen.If the built-in hardware serial port can not be divided into TFmini, you can also use the timer to simulate the serial port, find any IO port to do simulation serial port to connect TFmini.

**Note that when downloading the program, you must select the correct MCU model.You can use the MCU detection option of STC-ISP to select the MCU model automatically.**

- [TFmini_STC89C52_HardwareSerial](#tfmini_stc89c52_hardwareserial)  
- [TFmini_STC12C5A_HardwareSerial](#tfmini_stc12c5a_hardwareserial)  
- [TFmini_STC15W204S_HardwareSerial](#tfmini_stc15w204s_hardwareserial)  
- [TFmini_STC15W204S_SoftwareSerial](#tfmini_stc15w204s_softwareserial)  
- [TFmini_STC15_Polling](#tfmini_stc15_polling) 

## TFmini_STC89C52_HardwareSerial
STC89C52 has a hardware serial port and can also realize software serial port with timer.(reference: [TFmini_STC15W204S_SoftwareSerial](#tfmini_stc15w204s_softwareserial) ). Download the program first:  

Connection relationship when downloading the program:

STC89C52 | USB serial port(CH340, CP2102等)
---------|----------
 VCC | 5V 
 GND | GND 
 P30(RX) | TX
 P31(TX) | RX   

 Configuration of STC-ISP:  

 ![stcispstc89](/Assets/stcispstc89.png)  
 
  **You can connect to TFmini after the program is finished**, connect the TFmini's TX (green line) to the STX89C52's RX (P30), and the TFminid's RX (white line) can be left unconnected. The connection relationship is as follows:

STC89C52 | USB serial port | TFmini
---------|----------|----------
 VCC | 5V | 5V(red)
 GND | GND | GND(black)
 P30(RX) | | TX(green)
 P31(TX) | RX | 
 
 As shown in the figure (onboard crystal oscillator is 11.0592MHz): 

 ![stc89](/Assets/stc89.png)   

 Open the serial port debugging assistant and select the correct serial port, The baud rate is 115200. The left side of the data is the distance, the unit is cm, and the right side is the signal strength:

 ![sscom89](/Assets/sscom89.png)    

 TFmini's 9-byte output parsing can refer to the following code:
 
 ```C
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
	static unsigned int rx[9];	//change int to char if not work
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
 ```
 
 
 ## TFmini_STC12C5A_HardwareSerial
Take STC12C5A60S2 as an example, there are two serial ports. In this example, the serial port 2 is used to read the TFmini data, and then sent to the PC through the serial port 1. Serial port 2 uses the independent baud rate generator BRT, and serial port 1 uses timer 1 as the baud rate generator. The hardware connection relationship is as follows:

STC12C5A60S2 | USB serial port | TFmini
---------|----------|----------
 VCC | 5V | 5V(red)
 GND | GND | GND(black)
 P12(RX2) | | TX(green)
 P31(TX) | RX | 

As the picture shows: 

![stc12](/Assets/stc12.png)  

 Configuration of STC-ISP: 

![stcispstc12](/Assets/stcispstc12.png)   

Open the serial port debugging assistant and select the correct serial port, The baud rate is 115200. The left side of the data is the distance, the unit is cm, and the right side is the signal strength:  

![sscom89](/Assets/sscom89.png)    



## TFmini_STC15W204S_HardwareSerial
STC15W204S, The SOP-8 package has a serial port, the timer has 0 and 2, no 1.Here we choose Timer 2 as the baud rate generator of serial port 1. Because the download program and TFmini use the same serial port, so we download the program first, then unplug the RX connection of STC15W204S, connect TFmini The TX .

Connection relationship when downloading the program: 

STC15W204S | USB serial port
---------|----------
 VCC | 5V 
 GND | GND 
 P30(RX) | TX
 P31(TX) | RX  

Configuration of STC-ISP:  

![stc15w](/Assets/stcispstc15w.png) 

Note that the clock is 11.0592mhz. After finishing the program, the connection of P30(RX) -tx (USB serial port) is broken, and the TX(green and white wires can be suspended) of TFmini is connected. The connection relationship is as follows:

STC15W204S | USB serial port | TFmini
---------|----------|----------
 VCC | 5V | 5V(red)
 GND | GND | GND(black)
 P30(RX) | | TX(green)
 P31(TX) | RX | 

 As shown in the figure, another USB to serial port here is only used to power the TFmini:  

 ![stc15w](/Assets/stc15w.png)  

 Open the serial port debugging assistant and select the correct serial port, The baud rate is 115200. The left side of the data is the distance, the unit is cm, and the right side is the signal strength:  

 ![sscom](/Assets/sscom.png)  
 
 ## TFmini_STC15W204S_SoftwareSerial
If there is no redundant hardware serial port for TFmini, you can use the software serial port, the software serial port has another advantage. In theory, every general IO can be connected to a TFmini.

The principle of 51 single-chip analog serial port can refer to the following articles:

- [51单片机GPIO口模拟串口通信](http://blog.csdn.net/sdwuyulunbi/article/details/6656193)  
- [51单片机模拟串口的三种方法](http://www.21ic.com/jichuzhishi/mcu/series/2013-02-20/158749.html)  
- [51单片机IO口模拟UART串口通信心得](http://dzxxsyzx.zzia.edu.cn/s/100/t/757/e9/9d/info59805.htm)  


Here we borrow the sample program given by STC-ISP and use Timer 0 to simulate a full-duplex serial port (3 times baud rate sampling). The connection method is as follows:

STC15W204S | USB serial port | TFmini
---------|----------|----------
 VCC | 5V | 5V(red)
 GND | GND | GND(black)
 P54(simulationRX) | | TX(green)
 P31(TX) | RX |   

 The TFmini data is read by the analog RX (P54), and then the data is sent to the computer through the hardware serial port P31 (TX). Refer to the TFmini_STC15W204S_SoftwareSerial project for specific code. **When downloading, select the frequency as 33.1776MHz**.
 
 ## TFmini_STC15_Polling   

![TFmini-STC15](/Assets/TFmini-STC15.png)  

STC15 has two or more serial ports, but here we only use one serial port, the connection method is:  
> TFmini TX(the green cable) -- STC15 RXD(P30)  
> STC15 TXD(P31) -- PC USBtoUART RX

Note that when downloading the program, disconnect the TFmini from the STC15 P30, and then connect the P30 to the RX of the USB to serial port. After the program is finished, reverse the operation.

The TFmini 9-byte output data is parsed as follows: 

```C
/******************************************************************************
 TFmini 9 bytes output: 
 [0x59, 0x59, distanceL, distanceH, strengthL, strengthH, Mode, 0x00, checksum]
 *****************************************************************************/
unsigned int TFmini_GetValue() {
	static unsigned char i = 0;
	unsigned char j = 0;
	unsigned int checksum = 0; 
	static unsigned int rx[9];
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
```  

This program can be used in polling or serial interrupts.

