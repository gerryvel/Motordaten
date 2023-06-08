/*
    Name:       bootsdaten.ino
    Created:	22.10.2020
	Update: 	08.04.2023
    Author:     astec-PG\gerryadmin
*/

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "BoardInfo.h"
#include "configuration.h"
#include "helper.h"
#include <esp.h>
#include <ESPmDNS.h>
#include <ESP_WiFi.h>
#include <ESPAsyncWebServer.h>
#include "LED.h"
#include <SparkFun_MMA8452Q.h>
#include <Wire.h>
#include "Analog.h"
#include <LittleFS.h>
#include "NMEA0183Telegram.h"

// Set web server port number to 80
AsyncWebServer server(80);

// Info Board for HTML-Output
String sBoardInfo;
BoardInfo boardInfo;

//Gyro
MMA8452Q mma;

//Variables for website
String sCL_Status = sWifiStatus(WiFi.status());
String replaceVariable(const String& var)
{
	if (var == "sKraengung")return String(fKraengung,1);
	if (var == "sGaugeKraengung")return String(fGaugeKraengung, 1);
	if (var == "sSStellung")return String(fSStellung,1);
	if (var == "sAbsTief")return String(fAbsTief,1);
	if (var == "sSTBB")return sSTBB;
	if (var == "sBoardInfo")return sBoardInfo;
	if (var == "sFS_Space")return String(LittleFS.usedBytes());
	if (var == "sAP_IP")return String(tAP_Config.wAP_IP);
  	if (var == "sAP_Clients")return String(WiFi.softAPgetStationNum());
  	if (var == "sCL_Addr")return WiFi.localIP().toString();
  	if (var == "sCL_Status")return String(sCL_Status);
  	if (var == "sBMP_Status")return String(sBMP_Status);
	if (var == "CONFIGPLACEHOLDER")return processor(var);
  	return "NoVariable";
}

String processor(const String& var)
{
	if (var == "CONFIGPLACEHOLDER")
	{
		String buttons = "";
		buttons += "<form onSubmit = \"event.preventDefault(); formToJson(this);\">";
		buttons += "<p class=\"CInput\"><label>SSID </label><input type = \"text\" name = \"SSID\" value=\"";
		buttons += tAP_Config.wAP_SSID;
		buttons += "\"/></p>";
		buttons += "<p class=\"CInput\"><label>IP </label><input type = \"text\" name = \"IP\" value=\"";
		buttons += tAP_Config.wAP_IP;
		buttons += "\"/></p>";
		buttons += "<p class=\"CInput\"><label>Password </label><input type = \"text\" name = \"Password\" value=\"";
		buttons += tAP_Config.wAP_Password;
		buttons += "\"/></p>";
		buttons += "<p class=\"CInput\"><label>Kieloffset </label><input type = \"text\" name = \"Kiel_Offset\" value=\"";
		buttons += tAP_Config.wKiel_Offset;
		buttons += "\"/> cm</p>";
		buttons += "<p><input type=\"submit\" value=\"Speichern\"></p>";
		buttons += "</form>";
		return buttons;
	}
	return String();
}

//NMEA 0183 Stream
WiFiUDP udp;  // Create UDP instance

void SendNMEA0183Message(String var) {

  udp.beginPacket(udpAddress, udpPort);   // Send to UDP
  udp.println(var);
  udp.endPacket();
}


// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(115200);

	//Filesystem prepare for Webfiles
	if (!LittleFS.begin(true)) {
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
	Serial.println("Speicher LittleFS benutzt:");
	Serial.println(LittleFS.usedBytes());

	File root = LittleFS.open("/");
  	listDir(LittleFS, "/", 3);
	//file exists, reading and loading config file
	readConfig("/config.json");
	AP_SSID = tAP_Config.wAP_SSID;
	AP_PASSWORD = tAP_Config.wAP_Password;
	fKielOffset = atof(tAP_Config.wKiel_Offset);

  	freeHeapSpace();

	sBoardInfo = boardInfo.ShowChipIDtoString();

	pinMode(iMaxSonar, INPUT);

	// I2C
  	Wire.begin(I2C_SDA, I2C_SCL);
  	I2C_scan();

	//LED
  	LEDInit();
  	LEDoff();
	
	// Boardinfo	
  	sBoardInfo = boardInfo.ShowChipIDtoString();

	//MMA
	bool MMAbegin = mma.init();

	switch (MMAbegin) {
	case 0:
		Serial.println("\nMMA could not start!");
		sBMP_Status = "Keinen Sensor gefunden!";
		bI2C_Status = 0;
		break;
	case 1:
		Serial.println("\nMMA found!");
		mma.init(SCALE_2G);
		Serial.print("Range = "); Serial.print(2 << mma.available());
		Serial.println("G");   
		sBMP_Status = "Sensor gefunden!"; 
		bI2C_Status = 1;          
	}
	
	//WIFI
	if (!WiFi.setHostname(HostName))
		Serial.println("\nSet Hostname success");
	else
		Serial.println("\nSet Hostname not success");

	//WiFiServer AP starten
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(AP_SSID, AP_PASSWORD);
	delay(1000);
	if (WiFi.softAPConfig(IP, Gateway, NMask))
		Serial.println("\nIP config success");	
	else
		Serial.println("IP config not success");	

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP configured with address: ");
	Serial.println(myIP);
	

	if (!MDNS.begin(HostName)) {
		Serial.println("Error setting up MDNS responder!");
		while (1) {
			delay(1000);
		}
	}
	Serial.println("mDNS responder started");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/index.html", String(), false, replaceVariable);
	});
	server.on("/system.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/system.html", String(), false, replaceVariable);
	});
	server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/settings.html", String(), false, replaceVariable);
	});
	server.on("/ueber.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/ueber.html", String(), false, replaceVariable);
	});
	server.on("/gauge.min.js", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/gauge.min.js");
	});
	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(LittleFS, "/style.css", "text/css");
	});
	server.on("/settings.html", HTTP_POST, [](AsyncWebServerRequest *request)
	{
		int count = request->params();
		Serial.printf("Anzahl: %i\n", count);
		for (int i = 0;i < count;i++)
		{
			AsyncWebParameter* p = request->getParam(i);
			Serial.print("PWerte von der Internet - Seite: ");
			Serial.print("Param name: ");
			Serial.println(p->name());
			Serial.print("Param value: ");
			Serial.println(p->value());
			Serial.println("------");
			// p->value in die config schreiben
			writeConfig(p->value());
		}
		request->send(200, "text/plain", "Daten gespeichert");
	});



	// Start TCP (HTTP) server
	server.begin();
	Serial.println("TCP server started");

	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);



	ArduinoOTA
		.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else
			type = "filesystem";
		Serial.println("start updating " + type);
	})
		.onEnd([]() {
		Serial.println("\nEnd");
	})
		.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	})
		.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});

	ArduinoOTA.setHostname(HostName);
	ArduinoOTA.begin();
}

// Add the main program code into the continuous loop() function
void loop()
{
	//Wifi variables
	bConnect_CL = WiFi.status() == WL_CONNECTED ? 1 : 0;
	//bAP_on = WiFi.enableAP(bAP_on);
	
	// LED visu Wifi
	LEDoff();
	LEDflash(LED(LEDBoard)); // Betrieb ok
	if (bI2C_Status == 0) LEDflash(LED(Red)); // Sensorfehler

// OTA	
	ArduinoOTA.handle();
	delay(1000);

// Status AP 
  Serial.printf("Stationen mit AP verbunden = %d\n", WiFi.softAPgetStationNum());
  Serial.printf("Soft-AP IP address = %s\n", WiFi.softAPIP().toString());
  sCL_Status = sWifiStatus(WiFi.status());

// read MMA, use only x for Kraengung (Pitch)
	Serial.printf("X: %f °\n", mma.getX() / 11.377);
	fKraengung = mma.getX() / 11.377;
	fKraengung = (abs(fKraengung));
	fGaugeKraengung = mma.getX() / 11.377;
	Serial.printf("Y: %f °\n", mma.getY() / 11.377);
	Serial.printf("Z: %f °\n", mma.getZ() / 11.377);

	// Direction Kraengung
	if (mma.getX() < -1)
		sSTBB = "Backbord";
	else sSTBB = "Steuerbord";
	Serial.printf("Kraengung nach: %s %f °\n", sSTBB, fKraengung);

	SendNMEA0183Message(sendXDR());

	// read I2C Orientation Sensor
	bool bSFM = 0; // SensorFalschMontiert
	uint8_t Orientation = mma.readPL();
	switch (Orientation) {
	case PORTRAIT_U: sOrient = "Oben";break;
	case PORTRAIT_D: sOrient = "Unten"; 
		bSFM = 1;
		Serial.print("Sensor falsch montiert\n");
		break;
	case LANDSCAPE_R: sOrient = "Rechts";break;		
	case LANDSCAPE_L: sOrient = "Links";break;		
	case LOCKOUT: sOrient = "Horizontal";
		bSFM = 1;
		Serial.print("Sensor falsch montiert\n");
		break;
	}
	Serial.printf("Orientation: %s\n", sOrient);

	// LED Kraengung
	if (bSFM == 0 && bI2C_Status == 1)
	{
		if (sSTBB == "Backbord")
		{
			digitalWrite(LED(Red), HIGH);
		}
		else 
		{
			digitalWrite(LED(Green), HIGH);
		}
	}
	else if (bI2C_Status == 1)
	{
		digitalWrite(LED(Blue), HIGH);
	}	

//AI Distance-Sensor
	iDistance = analogRead(iMaxSonar);
	Serial.printf("Analogeingang: %i\n", iDistance);
	int Err = 0;
	fSStellung = analogInScale(iDistance, 3800, 300, 7.0, 80.0, Err);
	Serial.printf("Schwert: %f cm\n", fSStellung);
	fAbsTief = fSStellung + fKielOffset;
	Serial.printf("Tiefgang: %f cm\n", fAbsTief);
	delay(500);

// config von LittleFS lesen, Kieloffset schreiben
	readConfig("/config.json");
	fKielOffset = atof(tAP_Config.wKiel_Offset);

	freeHeapSpace();
}
