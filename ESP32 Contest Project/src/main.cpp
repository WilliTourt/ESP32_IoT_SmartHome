/*******************************************************
 * ESP32 Control Platform
 * Copyright (c) 2024 WilliTourt 2944925833@qq.com
*******************************************************/


#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <SPIFFS.h>

#include <ExternalDevices.h>
// #include <fingerprint.h>
#include <arduino-timer.h>

WiFiMulti wifiMulti;
WebServer server(80);

WiFiUDP ntp;

const char* ntpServer = "pool.ntp.org"; 		//NTP服务器地址
NTPClient ntpClient(ntp, ntpServer, 3600*8); 	//UTC+8

struct TimerTask { 		//定时任务结构体
	String device;
	String action;
	String timeStr;
} timerTask;

struct DeviceInfo { 	//设备数据结构体
	String ledStatus;
	String doorStatus;
	String windowStatus;
	String guardStatus;
	float dhtTemp;
	float dhtHumi;
	bool auth;
} deviceInfo;

bool doorState = 0;				//门状态
bool auto_Window = 0;			//自动窗帘状态

bool taskScheduled = false; 	//标记是否有任务
bool alarmScheduled = false; 	//标记是否有闹钟
bool taskExecuted = false; 		//标记是否执行了任务
bool alarmExecuted = false; 	//标记是否执行了闹钟

String scheduledTask; 			//存放定时任务
String scheduledAlarm; 			//存放闹钟时间

String password = "123456";		//初始密码123456
String enteredPassword = "";
unsigned char pwdErrCount = 0;	//密码错误次数

void handleUserRequest();
void handleNtpRequest();
void handleDeviceInfoRequest();
void handleANTRequest();

void handleTimer();
void handleTimerTask();
void chkTimer();
void handleAlarm();
void chkAlarm();

void handlePassword();
void handlekeypad();

void youAreDone();
void autoWindow();
// void handleAudio();

// void addFR();
// void delFR();

void handleLED();
void handleDoor();
void handleWindow();
void handleDHT();

void oledPrintIP();
void oledPrintReconnect();
bool handleFileRead(String resource);
String getContentType(String filename);

Timer<1, micros> timer;

// bool checkUART(void *) {
// 	while (Serial.available()) {
// 		if (rxCount < 20) {
// 			rxBuffer[rxCount] = Serial.read();
// 			rxCount++;
// 		}
// 	}
//    return true;
// }

// void IRAM_ATTR AS608_Receive() {
// 	while (Serial.available()) {
// 		if (rxCount < 20) {
// 			rxBuffer[rxCount - 1] = Serial.read();	//潜在bug
// 			rxCount++;
// 		}
// 	}
// }

bool infoUpdate(void *) {

	server.handleClient();
	ntpClient.update();
	chkTimer();
	chkAlarm();

   	return true;
}

void setup() {

	initExtDevices();

	oledPrintText(27, 0, "ESP32 Server", 8);
	oledPrintText(3, 1, "Initializing...", 8);

	delay(20);
	oledPrintText(3, 2, "WiFi connecting...", 8);

	// wifiMulti.addAP("WilliTourt", "williamtourtei");
	// wifiMulti.addAP("CMCC_R7cY", "a9asfeba");
	wifiMulti.addAP("Ciallo~☆", "williamtourtei"); //手机热点

	while (wifiMulti.run() != WL_CONNECTED);
	oledPrintText(3, 2, "WiFi connected.   ", 8);

	if (SPIFFS.begin()) {
		oledPrintText(3, 3, "SPIFFS Started.", 8);
	} else {
		oledPrintText(3, 3, "---SPIFFS ERROR!---", 8);
	}

	server.on("/led", handleLED);					//JS handleLED();
	server.on("/door", handleDoor);					//JS handleDoor();
	server.on("/window", handleWindow);				//JS handleWindow();
	server.on("/dht", handleDHT);					//JS fetchDHT();

	server.on("/PWD", handlePassword);				//JS setPWD();
	// server.on("/doorAudio", handleAudio);		//JS doorAudio();
	// server.on("/addFinger", addFR);				//JS addFinger();
	// server.on("/delFinger", delFR); 				//JS delFinger();

	server.on("/alarm", handleAlarm);				//JS alarmTask();  chkAlarm();
	server.on("/timer", handleTimer);				//JS timerTask();  chkScheduledTask();
	server.on("/aNt", handleANTRequest);			//JS fetchANT();

	server.on("/time", handleNtpRequest);			//JS syncTime();
	server.on("/device", handleDeviceInfoRequest); 	//JS getDeviceInfo();
	server.on("/esp32.html", handleUserRequest);	//esp32.html Homepage

	server.onNotFound(handleUserRequest);
	server.begin();
	oledPrintText(3, 4, "HTTP server started.", 8);

	ntpClient.begin();
	oledPrintText(3, 5, "NTP client started.", 8);

	oledPrintText(3, 7, "Initialization OK!", 8);
	delay(4000);
	oledPrintIP();

	// Serial.begin(57600, SERIAL_8N2);
	// Serial.setRxBufferSize(20);
    // attachInterrupt(digitalPinToInterrupt(44), AS608_Receive, CHANGE);

	deviceInfo.auth = true;

	timer.every(125, infoUpdate);

}

void loop() {
	timer.tick();

	// server.handleClient();
	// ntpClient.update();
	// chkTimer();
	// chkAlarm(); 

	if (keypad.getKey() == 'A') {
		handlekeypad();
	}

	if (wifiMulti.run() != WL_CONNECTED) {
		while (wifiMulti.run() != WL_CONNECTED) {
			oledPrintReconnect();
		}
		oledPrintIP();
	}

	autoWindow();

	// if (Serial.available() > 0) {
	// 	rxBuffer[rxCount++] = Serial.read();
	// }

}





void handleLED() {
	digitalRead(LED) == HIGH ? digitalWrite(LED, LOW) : digitalWrite(LED, HIGH);
	deviceInfo.ledStatus = (digitalRead(LED) == HIGH ? "on" : "off");

	server.send(200, "text/plain", deviceInfo.ledStatus);
}

void handleDoor() {

	if (doorState) {
		door.write(3);
		deviceInfo.doorStatus = "closed";
	} else {
		door.write(90);
		deviceInfo.doorStatus = "opened";
	}

	doorState = !doorState;

	server.send(200, "text/plain", deviceInfo.doorStatus);
}

void handleWindow() {
	String value = server.arg("plain");

	window.write(value.toInt());
	deviceInfo.windowStatus = value;

	server.send(200, "text/plain", value);
}

void handleDHT() {
	String dhtInfo;
	deviceInfo.dhtTemp = dht.readTemperature();
	deviceInfo.dhtHumi = dht.readHumidity();

	if (isnan(deviceInfo.dhtTemp) || isnan(deviceInfo.dhtHumi)) {
		dhtInfo = "0,0";
	} else {
		dhtInfo = String(deviceInfo.dhtTemp) + "," + String(deviceInfo.dhtHumi);
	}
	
	server.send(200, "text/plain", dhtInfo);
}

void autoWindow() {
	if (deviceInfo.dhtHumi > 87 && auto_Window == true) {
		window.write(0);
		deviceInfo.windowStatus = "0";
	} else if (deviceInfo.dhtHumi < 75 && auto_Window == true) {
		window.write(90);
		deviceInfo.windowStatus = "90";
	}
}

//处理设备信息获取请求
void handleDeviceInfoRequest() {
	String info = 	deviceInfo.ledStatus + "," + 
					deviceInfo.doorStatus + "," + 
					deviceInfo.windowStatus + "," + 
					deviceInfo.guardStatus + "," + 
					String(deviceInfo.dhtTemp) + "," + 
					String(deviceInfo.dhtHumi) + "," + 
					(deviceInfo.auth ? "true" : "false");

	server.send(200, "text/plain", info);
}



void handleAlarm() {
	alarmExecuted = false;
	String command = server.arg("plain"); 			//获取POST请求
	alarmScheduled = true;
	scheduledAlarm = command; 						//网页发来的闹钟时间(hh:mm:ss)

	server.send(200);
}

//检查闹钟是否到期
void chkAlarm() {
	if (alarmScheduled) {
		if (ntpClient.getFormattedTime() == scheduledAlarm) {
			digitalWrite(buzzer, HIGH);
			delay(1000);
			digitalWrite(buzzer, LOW);
			alarmExecuted = true;
			alarmScheduled = false;
		}
	}
}

//处理定时器请求
void handleTimer() {
	taskExecuted = false;
	String command = server.arg("plain"); 			//获取POST请求

	int actionIndex = command.indexOf("-");
	int timeIndex = command.indexOf("-", actionIndex + 1);

	String device = command.substring(0, actionIndex);
	String action = command.substring(actionIndex + 1, timeIndex);
	String timeStr = command.substring(timeIndex + 1);
	timeStr.replace("-", ":");

	timerTask.device = device;
	timerTask.action = action;
	timerTask.timeStr = timeStr; 					//网页发来的定时器任务

	taskScheduled = true;

	if (action == "auto") {
		scheduledTask = "";
	} else {
		scheduledTask = device + " " + action + " at " + timeStr;
	}

	server.send(200);
}

//检查定时器任务是否到期
void chkTimer() {
	if (taskScheduled) {
		if (ntpClient.getFormattedTime() == timerTask.timeStr) {
			handleTimerTask();
			taskScheduled = false;
		}
	}
}

//处理定时器任务
void handleTimerTask() {
	if (timerTask.device == "led") {
		if (timerTask.action == "on" || timerTask.action == "open" || timerTask.action == "up") {
			digitalWrite(LED, HIGH);
			deviceInfo.ledStatus = "on";
		} else if (timerTask.action == "off" || timerTask.action == "close" || timerTask.action == "down") {
			digitalWrite(LED, LOW);
			deviceInfo.ledStatus = "off";
		} else if (timerTask.action == "toggle") {
			digitalRead(LED) == HIGH ? digitalWrite(LED, LOW) : digitalWrite(LED, HIGH);
			deviceInfo.ledStatus = (digitalRead(LED) == HIGH ? "on" : "off");
		}
	} else if (timerTask.device == "door") {
		if (timerTask.action == "on" || timerTask.action == "open" || timerTask.action == "up") {
			door.write(90);
			deviceInfo.doorStatus = "opened";
		} else if (timerTask.action == "off" || timerTask.action == "close" || timerTask.action == "down") {
			door.write(3);
			deviceInfo.doorStatus = "closed";
		} else if (timerTask.action == "toggle") {
			if (doorState) {
				door.write(3);
				deviceInfo.doorStatus = "closed";
			} else {
				door.write(90);
				deviceInfo.doorStatus = "opened";
			}
			doorState = !doorState;
		}
	} else if (timerTask.device == "window") {
		if (timerTask.action == "on" || timerTask.action == "open" || timerTask.action == "up") {
			window.write(90);
			deviceInfo.windowStatus = "90";
		} else if (timerTask.action == "off" || timerTask.action == "close" || timerTask.action == "down") {
			window.write(0);
			deviceInfo.windowStatus = "0";
		} else if (timerTask.action == "toggle") {
			if (window.read() < 45) {
				window.write(90);
				deviceInfo.windowStatus = "90";
			} else {
				window.write(0);
				deviceInfo.windowStatus = "0";
			}
		} else if (timerTask.action == "auto") {
			auto_Window = true;
		} else if (timerTask.action == "manual") {
			auto_Window = false;
		}
	}
	/*else if {...}*/
	taskExecuted = true;
}

void handleANTRequest() {
    String info = "";

    if (!alarmExecuted && alarmScheduled) {
        info += scheduledAlarm + ",";
    } else {
        info += "0,";
    }

    if (!taskExecuted && taskScheduled) {
        info += scheduledTask;
    } else {
        info += "0";
    }

    server.send(200, "text/plain", info); // 0,0 / alarm,0 / 0,task / alarm,task
}

//处理NTP时钟请求
void handleNtpRequest() {
	time_t epochTime = ntpClient.getEpochTime(); 	//Unix时间戳
	String time = ntpClient.getFormattedTime(); 	//获取格式化时间

	server.send(200, "text/plain", time);
}



void handlePassword() {
	password = server.arg("plain");
	server.send(200);
}

void handlekeypad() {
	unsigned char count = 0;
	unsigned int num;
	enteredPassword = "";
	count = 0;
	oledClearPart(0, 1, 128, 8);
	oledPrintText(16, 2, "PASSWORD INSERT:", 8);
	oledPrintText(3, 3, "Password:", 8);
	while (1) {
		server.handleClient();
		ntpClient.update();
		chkTimer();
		chkAlarm();
		char key = keypad.getKey();
		switch (key) {
			case '1': num = 1; break;
			case '2': num = 2; break;
			case '3': num = 3; break;
			case '4': num = 4; break;
			case '5': num = 5; break;
			case '6': num = 6; break;
			case '7': num = 7; break;
			case '8': num = 8; break;
			case '9': num = 9; break;
			case '0': num = 0; break;
		}
		if (key != NO_KEY){
			oledPrintVariable(3+(6*count), 4, num, "int", 1);
			enteredPassword += String(num);
			count++;
			if(count == password.length()){
				if(enteredPassword == password){
					oledPrintText(3, 5, "PasswordOK!", 8);
					digitalWrite(buzzer, HIGH);
					delay(500);
					digitalWrite(buzzer, LOW);
					door.write(90);
					delay(2500);
					door.write(3);
					pwdErrCount = 0;
					deviceInfo.auth = true;
					break;
				} else {
					oledPrintText(3, 5, "Password ERROR!", 8);
					digitalWrite(buzzer, HIGH);
					delay(100);
					digitalWrite(buzzer, LOW);
					delay(100);
					digitalWrite(buzzer, HIGH);
					delay(100);
					digitalWrite(buzzer, LOW);
					delay(100);
					digitalWrite(buzzer, HIGH);
					delay(100);
					digitalWrite(buzzer, LOW);
					delay(100);

					pwdErrCount++;
					if (pwdErrCount == 3) {
						deviceInfo.auth = false;
						youAreDone();
						pwdErrCount = 0;
					}
					break;
				}
				count = 0;
				enteredPassword = "";
			}
		}	
	}
	oledClearPart(0, 1, 128, 8);
	oledPrintIP();
}

// void addFR() {
// 	String fingerprintStr = server.arg("plain");
// 	int fingerprintID = fingerprintStr.toInt();

// 	addFingerprint(fingerprintID);
	
// 	if (addOK == true) {
// 		server.send(200, "text/plain", "ok");
// 	} else {
// 		server.send(200, "text/plain", "error");
// 	}
// }

// void delFR() {
// 	//Waiting for solution
// }


//处理HTTP请求
void handleUserRequest() {
	String reqResource = server.uri();
	bool fileFound = handleFileRead(reqResource);

	if (!fileFound) {
		server.sendHeader("Location", "/404.html");
		server.send(303);
	}
}





/**
  * @brief  oled显示IP信息
  * @param  None
  * @retval None
  */
void oledPrintIP() {
	oledClearPart(0, 1, 128, 8);
	oledPrintVLine(0, 2, 6);
	oledPrintVLine(1, 2, 6);
	oledPrintText(3, 2, "SSID:", 8);
	oledPrintText(10, 3, WiFi.SSID(), 16);
	oledPrintText(3, 5, "IP:", 8);
	oledPrintText(10, 6, WiFi.localIP().toString(), 16);
}

/**
  * @brief  oled显示WiFi重连信息
  * @param  None
  * @retval None
  */
void oledPrintReconnect() {
	oledClearPart(0, 2, 128, 8);
	delay(400);
	oledPrintVLine(0, 3, 3);
	oledPrintVLine(1, 3, 3);
	oledPrintText(3, 3, "Connection lost!", 8);
	oledPrintText(3, 5, "Reconnecting...", 8);
	delay(100);
}

/**
  * @brief  处理文件读取请求
  * @param  resource 资源地址
  * @retval 是否成功读取
  */
bool handleFileRead(String resource) {

	if (resource.endsWith("/")) {                   //如果访问地址以"/"为结尾
		resource = "/esp32.html";                   //转到主页
	}
	
	String contentType = getContentType(resource);  //获取文件类型
	
	if (SPIFFS.exists(resource)) {                  //如果访问的文件可以在SPIFFS中找到
		File file = SPIFFS.open(resource, "r");     //则尝试打开该文件
		server.streamFile(file, contentType);		//并且将该文件返回给浏览器
		file.close();                               //并且关闭文件
		return true;                                //返回true
	}

	return false;                                   //如果文件未找到，则返回false
}

/**
  * @brief 获取文件类型
  * @param filename 文件名
  * @return 文件类型
  */
String getContentType(String filename){
	if(filename.endsWith(".htm")) return "text/html";
	else if(filename.endsWith(".html")) return "text/html";
	else if(filename.endsWith(".css")) return "text/css";
	else if(filename.endsWith(".js")) return "application/javascript";
	else if(filename.endsWith(".png")) return "image/png";
	else if(filename.endsWith(".gif")) return "image/gif";
	else if(filename.endsWith(".jpg")) return "image/jpeg";
	else if(filename.endsWith(".ico")) return "image/x-icon";
	else if(filename.endsWith(".xml")) return "text/xml";
	else if(filename.endsWith(".pdf")) return "application/x-pdf";
	else if(filename.endsWith(".zip")) return "application/x-zip";
	else if(filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

void youAreDone() {
	int i = 0;
	while (i < 50) {
		digitalWrite(r2, LOW);
		digitalWrite(r1, HIGH);
		digitalWrite(l1, HIGH);
		digitalWrite(l2, LOW);
		delay(80);
		digitalWrite(r2, LOW);
		digitalWrite(r1, LOW);
		digitalWrite(l1, LOW);
		digitalWrite(l2, LOW);
		delay(20);
		digitalWrite(r2, HIGH);
		digitalWrite(r1, LOW);
		digitalWrite(l1, LOW);
		digitalWrite(l2, HIGH);
		delay(80);
		digitalWrite(r2, LOW);
		digitalWrite(r1, LOW);
		digitalWrite(l1, LOW);
		digitalWrite(l2, LOW);
		i++;
	}
	digitalWrite(r2, LOW);
	digitalWrite(r1, HIGH);
	digitalWrite(l1, HIGH);
	digitalWrite(l2, LOW);
	delay(80);
	digitalWrite(r2, LOW);
	digitalWrite(r1, LOW);
	digitalWrite(l1, LOW);
	digitalWrite(l2, LOW);
}



// #include <Arduino.h>
// #include <fingerprint.h>
// // #include <arduino-timer.h>
// #include <oled.h>

// // Timer<1, micros> timer;


// // bool checkUART(void *) {
// // 	while (Serial.available() > 0) {
// // 		if (rxCount < 20) {
// // 			rxBuffer[rxCount] = Serial.read();
// // 			rxCount++;
// // 		}
// // 	}
// //    return true;
// // }


// void IRAM_ATTR onUARTReceive() {
// 	while (Serial.available()) {
// 		rxBuffer[rxCount - 1] = Serial.read();
// 		rxCount++;
// 	}
// }

// void setup() {
// 	int a;
// 	pinMode(5, OUTPUT);
// 	pinMode(6, OUTPUT);
// 	pinMode(42, INPUT);
// 	Serial.begin(57600, SERIAL_8N2);
// 	Serial.setRxBufferSize(20);
    
//  attachInterrupt(digitalPinToInterrupt(44), onUARTReceive, CHANGE);
// 	oledInit();
// 	oledClear();

// 	// timer.every(1000, checkUART);
// 	oledPrintText(0, 0, "check in 3s...", 16);
// 	delay(3000);

// 	// a = scanFingerprint();

// 	// if (chkOK == true) {
// 	// 	oledPrintText(0, 5, "ok", 8);
// 	// } else {
// 	// 	oledPrintText(0, 5, "error", 8);
// 	// }
// 	// oledPrintText(0, 6, "return ID:", 8);
// 	// oledPrintVariable(0, 7, a, "int", 3);

// 	// delay(1000);
// 	addFingerprint(106);
// 	if (addOK == true) {
// 		oledPrintText(0, 5, "ok", 8);
// 	} else {
// 		oledPrintText(0, 5, "error", 8);
// 	}
// }

// void loop() {
// 	// timer.tick();

// 	// getImage();
// 	// oledPrintVariable(0, 2, rxBuffer[0], "int", 3);
// 	// oledPrintVariable(19, 2, rxBuffer[1], "int", 3);
// 	// oledPrintVariable(38, 2, rxBuffer[2], "int", 3);
// 	// oledPrintVariable(57, 2, rxBuffer[3], "int", 3);
// 	// oledPrintVariable(76, 2, rxBuffer[4], "int", 3);
// 	// oledPrintVariable(95, 2, rxBuffer[5], "int", 3);
// 	// oledPrintVariable(0, 3, rxBuffer[6], "int", 3);
// 	// oledPrintVariable(19, 3, rxBuffer[7], "int", 3);
// 	// oledPrintVariable(38, 3, rxBuffer[8], "int", 3);
// 	// oledPrintVariable(57, 3, rxBuffer[9], "int", 3);
// 	// oledPrintVariable(76, 3, rxBuffer[10], "int", 3);
// 	// oledPrintVariable(95, 3, rxBuffer[11], "int", 3);
// 	// delay(1000);
// }