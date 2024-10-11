#include "stm32f10x.h"                  // Device header
#include "time.h"   

extern uint16_t MyRTC_Time[]; 

void MyRTC_SetTime(void);

void MyRTC_Init(void)
{
	//St1:时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP,ENABLE);
	
	//St2:备份寄存器访问使能
	PWR_BackupAccessCmd(ENABLE);
	
	//St3:准备阶段
	RCC_LSEConfig(RCC_LSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)!=SET);
	
	//St4:选择RTCCLK时钟源
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);
	
	//St5:等待同步
	RTC_WaitForSynchro();						//等待同步函数
	RTC_WaitForLastTask();						//等待上一次操作完成
	
//	//St6:设置初始时间
//	MyRTC_SetTime();							//初始化时设置时间
}

//设置时间
void MyRTC_SetTime(void)
{
	time_t time_cnt;
	struct tm time_data;
	
	time_data.tm_year = MyRTC_Time[0]-1900;		//年
	time_data.tm_mon  = MyRTC_Time[1]-1;		//月
	time_data.tm_mday = MyRTC_Time[2];			//日
	time_data.tm_hour = MyRTC_Time[3];			//时
	time_data.tm_min  = MyRTC_Time[4];			//分
	time_data.tm_sec  = MyRTC_Time[5];			//秒
	
	time_cnt = mktime(&time_data)-8*60*60;		//设置东八区时间
	
	RTC_SetCounter(time_cnt);					//读取时间
	RTC_WaitForLastTask();						//等待上一次操作完成
}

//读取时间
void MyRTC_ReadTime(void)
{
	time_t time_cnt;
	struct tm time_data;						//设置时间变量结构体
	
	time_cnt = RTC_GetCounter()+8*60*60;		//调整时区为东八区
	
	time_data = *localtime(&time_cnt);
	
	MyRTC_Time[0] = time_data.tm_year + 1900;
	MyRTC_Time[1] = time_data.tm_mon +1;
	MyRTC_Time[2] = time_data.tm_mday;
	MyRTC_Time[3] = time_data.tm_hour;
	MyRTC_Time[4] = time_data.tm_min;
	MyRTC_Time[5] = time_data.tm_sec;
}
