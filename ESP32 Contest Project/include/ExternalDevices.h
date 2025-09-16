/*******************************************************
 * ESP32 Control Platform
 * Copyright (c) 2024 WilliTourt 2944925833@qq.com
*******************************************************/


#ifndef EXTERNALDEVICES_H
#define EXTERNALDEVICES_H

#include <Arduino.h>

#include <ESP32Servo.h>
#include <DHT.h>
#include <oled.h>
#include <Keypad.h>

char keys[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},	
    {'7','8','9','C'},
    {'*','0','#','D'}
};
byte rowPins[4] = {35, 36, 37, 38};// p b g y
byte colPins[4] = {39, 40, 41, 42};// r r1 r2 r3

#define oledSDA 1 //
#define oledSCL 2 //
#define LED 21 //
#define DHTpin 47 //
#define DHTtype DHT22
#define doorServo 17 // 
#define windowServo 18 //
#define buzzer 48 //

#define r2 4
#define r1 5
#define l1 6
#define l2 7

#define as608WCK 45


Servo door;
Servo window;
DHT dht(DHTpin, DHTtype);

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);


/**
  * @brief  设置外设引脚模式
  * @param  None
  * @retval None
  */
void setPinMode() {
	pinMode(oledSDA, OUTPUT);
	pinMode(oledSCL, OUTPUT);
	pinMode(LED, OUTPUT);
	pinMode(buzzer, OUTPUT);
	pinMode(r2, OUTPUT);
	pinMode(r1, OUTPUT);
	pinMode(l1, OUTPUT);
	pinMode(l2, OUTPUT);
	// pinMode(as608WCK, INPUT);
}

/**
  * @brief  初始化外设
  * @param  None
  * @retval None
  */
void initExtDevices() {
	setPinMode();
	dht.begin();
	door.setPeriodHertz(50);
  	door.attach(doorServo, 500, 2500);
	window.setPeriodHertz(50);
  	window.attach(windowServo, 500, 2500);
	oledInit();
	oledClear();
	// Serial.begin(57600);
}

#endif