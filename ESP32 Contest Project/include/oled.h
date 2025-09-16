#ifndef OLED_H_
#define OLED_H_

#include <Arduino.h>
#include <i2c.h>
#include <oledCharacters.h>
#include <stdio.h>

#define OLED_SCL SCL
#define OLED_SDA SDA
#define OLED_IIC_ADDR 0x78



/**
  * @brief  OLED 写入命令
  * @param  command: 命令
  * @retval None
  */
void oledWriteCommand(uint8_t command) {
    i2cStart();
    i2cSendByte(OLED_IIC_ADDR);
    i2cChkACK();
    i2cSendByte(0x00);
    i2cChkACK();
    i2cSendByte(command);
    i2cChkACK();
    i2cStop();
}

/**
  * @brief  OLED 向GDDRAM写入数据
  * @param  Data: 数据
  * @retval None
  */
void oledWriteData(uint8_t Data) {
    i2cStart();
    i2cSendByte(OLED_IIC_ADDR);
    i2cChkACK();
    i2cSendByte(0x40);
    i2cChkACK();
    i2cSendByte(Data);
    i2cChkACK();
    i2cStop();
}

/**
  * @brief  OLED 初始化
  * @param  None
  * @retval None
  */
void oledInit() {
    i2cStart();

    delay(100);

    oledWriteCommand(0xAE); //关闭显示

    oledWriteCommand(0xD5);
    oledWriteCommand(0x80); //设置时钟分频比/振荡器频率

    oledWriteCommand(0xA8);
    oledWriteCommand(0x3F); //设置驱动路数

    oledWriteCommand(0xD3);
    oledWriteCommand(0x00); //设置显示偏移

    oledWriteCommand(0x40); //设置显示起始行

    oledWriteCommand(0xA1); //设置段重映射

    oledWriteCommand(0xC8); //设置COM扫描方向

    oledWriteCommand(0xDA);
    oledWriteCommand(0x12); //设置COM硬件配置

    oledWriteCommand(0x81);
    oledWriteCommand(0xCF); //设置亮度

    oledWriteCommand(0xD9);
    oledWriteCommand(0xF1); //设置预充电周期

    oledWriteCommand(0xDB);
    oledWriteCommand(0x30); //设置VCOMH

    oledWriteCommand(0xA4); //设置显示开启

    oledWriteCommand(0xA6); //设置正常显示

    oledWriteCommand(0x8D);
    oledWriteCommand(0x14); //设置充电泵开启

    oledWriteCommand(0xAF); //开启显示

    delay(100);
}

/**
  * @brief  OLED 设置光标位置
  * @param  x: X坐标 (0-127)
  * @param  y: 行坐标 (0-7)
  * @note   (0,0)------------->(127,0)
  * @note   (0,7)------------->(127,7)
  * @retval None
  */
void oledSetCursor(uint8_t x, uint8_t y) {
    if (y > 7) {
        y = 7;
    }
    if (x > 127) {
        x = 127;
    }
    oledWriteCommand(0xB0 + y); //设置页地址
    oledWriteCommand(0x00 | (x & 0x0F)); //设置列低地址
    oledWriteCommand(0x10 | ((x & 0xF0) >> 4)); //设置列高地址
}

/**
  * @brief  OLED 清屏
  * @param  None
  * @retval None
  */
void oledClear() {
    uint8_t i, j;
    for (i = 0; i < 8; i++) {
        oledSetCursor(0, i);
        for (j = 0; j < 128; j++) {
            oledWriteData(0x00);
        }
    }
    oledSetCursor(0, 0);
}

/**
  * @brief  OLED 清除部分屏幕
  * @param  x1: X坐标起始 (0-128)
  * @param  y1: 行坐标起始 (0-8)
  * @param  x2: X坐标结束 (0-128)
  * @param  y2: 行坐标结束 (0-8)
  * @retval None 
  */
void oledClearPart(u8_t x1, u8_t y1, u8_t x2, u8_t y2) {
    uint8_t i, j;
    for (i = y1; i < y2; i++) {
        oledSetCursor(x1, i);
        for (j = x1; j < x2; j++) {
            oledWriteData(0x00);
        }
    }
    oledSetCursor(x1, y1);
}

/**
  * @brief  OLED 显示文本(暂仅支持ASCII字符)
  * @param  x: X坐标 (0-127)
  * @param  y: 行坐标 (0-7)
  * @param  str: 显示的字符串
  * @param  size: 字体大小 (8/16)
  * @retval None
  */
void oledPrintText(uint8_t x, uint8_t y, String str, uint8_t size) {
    uint8_t i, j = 0;
    uint8_t character;

    if (str[1] == '\0') {

        character = str[0];
        oledSetCursor(x, y);
        if (size == 8) {
            for (i = 0; i < 6; i++) {
                oledWriteData(oledASCII6x8[character - ' '][i]);
            }
        } else if (size == 16) {
            for (i = 0; i < 8; i++) {
                oledWriteData(oledASCII8x16[character - ' '][i]);
            }
            oledSetCursor(x, y + 1);
            for (i = 0; i < 8; i++) {
                oledWriteData(oledASCII8x16[character - ' '][i + 8]);
            }
        }
    
    } else {

        while (str[j] != '\0') {
            oledSetCursor(x, y);
            character = str[j]; //获取当前字符

            if (size == 8 && (character - ' ') < 95) {
                for (i = 0; i < 6; i++) {
                    oledWriteData(oledASCII6x8[character - ' '][i]);
                }
                x += 6;
            } else if (size == 16 && (character - ' ') < 95) {
                for (i = 0; i < 8; i++) {
                    oledWriteData(oledASCII8x16[character - ' '][i]);
                }
                oledSetCursor(x, y + 1);
                for (i = 0; i < 8; i++) {
                    oledWriteData(oledASCII8x16[character - ' '][i + 8]);
                }
                x += 8;
            }

            j++; //指向下一个字符

            if (x > 127) { //超出屏幕，换行
                x = 0;
                if (size == 8) {
                    y++;
                } else if (size == 16) {
                    y += 2;
                }
            }
        }
    }
}

/**
  * @brief  OLED 显示图片
  * @param  x: X坐标 (0-127)
  * @param  y: Y坐标 (0-63)
  * @param  width: 图片宽度 (1-128)
  * @param  height: 图片高度 (1-8)
  * @param  image: 图片数据(一维数组)
  * @retval None
  */
void oledPrintImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t* image) {
    uint8_t i, j;
    for (i = 0; i < height; i++) {
        oledSetCursor(x, y + i);
        for (j = 0; j < width; j++) {
            oledWriteData(image[i * width + j]);
        }
    }
}

bool cmpStrings(const char* str1, const char* str2) {
    while (*str1 && *str2) { // 当两个字符串都没结束时
        if (*str1 != *str2) {
            return false; // 如果字符不同，返回 false
        }
        str1++;
        str2++;
    }
    return (*str1 == *str2); // 如果两者都到达字符串末尾，则相等
}

/**
  * @brief  OLED 显示变量
  * @param  x: X坐标 (0-127)
  * @param  y: Y坐标 (0-7)
  * @param  num: 变量值
  * @param  type: 变量类型 ("int"/"float")
  * @param  length: 打印长度 (1/2/3)
  * @retval None
  */
void oledPrintVariable(uint8_t x, uint8_t y, float num, const char* type, uint8_t length) {
    uint8_t i, j;
    char str[50];

    if (cmpStrings(type, "int")) {
        if (length == 1) {
            sprintf(str, "%d", (int)num);
        } else if (length == 2) {
            sprintf(str, "%02d", (int)num);
        } else if (length == 3) {
            sprintf(str, "%03d", (int)num);
        }
    } else if (cmpStrings(type, "float")) {
        if (length == 1) {
            sprintf(str, "%.1f", num);
        } else if (length == 2) {
            sprintf(str, "%.2f", num);
        } else if (length == 3) {
            sprintf(str, "%.3f", num);
        }
    } else {
        sprintf(str, "ERR");
    }

    for (j = 0; str[j] != '\0'; j++) {
        oledSetCursor(x, y);
        for (i = 0; i < 6; i++) {
            oledWriteData(oledASCII6x8[str[j] - ' '][i]);
        }
        x += 6;
    }
}

/**
  * @brief  OLED 显示水平
  * @param  x: X坐标 (0-127)
  * @param  y: Y坐标 (0-7)
  * @param  width: 线宽 (1-128)
  * @retval None
  */
void oledPrintHLine(uint8_t x, uint8_t y, uint8_t width) {
    uint8_t i;
    for (i = 0; i < width; i++) {
        oledSetCursor(x + i, y);
        oledWriteData(0x01);
    }
}

/**
  * @brief  OLED 显示垂直线
  * @param  x: X坐标 (0-127)
  * @param  y: Y坐标 (0-7)
  * @param  height: 线高 (1-8)
  * @retval None
  */
void oledPrintVLine(uint8_t x, uint8_t y, uint8_t height) {
    uint8_t i;
    for (i = 0; i < height; i++) {
        oledSetCursor(x, y + i);
        oledWriteData(0xFF);
    }
}

#endif