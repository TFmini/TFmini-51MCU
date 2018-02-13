# TFmini-51MCU
TFmini在51单片机上的一些例程.  

## TFmini-STC15-Polling 

![TFmini-STC15](/Assets/TFmini-STC15.png)  

STC15有两个或者更多的串口, 但这里我们只使用一个串口, 连接方式为:   
> TFmini TX(绿线) -- STC15 RXD(P30)  
> STC15 TXD(P31) -- PC USBtoUART RX

注意下载程序的时候断开TFmini与STC15 P30的连线, 然后把P30连到USB转串口的RX上. 下完程序后再逆操作.   

TFmini 9字节输出数据的解析如下:  

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

这段程序用到轮询或者串口中断里面都是可以的. 

