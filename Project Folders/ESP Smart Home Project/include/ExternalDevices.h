
/*******************************************************
 * ESP32-C3 FH4 Ctrl Platform
 * Copyright (c) 2025 WilliTourt 2944925833@qq.com
*******************************************************/

#ifndef EXTERNALDEVICES_H
#define EXTERNALDEVICES_H

#include <Arduino.h>
#include <DHT.h>
#include <oled.h>

#define oledSDA 5
#define oledSCL 6
#define DHTpin 7
#define DHTtype DHT22

#define buzzer 4
#define OK 0
#define ERROR 1
#define E 3
#define S 2


DHT dht(DHTpin, DHTtype);


/**
  * @brief  设置外设引脚模式
  * @param  None
  * @retval None
  */
void setPinMode() {
	pinMode(oledSDA, OUTPUT);
	pinMode(oledSCL, OUTPUT);
	pinMode(buzzer, OUTPUT);
	pinMode(OK, OUTPUT);
	pinMode(ERROR, OUTPUT);
	pinMode(E, OUTPUT);
	pinMode(S, INPUT_PULLUP);
}

void sleepMode() {
	pinMode(OK, INPUT);
	pinMode(ERROR, INPUT);
	pinMode(E, INPUT);
	digitalWrite(OK, LOW);
	digitalWrite(ERROR, LOW);
	digitalWrite(E, LOW);
	
	oledClear();
	// oledPrintText(16, 7, ">> Sleep Mode <<", 8);
	oledPrintImage(111, 6, 16, 2, moon_outline);
}

void wakeUpMode() {
	pinMode(OK, OUTPUT);
	pinMode(ERROR, OUTPUT);
	pinMode(E, OUTPUT);
	digitalWrite(OK, HIGH);
	digitalWrite(ERROR, LOW);
	digitalWrite(E, LOW);
	
	oledClear();
	oledPrintText(27, 0, "ESP32 Server", 8);
}

/**
  * @brief  初始化外设
  * @param  None
  * @retval None
  */
void initExtDevices() {
	setPinMode();
	dht.begin();
	oledInit();
	oledClear();
}

#endif
