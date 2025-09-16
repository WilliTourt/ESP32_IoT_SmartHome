#include <REGX52.H>
#include "LCD1602.h"
#include "Uart.h"
#include "AS608.h"
#include "Delay.h"

#define CharBuffer1 0x01
#define CharBuffer2 0x02
#define PageNum 12
extern uchar numOfUART_Receives;
extern uchar UART_Receives[50];
extern uint PWM_counter;

void Figer_Init()
{
	LCD_Init();
	LCD_ShowString(1,1,"Figer:");
	LCD_ShowNum(1,13,216,3);
}

unsigned char Finger()
{
    uchar i, temp1= 0, temp2 = 0;
    static uchar ensure1 = 0xff, ensure2 = 0xff, ensure3 = 0xff;
    uchar FigerID;
   
    ensure1 = AS608_GetImage();
    Delay(300);
    
    if(ensure1 == 0x00)
    {
        // for(i = 0; i < 10; i ++)
        // {
            Delay(300);
            P2_0 = 0;
            ensure2 = AS608_GenChar(CharBuffer1);
            if(ensure2 == 0x00)
            {temp1 = 1;}
           
               
        if(temp1)
        {
            
            for(i = 0; i < 2; i ++)
            {
                Delay(300);
                P2_0 = 1;
                P2_1 = 0;
                temp1 = 0;
                
                ensure3 = AS608_HighSpeedSearch(CharBuffer1, 0, PageNum, &FigerID);

                if(ensure3 == 0x00)
                {temp2 = 1;}

                  

            }
            
            if(temp2)
            {
                P2_2 = 0;
                temp2 = 0;
               
                if( FigerID >= 0 && FigerID <= 2)
                {LCD_ShowString(2, 1, "S.J.W");}
                else if(FigerID >= 3 && FigerID <= 5)
                {LCD_ShowString(2, 1, "Z.J");}
                else if(FigerID >= 6 && FigerID <= 8)
                {LCD_ShowString(2, 1, "S.C.Y");}
                else if(FigerID >= 9 && FigerID <= 11)
                {LCD_ShowString(2, 1, "C.Z.H");}
                Delay(1000);
                
                P2 = 0xFF;
               
                return 1;
                 
            }
        }

    } else {
        return 0;
    }
}