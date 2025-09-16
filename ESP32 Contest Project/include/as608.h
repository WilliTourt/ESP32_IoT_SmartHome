#include <Arduino.h>
#include <oled.h>

unsigned char rxBuffer[20];
unsigned char rxCount = 0;

unsigned char AS608_PackHead[6] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned char AS608_getImage[6] = {0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
unsigned char AS608_genChara[4] = {0x01, 0x00, 0x04, 0x02};
unsigned char AS608_matchChara[6] = {0x01, 0x00, 0x03, 0x03, 0x00, 0x07};
unsigned char AS608_genTemplate[6] = {0x01, 0x00, 0x03, 0x05, 0x00, 0x09};
unsigned char AS608_storeTemplate[4] = {0x01, 0x00, 0x06, 0x06};
unsigned char AS608_delTemplate[4] = {0x01, 0x00, 0x07, 0x0C};
unsigned char AS608_highSpeedSearch[4] = {0x01, 0x00, 0x08, 0x1B};
unsigned char AS608_clrTemplate[6] = {0x01, 0x00, 0x03, 0x0D, 0x00, 0x11};





void SendHead() {
    for (int i = 0; i < 6; i++) {
        Serial.write(AS608_PackHead[i]);
    }
}

void SendFlag(unsigned char flag) {
    Serial.write(flag);
}

void SendLength(int length) {
    Serial.write(length >> 8);
    Serial.write(length);
}

void SendCmd(unsigned char cmd) {
    Serial.write(cmd);
}

void SendCheck(ushort check) {
    Serial.write(check >> 8);
    Serial.write(check);
}





/**
  * @brief  获取指纹图像
  * @param  None
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 03 01 00 05
  */
unsigned char getImage() {
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 6; i++) {
        Serial.write(AS608_getImage[i]);
    }

    delay(500);

    ensure = rxBuffer[9];
    rxCount = 0;
    return ensure;
}

/**
  * @brief  生成指纹图像特征
  * @param  BufferID 指纹特征编号
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 04 02 BufferID sum
  */
unsigned char genChara(unsigned char BufferID) {
    unsigned short chk;
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 4; i++) {
        Serial.write(AS608_genChara[i]);
    }
    Serial.write(BufferID);
    chk = 0x01 + 0x04 + 0x02 + BufferID;
    SendCheck(chk);

    delay(500);

    ensure = rxBuffer[9];
    rxCount = 0;

    return ensure;
}

/**
  * @brief  比对两指纹特征
  * @param  None
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 03 03 00 07
  */
unsigned char matchChara() {
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 6; i++) {
        Serial.write(AS608_matchChara[i]);
    }

    ensure = rxBuffer[9];
    rxCount = 0;

    return ensure;
}

/**
  * @brief  生成指纹模板
  * @param  None
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 03 05 00 09
  */
unsigned char genTemplate() {
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 6; i++) {
        Serial.write(AS608_genTemplate[i]);
    }

    ensure = rxBuffer[9];
    rxCount = 0;

    return ensure;
}

/**
  * @brief  存储指纹模板
  * @param  BufferID 指纹特征编号
  * @param  pageID 指纹模板页号
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 06 06 BufferID pageID sum
  */
unsigned char storeTemplate(unsigned char BufferID, unsigned short pageID) {
    unsigned short chk;
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 4; i++) {
        Serial.write(AS608_storeTemplate[i]);
    }
    Serial.write(BufferID);
    Serial.write(pageID >> 8);
    Serial.write(pageID);
    chk = 0x01 + 0x06 + 0x06 + BufferID + (pageID >> 8) + (unsigned char)pageID;
    SendCheck(chk);

    ensure = rxBuffer[9];
    rxCount = 0;

    return ensure;
}

/**
  * @brief  删除指纹模板
  * @param  PageID 指纹模板页号
  * @param  delNum 要删除的模板数量
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 07 0C PageID delNum sum
  */
unsigned char delTemplate(unsigned short PageID, unsigned short delNum) {
    unsigned short chk;
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 4; i++) {
        Serial.write(AS608_delTemplate[i]);
    }
    Serial.write(PageID >> 8);
    Serial.write(PageID);
    Serial.write(delNum >> 8);
    Serial.write(delNum);
    chk = 0x01 + 0x07 + 0x0C + (PageID >> 8) + (unsigned char)PageID + (delNum >> 8) + (unsigned char)delNum;
    SendCheck(chk);
    
    ensure = rxBuffer[9];
    rxCount = 0;

    return ensure;
}

/**
  * @brief  高速搜索指纹库
  * @param  BufferID 指纹特征编号
  * @param  *fingerprintID 指纹特征ID的地址
  * @param  StartPage 起始页号
  * @param  PageNum 页数
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 08 1B BufferID StartPage PageNum sum
  */
unsigned char highSpeedSearch(unsigned char BufferID, unsigned char *fingerprintID, unsigned char StartPage, unsigned char PageNum) {
    unsigned char chk;
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 4; i++) {
        Serial.write(AS608_highSpeedSearch[i]);
    }
    
    Serial.write(BufferID);
    Serial.write(0x00);
    Serial.write(StartPage);
    Serial.write(0x00);
    Serial.write(PageNum);
    chk = 0x01 + 0x08 + 0x1B + BufferID + StartPage + PageNum;
    SendCheck(chk);

    delay(1000);

    ensure = rxBuffer[9];
    *fingerprintID = rxBuffer[10] + rxBuffer[11];

    rxCount = 0;

    return ensure;
}

/**
  * @brief  清空指纹库
  * @param  None
  * @retval ensure
  * @note   指令包：EF 01 FF FF FF FF 01 00 03 0D 00 11
  */
unsigned char clrTemplate() {
    unsigned char ensure;

    SendHead();
    for (int i = 0; i < 6; i++) {
        Serial.write(AS608_clrTemplate[i]);
    }

    ensure = rxBuffer[9];
    rxCount = 0;

    return ensure;
}


unsigned char Handshake(unsigned int* Addr)
{
    SendHead();
    Serial.write(0x01);
    Serial.write(0x00);
    Serial.write(0x00);
    
    delay(200);

    if(rxBuffer[0] = 0xEF && rxBuffer[1] == 0xBE && rxBuffer[2] == 0x01 && rxBuffer[3] == 0x07) {
        *Addr = ( rxBuffer[2]<<24 + (rxBuffer[3]<<16) + (rxBuffer[4]<<8) + rxBuffer[5] );
        return 0;
    }
    return 1;
}