#include <REGX52.H>
#include <Matrix_Keyboard.h>
#include "DigitalLocks.h"
#include <LCD1602.h>
#include <Uart.h>
#include "FigerLocks.h"
#include "Delay.h"


#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

sbit PWM=P2^4;           //舵机PWM控制变量

uchar UART_Receives[50];
uchar numOfUART_Receives = 0;
uchar verify = 0;
uint password_main = 4612;
// uchar flag = 1;
uint counter, PWM_counter = 1;

void UART_Routine() interrupt 4
{
	if(RI)
	{
		// while(RI == 0);	 //接收完为1 
		
		UART_Receives[numOfUART_Receives ++] = SBUF;
		
		RI = 0;
	}
}

void Timer0_Init();


void main()
{
	uchar ttt;
	
	Uart1_Init();
	Timer0_Init();
	LCD_Init();
	TR0 = 0;
	while(1)
	{
		
			P1_0 = 0;
		
			ttt = key_matrix_ranks_scan();
			Figer_Init();
			verify = Finger();
			
			if(ttt == 11)
			{
				verify = DigitalLock_InputPassword(password_main);    
			}
			if(ttt == 12)
			{
				if(DigitalLock_InputPassword(password_main) == 1)
				{
					password_main = DigitalLock_SetPassword();
					
				}	
				
			}
		
		if(verify == 1)
		{
			TR0 = 1;
			PWM_counter = 3;
			Delay(1000);
			LCD_ShowString(2, 1, "LOCK OPEN");
			Delay(1500);
			PWM_counter = 1;
			Delay(1000);
			TR0 = 0;
			verify = 0;
		}
		
	}
}





void Timer0_Init()
{
//	TMOD=0x01;
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x33;		//设置定时初值
	TH0 = 0xFE;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
	PT0=0;
}


void Timer0() interrupt 1
{
	TL0 = 0x33;		//设置定时初值
	TH0 = 0xFE;
	 
	counter ++;
	counter %= 40;
	if(counter < PWM_counter)
	{
		PWM = 1;
	}
	else
	{
		PWM = 0;
	}
	
	
}