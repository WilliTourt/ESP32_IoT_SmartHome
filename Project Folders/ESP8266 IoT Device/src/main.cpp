
/*******************************************************
 * ESP8266 IoT Device Terminal
 * Copyright (c) 2025 WilliTourt 2944925833@qq.com
*******************************************************/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include <extDevice.h>

IPAddress IP(192, 168, 1, 101);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

ESP8266WiFiMulti wifiMulti;
 
ESP8266WebServer server(80);

extDevice led_err(4);
extDevice led_ok(5);

extDevice relaySW(14);

// void handleUserRequest();
void handleLightCtrl();
void handleLightTask();
void displayMode();

// bool handleFileRead(String resource);
// String getContentType(String filename);

void setup() {
	pinMode(LED_BUILTIN, INPUT);
	digitalWrite(LED_BUILTIN, LOW);

	if (!WiFi.config(IP, gateway, subnet, dns)) {
    	led_err.on();
  	}

	wifiMulti.addAP("WilliTourt", "williamtourtei");
	// wifiMulti.addAP("CMCC_R7cY", "a9asfeba");
	// wifiMulti.addAP("Ciallo~☆", "williamtourtei");

	while(wifiMulti.run() != WL_CONNECTED) {
		delay(10);
		led_err.on();
		delay(200);
		led_err.off();
	}

	if(!LittleFS.begin()) {
		led_err.on();
	}

	server.on("/light", handleLightCtrl);
	server.on("/lightTask", handleLightTask);
	server.on("/display", displayMode);
	// server.on("/esp8266.html", handleUserRequest);	//Homepage
	// server.onNotFound(handleUserRequest);

	server.begin();

	led_ok.on();
}

void loop() {

	server.handleClient();
	
}

void handleLightCtrl() {
	String state;

	(relaySW.read() == HIGH) ? (relaySW.off()) : (relaySW.on());
	state = (relaySW.read() == HIGH) ? ("on") : ("off");

	server.send(200, "text/plain", state);
}

void handleLightTask() {
	String state = server.arg("state");

	if (state == "on") {
		relaySW.on();
	} else if (state == "off") {
		relaySW.off();
	}

	server.send(200, "text/plain", state);
}

void displayMode() {
	String state = server.arg("state");
	if (state == "off") {
		led_err.sleep();
		led_ok.sleep();
	} else if (state == "on") {
		led_err.wake();
		led_ok.wake();
		led_ok.on();
	}
	server.send(200, "text/plain", "ok");
}

// void handleUserRequest() {
// 	String reqResource = server.uri();
// 	bool fileFound = handleFileRead(reqResource);

// 	if (!fileFound) {
// 		server.sendHeader("Location", "/404.html");
// 		server.send(303);
// 	}
// }

// bool handleFileRead(String resource) {

// 	if (resource.endsWith("/")) {
// 		resource = "/esp8266.html";
// 	} 
	
// 	String contentType = getContentType(resource);
	
// 	if (LittleFS.exists(resource)) {
// 		File file = LittleFS.open(resource, "r");
// 		server.streamFile(file, contentType);
// 		file.close();
// 		return true;
// 	}

// 	return false;
// }

// //获取文件类型
// String getContentType(String filename) {
// 	if(filename.endsWith(".htm")) return "text/html";
// 	else if(filename.endsWith(".html")) return "text/html";
// 	else if(filename.endsWith(".css")) return "text/css";
// 	else if(filename.endsWith(".js")) return "application/javascript";
// 	else if(filename.endsWith(".png")) return "image/png";
// 	else if(filename.endsWith(".gif")) return "image/gif";
// 	else if(filename.endsWith(".jpg")) return "image/jpeg";
// 	else if(filename.endsWith(".ico")) return "image/x-icon";
// 	else if(filename.endsWith(".xml")) return "text/xml";
// 	else if(filename.endsWith(".pdf")) return "application/x-pdf";
// 	else if(filename.endsWith(".zip")) return "application/x-zip";
// 	else if(filename.endsWith(".gz")) return "application/x-gzip";
// 	return "text/plain";
// }
