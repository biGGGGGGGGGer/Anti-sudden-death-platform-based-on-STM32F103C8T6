#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"
#include "oled.h"
#include "myiic.h"
#include "max30102.h"
#include "algorithm.h"

// 心率
extern int flag;
extern int time_flag;
extern int time;
extern unsigned int rec_data[4];
uint8_t RX_Data;
u8 HR;
u8 SPO2;
float BeforeProcessData[50],Temp,AfterProcessData;
void ShowMax(void);

//GPS有关声明
void errorLog(int num);
void parseGpsBuffer(void);
void ShowGpsBuffer(void);
void ShowMax(void);
float longitude = 0;
float latitude = 0;

// SIM800C 用UART2
void SendMessage(void);

//硬件初始化
void Hardware_Init(void)
{	
	USART2_Config();
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	delay_init();			//延时函数初始化
	max30102_init();
	MAX30102_data_set();	
	OLED_Init();
}

int main(void)
{	
	Hardware_Init();
	OLED_ColorTurn(0);//0正常显示，1 反色显示
	OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
	clrStruct();
	delay_ms(3000);
	
	while(1)
	{
		ShowMax();
		parseGpsBuffer();
		ShowGpsBuffer();
		delay_ms(1000);
		if(AfterProcessData > 130 || AfterProcessData < 55)
		{
			for(int SendCmd = 0;SendCmd < 10;SendCmd++)
			{
				SendMessage();
				delay_ms(1000);
			}
		}
	}
}

void SendMessage(void)
{
	char ToSendGPS[50];
	delay_ms(2000);
	Usart_SendString(DEBUG_USART2,"AT\r\n");
	delay_ms(1000);
	Usart_SendString(DEBUG_USART2,"AT+CMGF=1\r\n");
	delay_ms(1000);
	Usart_SendString(DEBUG_USART2,"AT+CSMP=17,167,1,8\r\n");
	delay_ms(1000);
	Usart_SendString(DEBUG_USART2,"AT+CSCS=\"GSM\"\r\n");
	delay_ms(1000);
	Usart_SendString(DEBUG_USART2,"AT+CMGF=1\r\n");
	delay_ms(10000);
	Usart_SendString(DEBUG_USART2,"AT+CMGS=\"13140506989\"\r\n");
	delay_ms(1000);	
	sprintf(ToSendGPS,"N:%f   E:%f",longitude,latitude);
	Usart_SendString(DEBUG_USART2,ToSendGPS);
	delay_ms(1000);
	Usart_SendByte(DEBUG_USART2,0x1A);
	delay_ms(2000);
}

void errorLog(int num)
{
	while (1)
	{
	  printf("ERROR%d\r\n",num);
	}
}

void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}
					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;
				}
				else
				{
					errorLog(2);	//解析错误
				}
			}
		}
	}
}

void ShowGpsBuffer()
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		if(Save_Data.isUsefull)
		{
			longitude = atof(Save_Data.longitude);
			latitude = atof(Save_Data.latitude);

			longitude = longitude/100;
			latitude = latitude/100;
			
			Save_Data.isUsefull = false;		
		}
		else
		{
//			OLED_ShowString(2,32,"waiting for GPS DATA!",20,1);
		}
	}
}

void ShowMax(void)
{
	for(int i = 0;i<50;i++)
	{
		MAX30102_get(&HR,&SPO2);
		BeforeProcessData[i] = HR;
		delay_ms(10);
	}
	for(int j = 0;j<50;j++)
	{
		Temp += BeforeProcessData[j];
	}
	AfterProcessData = Temp/50;
	OLED_Refresh();
	OLED_ShowString(2,0,"HeartRate:",16,1);      OLED_ShowNum(30,0,AfterProcessData,3,16,1);	   OLED_ShowString(65,0,"BPM",16,1);      //显示心率
}

