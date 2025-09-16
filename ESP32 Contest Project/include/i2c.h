/*
This IIC library is transplanted from my STC89C52RC microcontroller library.
*/

#ifndef I2C_H_
#define I2C_H_

#include <Arduino.h>

#define SCL 2
#define SDA 1

/**
  * @brief  IIC初始化函数
  * @param  None
  * @retval None
  */
void i2cStart() {
    digitalWrite(SDA, HIGH);
    digitalWrite(SCL, HIGH);
    
    digitalWrite(SDA, LOW);
    digitalWrite(SCL, LOW);
}

/**
  * @brief  IIC结束函数
  * @param  None
  * @retval None
  */
void i2cStop() {
    digitalWrite(SDA, LOW);
    digitalWrite(SCL, HIGH);
    
    digitalWrite(SDA, HIGH);
//    digitalWrite(SCL, HIGH);                            //
}

/**
  * @brief  IIC发送一个字节
  * @param  Data 需要发送的字节
  * @retval None
  */
void i2cSendByte(uint8_t Data) {
    unsigned char i;
    digitalWrite(SCL, LOW);

    for (i = 0; i < 8; i++) {
        digitalWrite(SDA, (Data & (0x80 >> i))); // 使用&=, 将Data中对应的位写入
        digitalWrite(SCL, HIGH);
        digitalWrite(SCL, LOW);
    }

//    digitalWrite(SCL, LOW);                          //
}

/**
  * @brief  IIC检查应答
  * @param  None
  * @retval ack, 0应答, 1不应答
  */
bool i2cChkACK() {
    bool ack = true;
    // digitalWrite(SCL, LOW);                            //
    digitalWrite(SDA, HIGH);

    digitalWrite(SCL, HIGH);
    if (digitalRead(SDA) == LOW) {
        ack = false;
    }
    digitalWrite(SCL, LOW);

    return ack;
}

/**
  * @brief  IIC发送应答
  * @param  ack 应答信息, 0应答, 1不应答
  * @retval None
  */
void i2cSendACK(bool ack) {
    // digitalWrite(SCL, LOW);                            //
    digitalWrite(SDA, ack);
    digitalWrite(SCL, HIGH);
    digitalWrite(SCL, LOW);
}

/**
  * @brief  IIC读取一个字节
  * @param  None
  * @retval Data 获取到的字节
  */
uint8_t i2cReadByte() {
    uint8_t Data = 0x00;
    unsigned char i;
    // digitalWrite(SCL, LOW);                            //
    digitalWrite(SDA, HIGH);

    for (i = 0; i < 8; i++) {
        digitalWrite(SCL, HIGH);
        if (digitalRead(SDA) == HIGH) {
            Data |= (0x80 >> i);
        }
        digitalWrite(SCL, LOW);
    }

    return Data;
}


#endif

