#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "string.h"
#include "Serial.h"
#include "MyRTC.h"
#include "LED.h"

uint16_t MyRTC_Time[]={2024,4,22,12,23,0};	//RTC时钟
uint16_t My_Time[]={0,0,0,0,0,0};			//串口时间数据处理
uint16_t My_ALTime[]={0,0,0};				//串口闹钟数据处理
uint8_t Alarm_Flag = 0;						//闹钟Flag
uint16_t tmp=0;								//暂存变量

void SetTime(void);
void VisitTime(void);
void SetAlarm(void);
void VisitAlarm(void); 

int main(void)
{
	/*模块初始化*/
	LED_Init();			//OLED初始化
	MyRTC_Init();	  	//RTC初始化
	Timer_Init();
	Serial_Init();
	MyRTC_SetTime();
	while(1)
	{
		MyRTC_ReadTime();										//RTC读取时间，最新的时间存储到MyRTC_Time数组中
		//串口接收数据
		if(Serial_RxFlag == 1)
		{
			/*1.设置时间命令*/
			if(strcmp(Serial_RxPacket, "AT+TIME=") == 0)		//设置时间命令
			{
				SetTime();
			}
			/*2.查询时间命令*/
			else if(strcmp(Serial_RxPacket, "AT+TIME?") == 0)	//接收时间询问
			{
				VisitTime();
			}
			/*3.设定闹钟*/
			else if(strcmp(Serial_RxPacket, "AT+ALARM=") == 0)	//设置闹钟命令
			{
				SetAlarm();
			}
			/*4.查询闹钟*/
			else if(strcmp(Serial_RxPacket, "AT+ALARM?") == 0)	//接收闹钟询问
			{
				VisitAlarm();
			}
			Serial_RxFlag = 0;
		}
	}
}

/*1.设置时间命令*/
void SetTime(void)
{
	int i=0,j=0;
	for(i=0;i<21;i++)
	{
		if('0'<=Serial_TimePacket[i] && Serial_TimePacket[i]<='9') //判断数字是否进入
		{
			tmp*=10;											   //处理数据
			tmp+=(Serial_TimePacket[i]-48);						  
		}
		else if(Serial_TimePacket[i]=='\0')						   //不处理最后一个字符
		{
			
		}
		else													   //将处理完的数据置入RTC数组中
		{
			MyRTC_Time[j]=tmp;
			MyRTC_SetTime();
			tmp=0;
			j++;
		}
	}
	VisitTime();
}

/*2.查询时间命令*/
void VisitTime(void)
{
	printf("TIME:%d-%d%d-%d%d %d%d:%d%d:%d%d.\r\n"
			,MyRTC_Time[0]						//年
			,MyRTC_Time[1]/10,MyRTC_Time[1]%10	//月
			,MyRTC_Time[2]/10,MyRTC_Time[2]%10	//日
			,MyRTC_Time[3]/10,MyRTC_Time[3]%10	//时
			,MyRTC_Time[4]/10,MyRTC_Time[4]%10	//分
			,MyRTC_Time[5]/10,MyRTC_Time[5]%10	//秒
			);
	printf("         OK\r\n");									//回传时间询问
}

/*3.设定闹钟*/
void SetAlarm(void)
{
	int i=0,j=0;
	for(i=0;i<9;i++)
	{
		if('0'<=Serial_TimePacket[i] && Serial_TimePacket[i]<='9') //判断数字是否进入
		{
			tmp*=10;											   //处理数据
			tmp+=(Serial_TimePacket[i]-48);						  
		}
		else if(Serial_TimePacket[i]=='\0')						   //不处理最后一个字符
		{
			
		}
		else													   //将处理完的数据置入RTC数组中
		{
			My_ALTime[j]=tmp;
			tmp=0;
			j++;
		}
	}
	VisitAlarm();
}

/*4.查询闹钟*/
void VisitAlarm(void)
{
	printf("ALTER:%d%d:%d%d:%d%d.\r\n"
			,My_ALTime[0]/10,My_ALTime[0]%10	//时
			,My_ALTime[1]/10,My_ALTime[1]%10	//分
			,My_ALTime[2]/10,My_ALTime[2]%10	//秒
			);
	printf("         OK\r\n");									//回传时间询问
}

/*5.闹钟响铃*/
void Ringdown(void)
{
	if(My_ALTime[0]==MyRTC_Time[3] && My_ALTime[1]==MyRTC_Time[4] && My_ALTime[2]==MyRTC_Time[5])
	{
		Alarm_Flag=1;							//到达定时时间
	}
}

//定时中断使用模板
void TIM2_IRQHandler(void)
{
	static uint32_t Count1,Count2;
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)	//判断是否达到中断条件
	{
		if(Alarm_Flag==0)							
		{
			Count1++;
			if(Count1>=2)							//每隔20ms查看是否到达定时时间
			{
				Ringdown();							//闹钟标志位置1，闹钟响铃
				Count1=0;
			}
		}
		if(Alarm_Flag==1)
		{
			Count2++;
			LED1_NO();
			if(Count2>=3000)						//响铃30s
			{
				Count2=0;
				LED1_OFF();
				Alarm_Flag=0;						//清除闹钟标志位
			}
		}
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	//清除定时中断标志位
	}
}
