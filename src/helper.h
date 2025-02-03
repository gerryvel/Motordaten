#ifndef _HELPER_H_
#define _HELPER_H_

/**
 * @file helper.h
 * @author Gerry Sebb
 * @brief Hilfsfunktionen
 * @version 1.1
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include <stdio.h>
#include <time.h>
#include <Arduino.h>
#include <LITTLEFS.h>
#include <FS.h>
#include <Wire.h>
#include <WiFi.h>
#include "configuration.h"
#include <ArduinoJson.h>
#include <Preferences.h>

void ShowTime(){
	time_t now = time(NULL);
	struct tm tm_now;
	localtime_r(&now, &tm_now);
	char buff[100];
	strftime(buff, sizeof(buff), "%d-%m-%Y %H:%M:%S", &tm_now);
	printf("Zeit: %s\n", buff);
}

/** Freie Speichergroesse aller 5s lesen */
void freeHeapSpace(){
	static unsigned long last = millis();
	if (millis() - last > 5000) {
		last = millis();
		Serial.printf("\n[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
	}
}

/** Ausgabe WIFI Parameter und Netzwerk scannen */
void WiFiDiag(void) {
  Serial.println("\nWifi-Diag:");
  AP_IP = WiFi.softAPIP();
  CL_IP = WiFi.localIP();
  Serial.print("AP IP address: ");
  Serial.println(AP_IP.toString());
  Serial.print("Client IP address: ");
  Serial.println(CL_IP.toString());
  WiFi.printDiag(Serial);
  Serial.print("\nScan AP's "); 
  {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) 
        {
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          delay(10);
        }
    }
  }
}

/***************************** Filesystem **************************/

/**
 * @brief LittleFS, Dateien auflisten
 * 
 * @param fs 
 * @param dirname 
 * @param levels 
 */

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

/**
 * @brief Konfiguration aus Json-Datei lesen
 * 
 * @param filename 
 */

void readConfig(String filename) {
	JsonDocument testDocument;
	File configFile = LittleFS.open(filename);
	if (configFile)
	{
		Serial.println("opened config file");
		DeserializationError error = deserializeJson(testDocument, configFile);

		// Test if parsing succeeds.
		if (error)
		{
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.f_str());
			return;
		}

		Serial.println("deserializeJson ok");
		{
			Serial.println("Lese Daten aus Config - Datei");
			strcpy(tAP_Config.wAP_SSID, testDocument["SSID"] | "Motordaten");
			strcpy(tAP_Config.wAP_IP, testDocument["IP"] | "192.168.15.30");
			strcpy(tAP_Config.wAP_Password, testDocument["Password"] | "12345678");
			strcpy(tAP_Config.wMotor_Offset, testDocument["MotorOffset"] | "0.0");
      strcpy(tAP_Config.wCoolant_Offset, testDocument["CoolantOffset"] | "0.0");
      strcpy(tAP_Config.wFuellstandmax, testDocument["Fuellstandmax"] | "0.0");
		}
		configFile.close();
		Serial.println("Config - Datei geschlossen");
	}

	else
	{
		Serial.println("failed to load json config");
	}
}

/**
 * @brief Webseiten Eingabe in Json-Datei schreiben
 * 
 * @param json 
 * @return true 
 * @return false 
 */

bool writeConfig(String json)
{
	Serial.println(json);

	Serial.println("neue Konfiguration speichern");

	File configFile = LittleFS.open("/config.json", FILE_WRITE);
	if (configFile)
	{
		Serial.println("Config - Datei öffnen");
		File configFile = LittleFS.open("/config.json", FILE_WRITE);
		if (configFile)
		{
			Serial.println("Config - Datei zum Schreiben geöffnet");
			JsonDocument testDocument;
			Serial.println("JSON - Daten übergeben");
			DeserializationError error = deserializeJson(testDocument, json);
			// Test if parsing succeeds.
			if (error)
			{
				Serial.print(F("deserializeJson() failed: "));
				Serial.println(error.f_str());
				// bei Memory - Fehler den <Wert> in StaticJsonDocument<200> testDocument; erhöhen
				return false;
			}
			Serial.println("Konfiguration schreiben...");
			serializeJson(testDocument, configFile);
			Serial.println("Konfiguration geschrieben...");

			// neue Config in Serial ausgeben zur Kontrolle
			serializeJsonPretty(testDocument, Serial);

			Serial.println("Config - Datei geschlossen");
			configFile.close();
		}
	}
	return true;
}

/***************************** I2C Bus **************************/
/** I2C Bus auslesen, alle Geräte mit Adresse ausgegeben */

void I2C_scan(void){
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) 
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
      {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
      {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
}

/**
 * @brief WIFI Status lesen
 * 
 * @param Status 
 * @return String 
 */

String sWifiStatus(int Status)
{
  switch(Status){
    case WL_IDLE_STATUS:return "Warten";
    case WL_NO_SSID_AVAIL:return "Keine SSID vorhanden";
    case WL_SCAN_COMPLETED:return "Scan komlett";
    case WL_CONNECTED:return "Verbunden";
    case WL_CONNECT_FAILED:return "Verbindung fehlerhaft";
    case WL_CONNECTION_LOST:return "Verbindung verloren";
    case WL_DISCONNECTED:return "Nicht verbunden";
    default:return "unbekannt";
  }
}

/**
 * @brief Convert string to char
 * 
 * @param command 
 * @return char* 
 */

char* toChar(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
    else{
      return 0;
    }
}


#endif  