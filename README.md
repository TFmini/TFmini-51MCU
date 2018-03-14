# TFmini-51MCU
TFmini在51单片机上的一些例程.   

以STC的51单片机为例, STC单片机既有经典的STC89, 也有STC12, STC15, STC8等, 自带的串口数至少有1个, 如果连接TFmini的话, TFmini发送数据的时候只需要TFmini的TX连接单片机的RX即可, 单片机的TX可以空出来做其他事, 比如打印数据到电脑或者串口屏. 如果自带的硬件串口实在分不出来给TFmini, 还可以使用定时器模拟串口的方法, 找任意IO口做模拟串口连接TFmini即可.  

**注意下载程序的时候单片机型号不要选择错误, 可以使用STC-ISP的 检测MCU选项, 可以自动选择出单片机型号**.  

- [TFmini_STC89C52_HardwareSerial](#tfmini_stc89c52_hardwareserial)  
- [TFmini_STC12C5A_HardwareSerial](#tfmini_stc12c5a_hardwareserial)  
- [TFmini_STC15W204S_HardwareSerial](#tfmini_stc15w204s_hardwareserial)  
- [TFmini_STC15W204S_SoftwareSerial](#tfmini_stc15w204s_softwareserial)  
- [TFmini_STC15_Polling](#tfmini_stc15_polling)  




## TFmini_STC89C52_HardwareSerial
STC89C52有一个硬件串口, 也可以用定时器实现软件串口(参考 [TFmini_STC15W204S_SoftwareSerial](#tfmini_stc15w204s_softwareserial) 一节). 先下载程序:  

下载程序时的连接关系:  

STC89C52 | USB转串口(CH340, CP2102等)
---------|----------
 VCC | 5V 
 GND | GND 
 P30(RX) | TX
 P31(TX) | RX   

 STC-ISP的配置:  

 ![stcispstc89](/Assets/stcispstc89.png)  

 **下完程序后才可以连接TFmini**, 把TFmini的TX(绿线)连到STC89C52的RX(P30)即可, TFminid的RX(白线)可以悬空不接. 连接关系如下:  

STC89C52 | USB转串口 | TFmini
---------|----------|----------
 VCC | 5V | 5V(红)
 GND | GND | GND(黑)
 P30(RX) | | TX(绿)
 P31(TX) | RX | 

 如图(板载晶振为11.0592MHz):  

 ![stc89](/Assets/stc89.png)   

 打开串口调试助手, 选择正确的串口, 波特率115200, 数据左边为距离, 单位cm, 右边为信号强度:  

 ![sscom89](/Assets/sscom89.png)    

 TFmini的9字节输出解析可参考下面的代码:  

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
以STC12C5A60S2为例, 有两个串口. 这个例子中用串口2读取TFmini的数据, 然后通过串口1发送给PC. 串口2使用独立波特率发生器BRT, 串口1使用定时器1做波特率发生器.  硬件连接关系如下:  

STC12C5A60S2 | USB转串口 | TFmini
---------|----------|----------
 VCC | 5V | 5V(红)
 GND | GND | GND(黑)
 P12(RX2) | | TX(绿)
 P31(TX) | RX | 

如图所示:  

![stc12](/Assets/stc12.png)  

STC-ISP的配置:  

![stcispstc12](/Assets/stcispstc12.png)   

打开串口调试助手, 选择正确的串口, 波特率115200, 数据左边为距离, 单位cm, 右边为信号强度:  

![sscom89](/Assets/sscom89.png)    

程序中的 `ES = 1;` 这句开串口总中断被注释了, 也就是串口1不能中断? 不注释掉串口2不能中断, 原因暂时不明.   



## TFmini_STC15W204S_HardwareSerial
STC15W204S, SOP-8封装的有一个串口, 定时器有0和2, 没有1. 这里我们选择定时器2作为串口1的波特率发生器. 因为下载程序和TFmini都要用到同一个串口, 所以, 我们先下载程序, 下完后拔掉STC15W204S的RX连线, 接上TFmini的TX即可.   

下载程序时的连接关系:  

STC15W204S | USB转串口
---------|----------
 VCC | 5V 
 GND | GND 
 P30(RX) | TX
 P31(TX) | RX  

STC-ISP的配置:  

![stc15w](/Assets/stcispstc15w.png)  

注意时钟是11.0592 MHz, 下完程序后断掉 P30(RX)-TX(USB转串口) 的连线, 把TFmini的TX(绿, 白线悬空即可)接上, 连接关系如下:  

STC15W204S | USB转串口 | TFmini
---------|----------|----------
 VCC | 5V | 5V(红)
 GND | GND | GND(黑)
 P30(RX) | | TX(绿)
 P31(TX) | RX | 

 如图, 这里的另一个USB转串口只是用来给TFmini供电:    

 ![stc15w](/Assets/stc15w.png)  

 打开串口调试助手, 选择正确的串口, 波特率115200, 数据左边为距离, 单位cm, 右边为信号强度:  

 ![sscom](/Assets/sscom.png)  



## TFmini_STC15W204S_SoftwareSerial
如果没有多余的硬件串口给TFmini用, 可以使用软件串口, 软件串口还有一个好处, 理论上每一个通用IO都可以接一个TFmini.  

51单片机模拟串口的原理可以参考下面几篇文章:  
- [51单片机GPIO口模拟串口通信](http://blog.csdn.net/sdwuyulunbi/article/details/6656193)  
- [51单片机模拟串口的三种方法](http://www.21ic.com/jichuzhishi/mcu/series/2013-02-20/158749.html)  
- [51单片机IO口模拟UART串口通信心得](http://dzxxsyzx.zzia.edu.cn/s/100/t/757/e9/9d/info59805.htm)  


这里借用STC-ISP给出的示例程序, 使用定时器0模拟一个全双工串口(3倍波特率采样), 连接方式如下:  

STC15W204S | USB转串口 | TFmini
---------|----------|----------
 VCC | 5V | 5V(红)
 GND | GND | GND(黑)
 P54(模拟RX) | | TX(绿)
 P31(TX) | RX |   

 通过模拟的RX(P54)读取TFmini的数据, 然后通过硬件串口P31(TX)把数据发送到电脑上.  具体代码参考TFmini_STC15W204S_SoftwareSerial工程. **注意下载的时候, 把频率选择为 33.1776MHz**. 



## TFmini_STC15_Polling   

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

