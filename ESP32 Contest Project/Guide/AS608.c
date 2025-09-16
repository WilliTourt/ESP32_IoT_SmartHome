#include <REGX52.H>
#include "Uart.h"
#include "LCD1602.h"
#include "Delay.h"

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long


ulong AS608_Adds = 0xFFFFFFFF;
extern uchar UART_Receives[50];
extern uchar numOfUART_Receives;

code unsigned char AS608_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  //协议包头
code unsigned char AS608_Get_Img[6] = {0x01,0x00,0x03,0x01,0x00,0x05}; 

void SendHead()
{
    uchar i;
    for(i = 0; i < 6; i++)
    {
        UART_SendByte(AS608_Pack_Head[i]);
    }  
}

void SendFlag(uchar flag)
{
    UART_SendByte(flag);
}

void SendLength(uint length)
{
	UART_SendByte(length >> 8);
	UART_SendByte(length);
}

void Sendcmd(uchar cmd)
{
	UART_SendByte(cmd);
}

void SendCheck(uint check)
{
	UART_SendByte(check >> 8);
	UART_SendByte(check);
}

uchar AS608_GetImage()
{
    uchar i;
    uchar ensure;
    SendHead();
    
    for(i = 0; i < 6; i ++)
    {
        UART_SendByte(AS608_Get_Img[i]);
    }
    
    
    if(numOfUART_Receives == 12)
    {
        ensure = UART_Receives[9];
        numOfUART_Receives = 0;
        return ensure;
    }
    else
    {
        ensure = 0xFF;
        numOfUART_Receives = 0;
        return ensure;
    }
}

uchar AS608_GenChar(uchar BufferID)
{
    uint check;
    uchar ensure;
    SendHead();
    
    SendFlag(0x01);
    SendLength(0x04);
    Sendcmd(0x02);
    UART_SendByte(BufferID);
    check = 0x01 + 0x04 + 0x02 + BufferID; 
    SendCheck(check);
    if(numOfUART_Receives == 12)
    {
        ensure = UART_Receives[9];
        numOfUART_Receives = 0;
        return ensure;
    }
    else
    {
        ensure = 0xFF;
        numOfUART_Receives = 0;
        return ensure;
    }
  
}

uchar AS608_HighSpeedSearch(uchar BufferID, uchar StartPage, uchar PageNum, uchar* pageID)
{
    uint check;
    uchar ensure;
    SendHead();
    
    SendFlag(0x01);
    SendLength(0x08);
    Sendcmd(0x04);
    UART_SendByte(BufferID);
    UART_SendByte(StartPage >> 8);
    UART_SendByte(StartPage);
    UART_SendByte(PageNum >> 8);
    UART_SendByte(PageNum);
    check = 0x01 + 0x08 + 0x04 + BufferID + StartPage + PageNum;
    SendCheck(check);
    
    
    if(numOfUART_Receives == 16)
    {
        ensure = UART_Receives[9];
        *pageID = UART_Receives[10] + UART_Receives[11];  
        numOfUART_Receives = 0;
        return ensure;
    }
    else
    {
        ensure = 0xFF;
        numOfUART_Receives = 0;
        return ensure;
    }

}

