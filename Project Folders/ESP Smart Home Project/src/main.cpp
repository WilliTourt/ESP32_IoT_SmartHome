
/*******************************************************
 * ESP32-C3 FH4 Ctrl Platform
 * Copyright (c) 2025 WilliTourt 2944925833@qq.com
*******************************************************/
 
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <WiFiUdp.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiClient.h>

#include <SPIFFS.h>

#include <ExternalDevices.h>
#include <arduino-timer.h>
#include <music.h>

WiFiMulti wifiMulti;
WebServer server(80);
WiFiClient client;

WiFiUDP ntp;

const char* ntpServer = "pool.ntp.org"; 		//NTP服务器地址
NTPClient ntpClient(ntp, ntpServer, 3600*8); 	//UTC+8

typedef struct { 		//定时任务结构体
	String device;
	String action;
	String timeStr;
} TimerTask;

TimerTask timerTask;
TimerTask timerTask2;

struct DeviceInfo { 	//设备数据结构体
	String bedroomlightStatus;
	String livingroomLightStatus;
	String diningroomLightStatus;
	String doorStatus;
	float dhtTemp;
	float dhtHumi;
} deviceInfo;

// bool doorState = 0;			//门状态


bool taskScheduled = false; 	//标记是否有任务
bool taskExecuted = false; 		//标记是否执行了任务

bool taskScheduled2 = false;
bool taskExecuted2 = false;

bool alarmScheduled = false; 	//标记是否有闹钟
bool alarmExecuted = false; 	//标记是否执行了闹钟
bool alarmBeeping = false; 		//标记闹钟是否正在响

String scheduledTask; 			//存放定时任务
String scheduledTask2;

String scheduledAlarm; 			//存放闹钟时间

String password = "123456";		//初始密码123456
String enteredPassword = "";

bool displayMode = true;		//显示模式

void handleUserRequest();
void handleNtpRequest();
void handleDeviceInfoRequest();
void handleANTRequest();

void handleTimer();
void handleTimerTask();
void chkTimer();

void handleTimer2();
void handleTimerTask2();
void chkTimer2();

void handleAlarm();
void chkAlarm();
void beepAlarm();

void handlePassword();

void handleLight(String device);
void handleDHT();

void handleDisplayMode();
void displayToggle(bool state);
void sendToggleCmd(String state, String espDevice);

void oledPrintIP();
void oledPrintReconnect();
bool handleFileRead(String resource);
String getContentType(String filename);

Timer<1, micros> timer;

// String blog = "http://www.woodmerlov.fun/";

IPAddress esp8266_1_IP(192, 168, 1, 101);
IPAddress esp8266_2_IP(192, 168, 1, 102);
IPAddress esp8266_3_IP(192, 168, 1, 103);

IPAddress IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);


bool infoUpdate(void *) {

	server.handleClient();
	ntpClient.update();
	chkTimer();
	chkTimer2();
	chkAlarm();

   	return true;
}

void setup() {

	// Serial.begin(115200); //For Debugging

	initExtDevices();

	oledPrintText(27, 0, "ESP32 Server", 8);
	oledPrintText(3, 1, " Initializing...", 8);

	delay(20);
	oledPrintText(3, 2, " WiFi connecting...", 8);

	if (!WiFi.config(IP, gateway, subnet, dns)) {
        oledPrintText(3, 2, ">> WiFi config failed!", 8);
		digitalWrite(E, HIGH);
    } else {
        oledPrintText(3, 2, " WiFi config OK.   ", 8);
    }

	wifiMulti.addAP("WilliTourt", "williamtourtei");
	// wifiMulti.addAP("CMCC_R7cY", "a9asfeba");
	// wifiMulti.addAP("Ciallo~☆", "williamtourtei");

	oledPrintText(3, 3, " WiFi connecting...", 8);

	while (wifiMulti.run() != WL_CONNECTED) {
		delay(10);
		digitalWrite(ERROR, HIGH);
		delay(200);
		digitalWrite(ERROR, LOW);
	}

	oledPrintText(3, 3, " WiFi connected.   ", 8);

	if (SPIFFS.begin()) {
		oledPrintText(3, 4, " SPIFFS Started.", 8);
	} else {
		oledPrintText(3, 4, ">> SPIFFS ERROR!", 8);
		digitalWrite(E, HIGH);
	}

	server.on("/bedroomLight", [&]() { handleLight("bedroomLight"); });					//JS handleLight(1);
	server.on("/livingroomLight", [&]() { handleLight("livingroomLight"); });	//JS handleLight(2);
	server.on("/diningroomLight", [&]() { handleLight("diningroomLight"); });	//JS handleLight(3);
	// server.on("/door", handleDoor);					//JS handleDoor();
	// server.on("/curtain", handleCurtain);			//JS handleCurtain();
	server.on("/dht", handleDHT);					//JS fetchDHT();

	server.on("/PWD", handlePassword);				//JS setPWD();

	server.on("/alarm", handleAlarm);				//JS alarmTask();  chkAlarm();
	server.on("/timer", handleTimer);				//JS timerTask();  chkScheduledTask();
	server.on("/timer2", handleTimer2);				//JS timerTask2();  chkScheduledTask2();
	server.on("/aNt", handleANTRequest);			//JS fetchANT();
	server.on("/time", handleNtpRequest);			//JS syncTime();

	server.on("/device", handleDeviceInfoRequest); 	//JS getDeviceInfo();
	server.on("/esp32.html", handleUserRequest);	//esp32.html Homepage
	server.on("/display", handleDisplayMode);		//JS setDisplayMode();

	server.onNotFound(handleUserRequest);			//404.html

	server.begin();

	oledPrintText(3, 5, " HTTP server started.", 8);

	ntpClient.begin();
	oledPrintText(3, 6, " NTP client started.", 8);

	if (digitalRead(E) == HIGH) {
		oledPrintText(3, 7, ">> INIT FAILED!", 8);
		for (int i = 0; i < 3; i++) {
			tone(buzzer, 1000, 100);
			delay(200);
		}
		while (1) {
			digitalWrite(E, HIGH);
			delay(1000);
			digitalWrite(E, LOW);
			delay(1000);
		}
	} else {
		oledPrintText(3, 7, ">> Init Success!", 8);
	}

	delay(2000);
	digitalWrite(OK, HIGH);
	tone(buzzer, 1000, 50);
	delay(100);
	tone(buzzer, 1300, 50);
	delay(100);
	tone(buzzer, 1600, 50);

	oledPrintIP();

	timer.every(201, infoUpdate);

}

void loop() {

	timer.tick();

	// server.handleClient();
	// ntpClient.update();
	// chkTimer();
	// chkAlarm();

	beepAlarm();

	if (wifiMulti.run() != WL_CONNECTED) {
		while (wifiMulti.run() != WL_CONNECTED) {
			oledPrintReconnect();
			digitalWrite(ERROR, HIGH);
			delay(200);
			digitalWrite(ERROR, LOW);
			delay(200);
		}
		oledPrintIP();
	}

}


/* HTTP Request Handlers ***********************************************************************************************/


void handleLight(String device) {
    IPAddress targetIP;
	String route;

    if (device == "bedroomLight") {
        targetIP = esp8266_1_IP;
		route = "/light";
    } else if (device == "livingroomLight") {
        targetIP = esp8266_2_IP;
		route = "/light1";
    } else if (device == "diningroomLight") {
        targetIP = esp8266_2_IP;
		route = "/light2";
    }

    if (client.connect(targetIP, 80)) {
        client.println("GET " + route + " HTTP/1.1");
        client.println("Host: " + targetIP.toString());
        client.println("Connection: close");
        client.println();

		unsigned long startTime = millis();
		String response;
		bool responseReceived = false;

		while (client.connected() && millis() - startTime < 700) {
			delay(10);
			if (client.available()) {
				String line = client.readStringUntil('\n');
				if (!responseReceived) {
					if (line.length() == 1 && line[0] == '\r') {
						responseReceived = true;
					}
				} else {
					response += line;
				}
			}
		}

		client.stop();
		response.trim();
			
		if (device == "bedroomLight") {
			deviceInfo.bedroomlightStatus = response;
		} else if (device == "livingroomLight") {
			deviceInfo.livingroomLightStatus = response;
		} else if (device == "diningroomLight") {
			deviceInfo.diningroomLightStatus = response;
		}

		server.send(200, "text/plain", response);
		digitalWrite(ERROR, LOW);
		return;
    } else {
        server.send(500, "text/plain", "Connection Failed");
		digitalWrite(ERROR, HIGH);
        return;
    }
}

// void handleDoor() {

// 	if (doorState) {
// //
// 		deviceInfo.doorStatus = "closed";
// 	} else {
// //
// 		deviceInfo.doorStatus = "opened";
// 	}

// 	doorState = !doorState;

// 	server.send(200, "text/plain", deviceInfo.doorStatus);
// }

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

//处理设备信息获取请求
void handleDeviceInfoRequest() {
	String info = deviceInfo.bedroomlightStatus + "," + 
				  deviceInfo.livingroomLightStatus + "," + 
				  deviceInfo.diningroomLightStatus + "," + 
				  deviceInfo.doorStatus + "," + 
				  String(deviceInfo.dhtTemp) + "," + 
				  String(deviceInfo.dhtHumi) + "," + 
				  (String)displayMode;

	server.send(200, "text/plain", info);
}



void handleAlarm() {
	alarmExecuted = false;
	scheduledAlarm = server.arg("plain"); 	//网页发来的闹钟时间(hh:mm:ss)
	alarmScheduled = true;
	server.send(200, "text/plain", "Alarm Scheduled");
}

//检查闹钟是否到期
void chkAlarm() {
	if (alarmScheduled) {
		if (ntpClient.getFormattedTime() == scheduledAlarm) {
			alarmBeeping = true;
			alarmExecuted = true;
			alarmScheduled = false;
		}
	}
}

//处理定时器请求
void handleTimer() {
	taskExecuted = false;
	String command = server.arg("plain");

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

	if (device == "bedroomLight" || device == "bed") {
		device = "Bedroom light";
	} else if (device == "livingroomLight" || device == "living") {
		device = "Living room light";
	} else if (device == "diningroomLight" || device == "dining") {
		device = "Dining room light";
	}

	scheduledTask = device + " " + action + " at " + timeStr;

	server.send(200, "text/plain", "Timer Scheduled");
}

void handleTimer2() {
	taskExecuted2 = false;
	String command = server.arg("plain");

	int actionIndex = command.indexOf("-");
	int timeIndex = command.indexOf("-", actionIndex + 1);

	String device = command.substring(0, actionIndex);
	String action = command.substring(actionIndex + 1, timeIndex);
	String timeStr = command.substring(timeIndex + 1);
	timeStr.replace("-", ":");

	timerTask2.device = device;
	timerTask2.action = action;
	timerTask2.timeStr = timeStr;

	taskScheduled2 = true;

	if (device == "bedroomLight" || device == "bed") {
		device = "Bedroom light";
	} else if (device == "livingroomLight" || device == "living") {
		device = "Living room light";
	} else if (device == "diningroomLight" || device == "dining") {
		device = "Dining room light";
	}

	scheduledTask2 = device + " " + action + " at " + timeStr;

	server.send(200, "text/plain", "Timer 2 Scheduled");
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

void chkTimer2() {
	if (taskScheduled2) {
		if (ntpClient.getFormattedTime() == timerTask2.timeStr) {
			handleTimerTask2();
			taskScheduled2 = false;
		}
	}
}

//处理定时器任务
void handleTimerTask() {
    IPAddress targetIP;
	String route;
	String state;

    if (timerTask.device == "bedroomLight" || timerTask.device == "bed") {
        targetIP = esp8266_1_IP;
		route = "/lightTask";
	} else if (timerTask.device == "livingroomLight" || timerTask.device == "living") {
        targetIP = esp8266_2_IP;
		route = "/lightTask1";
    } else if (timerTask.device == "diningroomLight" || timerTask.device == "dining") {
        targetIP = esp8266_2_IP;
		route = "/lightTask2";
	}

	if (timerTask.action == "on" || timerTask.action == "open") {
		state = "on";
	} else if (timerTask.action == "off" || timerTask.action == "close") {
		state = "off";
	}

	if (client.connect(targetIP, 80)) {
		client.println("GET " + route + "?state=" + state + " HTTP/1.1");	// GET /light?state=on HTTP/1.1
		client.println("Host: " + targetIP.toString());
		client.println("Connection: close");
		client.println();

		unsigned long startTime = millis();
		String response;
		bool responseReceived = false;

		while (client.connected() && millis() - startTime < 700) {
			delay(10);
			if (client.available()) {
				String line = client.readStringUntil('\n');
				if (!responseReceived) {
					if (line.length() == 1 && line[0] == '\r') {
						responseReceived = true;
					}
				} else {
					response += line;
				}
			}
		}

		client.stop();
		response.trim();

		if (timerTask.device == "bedroomLight" || timerTask.device == "bed") {
			deviceInfo.bedroomlightStatus = response;
		} else if (timerTask.device == "livingroomLight" || timerTask.device == "living") {
			deviceInfo.livingroomLightStatus = response;
		} else if (timerTask.device == "diningroomLight" || timerTask.device == "dining") {
			deviceInfo.diningroomLightStatus = response;
		}

		server.send(200, "text/plain", response);
		return;
	
	} else {
		server.send(500, "text/plain", "Connection Failed");
		digitalWrite(ERROR, HIGH);
		return;
	}

// 	if (timerTask.device == "door") {
// 		if (timerTask.action == "on" || timerTask.action == "open" || timerTask.action == "up") {

// 			deviceInfo.doorStatus = "opened";
// 		} else if (timerTask.action == "off" || timerTask.action == "close" || timerTask.action == "down") {

// 			deviceInfo.doorStatus = "closed";
// 		} else if (timerTask.action == "toggle") {
// 			if (doorState) {
// //
// 				deviceInfo.doorStatus = "closed";
// 			} else {
// //
// 				deviceInfo.doorStatus = "opened";
// 			}
// 			doorState = !doorState;
// 		}
// 	}
// 	/*else if {...}*/

	taskExecuted = true;

}

void handleTimerTask2() {
    IPAddress targetIP;
	String route;
	String state;

    if (timerTask2.device == "bedroomLight" || timerTask2.device == "bed") {
        targetIP = esp8266_1_IP;
		route = "/lightTask";
	} else if (timerTask2.device == "livingroomLight" || timerTask2.device == "living") {
        targetIP = esp8266_2_IP;
		route = "/lightTask1";
    } else if (timerTask2.device == "diningroomLight" || timerTask2.device == "dining") {
        targetIP = esp8266_2_IP;
		route = "/lightTask2";
	}

	if (timerTask2.action == "on" || timerTask2.action == "open") {
		state = "on";
	} else if (timerTask2.action == "off" || timerTask2.action == "close") {
		state = "off";
	}

	if (client.connect(targetIP, 80)) {
		client.println("GET " + route + "?state=" + state + " HTTP/1.1");	// GET /light?state=on HTTP/1.1
		client.println("Host: " + targetIP.toString());
		client.println("Connection: close");
		client.println();

		unsigned long startTime = millis();
		String response;
		bool responseReceived = false;

		while (client.connected() && millis() - startTime < 700) {
			delay(10);
			if (client.available()) {
				String line = client.readStringUntil('\n');
				if (!responseReceived) {
					if (line.length() == 1 && line[0] == '\r') {
						responseReceived = true;
					}
				} else {
					response += line;
				}
			}
		}

		client.stop();
		response.trim();

		if (timerTask2.device == "bedroomLight" || timerTask2.device == "bed") {
			deviceInfo.bedroomlightStatus = response;
		} else if (timerTask2.device == "livingroomLight" || timerTask2.device == "living") {
			deviceInfo.livingroomLightStatus = response;
		} else if (timerTask2.device == "diningroomLight" || timerTask2.device == "dining") {
			deviceInfo.diningroomLightStatus = response;
		}

		server.send(200, "text/plain", response);
		return;
	
	} else {
		server.send(500, "text/plain", "Connection Failed");
		digitalWrite(ERROR, HIGH);
		return;
	}

// 	if (timerTask.device == "door") {
// 		if (timerTask.action == "on" || timerTask.action == "open" || timerTask.action == "up") {

// 			deviceInfo.doorStatus = "opened";
// 		} else if (timerTask.action == "off" || timerTask.action == "close" || timerTask.action == "down") {

// 			deviceInfo.doorStatus = "closed";
// 		} else if (timerTask.action == "toggle") {
// 			if (doorState) {
// //
// 				deviceInfo.doorStatus = "closed";
// 			} else {
// //
// 				deviceInfo.doorStatus = "opened";
// 			}
// 			doorState = !doorState;
// 		}
// 	}
// 	/*else if {...}*/

	taskExecuted2 = true;

}

void handleANTRequest() {
    String info = "";

    if (!alarmExecuted && alarmScheduled) {
        info += scheduledAlarm + ",";
    } else {
        info += "0,";
    }

    if (!taskExecuted && taskScheduled) {
        info += scheduledTask + ",";
    } else {
        info += "0,";
    }

	if (!taskExecuted2 && taskScheduled2) {
        info += scheduledTask2;
    } else {
        info += "0";
    }

    server.send(200, "text/plain", info); // alarm,task,task2
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



//处理HTTP请求
void handleUserRequest() {
	String reqResource = server.uri();
	bool fileFound = handleFileRead(reqResource);

	if (!fileFound) {
		server.sendHeader("Location", "/404.html");
		server.send(303);
	}
}

void handleDisplayMode() {
	displayMode = (displayMode ? false : true);
	displayToggle(displayMode);
	server.send(200, "text/plain", (displayMode ? "on" : "off"));
}

void displayToggle(bool state) {
	if (state) {
		wakeUpMode();
		oledPrintIP();
		sendToggleCmd("on", "bedroom");	
		sendToggleCmd("on", "livingroom");
	} else {
		sleepMode();
		sendToggleCmd("off", "bedroom");
		sendToggleCmd("off", "livingroom");
	}
}

void sendToggleCmd(String state, String espDevice) {

	IPAddress targetIP;
    if (espDevice == "bedroom") {
        targetIP = esp8266_1_IP;
    } else if (espDevice == "livingroom") {
        targetIP = esp8266_2_IP;
    }

	bool responseReceived = false;
    if (client.connect(targetIP, 80)) {
        client.println("GET /display?state=" + state + " HTTP/1.1");
        client.println("Host: " + targetIP.toString());
        client.println("Connection: close");
        client.println();

        unsigned long startTime = millis();

        while (client.connected() && millis() - startTime < 700) { 	//700ms超时
            delay(10);
            if (client.available()) {
				String line = client.readStringUntil('\n');
				if (!responseReceived) {
					if (line.length() == 1 && line[0] == '\r') { 	//请求头结束
						responseReceived = true;
					}
				}
			}
        }

        client.stop();

	}

	if (responseReceived) {
		digitalWrite(ERROR, LOW);
	} else {
		digitalWrite(ERROR, HIGH);
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

void beepAlarm() {
	// uint16_t cnt = 25;
	pinMode(E, OUTPUT);

	if (alarmBeeping) {
		bool introed = false;
		while (alarmBeeping) {

			// digitalWrite(E, HIGH);
			// tone(buzzer, 2000, 300);
			// server.handleClient();
			// ntpClient.update();
			// chkTimer();
			// chkTimer2();		
			// delay(150);
			// digitalWrite(E, LOW);

			// for (int i = 0; i < cnt; i++) {
			// 	server.handleClient();
			// 	ntpClient.update();
			// 	chkTimer();
			// 	chkTimer2();
			// 	delay(150);
			// }

			// if (cnt > 2) {
			// 	cnt--;
			// }

			if (!introed) {
				for (uint8_t note = 0; note < 16; note++) {

					if (note == 0 || note == 4 || note == 8 || note == 12) {
						digitalWrite(E, HIGH);
					} else if (note == 2 || note == 6 || note == 10 || note == 14) {
						digitalWrite(E, LOW);
					}

					tone(buzzer, doodle_intro[note], BEAT_QUARTER);
					delay(BEAT_QUARTER);

					if (digitalRead(S) == LOW) {
						alarmBeeping = false;
						break;
					}

				}
				introed = true;
			}

			for (uint8_t note = 0; note < 64; note++) {

				if (note == 0 || note == 32) {
					digitalWrite(E, HIGH);
				} else if (note == 2 || note == 34) {
					digitalWrite(E, LOW);
				}

				tone(buzzer, doodle_main[note], BEAT_QUARTER);

				// if (doodle_main[note + 1] == 0 && doodle_main[note + 2] == 0 && doodle_main[note + 3] == 0) {
				// 	server.handleClient();
				// 	ntpClient.update();
				// 	chkTimer();
				// 	chkTimer2();
				// 	delay(BEAT_QUARTER - 3);
				// } else {
					delay(BEAT_QUARTER);
				// }

				if (digitalRead(S) == LOW) {
					alarmBeeping = false;
					break;
				}

			}

		}
		digitalWrite(E, LOW);
	}
}

/**
  * @brief  处理文件读取请求
  * @param  resource 资源地址
  * @retval 是否成功读取
  */
bool handleFileRead(String resource) {

	if (resource.endsWith("/")) {
		resource = "/esp32.html";
	}
	
	String contentType = getContentType(resource);  //获取文件类型
	
	if (SPIFFS.exists(resource)) {                  //如果访问的文件在SPIFFS中存在
		File file = SPIFFS.open(resource, "r");     //则打开该文件
		server.streamFile(file, contentType);		//返回给浏览器
		file.close();

		return true;
	}

	return false;
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
