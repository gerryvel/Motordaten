#ifndef __configuration__H__
#define __configuration__H__

#include <Arduino.h>

// Configuration N2k
#define ESP32_CAN_TX_PIN GPIO_NUM_4  // Set CAN TX port to 4 
#define ESP32_CAN_RX_PIN GPIO_NUM_5  // Set CAN RX port to 5
#define N2K_SOURCE 15
int NodeAddress;                     // To store Last Node Address
uint8_t chipid[6];
uint32_t id = 0;
int i = 0;
#define TempSendOffset 0
#define TankSendOffset 40
#define RPM_SendOffset 80
#define BatterySendOffset 100
#define SlowDataUpdatePeriod 1000  // Time between CAN Messages sent


//Configuration Web Page 
#define PAGE_REFRESH 10 // x Sec.
#define WEB_TITEL "Motordaten"

//Configuration mit Webinterface
struct Web_Config
{
	char wAP_IP[20];
	char wAP_SSID[64];
	char wAP_Password[12];
	char wTemp_Offset[5];
};
Web_Config tAP_Config;

//Configuration AP 
#define HostName        "Motordaten"
const int   channel        = 10;                // WiFi Channel number between 1 and 13
const bool  hide_SSID      = false;             // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
const int   max_connection = 4;                 // Maximum simultaneous connected clients on the AP

// Variables for WIFI-AP
IPAddress IP = IPAddress(192, 168, 15, 30);
IPAddress Gateway = IPAddress(192, 168, 15, 30);
IPAddress NMask = IPAddress(255, 255, 255, 0);
IPAddress AP_IP;
String AP_SSID = "";
String AP_PASSWORD  = "";
IPAddress CL_IP;
IPAddress SELF_IP;
String sAP_Station = "";

//Configuration Client (Network Data Windsensor)
#define CL_SSID      "NoWa"					//Windmesser
#define CL_PASSWORD  "12345678"		
int iSTA_on = 0;                            // Status STA-Mode
int bConnect_CL = 0;
bool bClientConnected = 0;

//Confuration Sensors I2C
#define I2C_SDA 21                      //Standard 21
#define I2C_SCL 22                      //Standard 22
#define SEALEVELPRESSURE_HPA (1013.25)  //1013.25
float fbmp_temperature = 0;
float fbmp_pressure = 0;
float fbmp_altitude = 0;
String sI2C_Status = "";
bool bI2C_Status = 0;

const int iMaxSonar = 35;			//Analoginput 
int iDistance = 0;

// Global Data Motordaten
float FuelLevel = 0;
float ExhaustTemp = 0;
float EngineRPM = 0;
float BordSpannung = 0;
String sOneWire_Status = "";

// Data wire for teperature (Dallas DS18B20) 
#define ONE_WIRE_BUS 13			//GPIO 13 on the ESP32

// Variables Website
float fDrehzahl = 0;
float fGaugeDrehzahl = 0;
float fBordSpannung = 0;
float fTemp = 0;
float fTempOffset = 0;
String sSTBB = "";
String sOrient = "";

//Definiton NMEA0183 MWV
double dMWV_WindDirectionT = 0;
double dMWV_WindSpeedM = 0;
double dVWR_WindDirectionM = 0;
double dVWR_WindAngle = 0;
double dVWR_WindSpeedkn = 0;
double dVWR_WindSpeedms = 0;

//Configuration NMEA0183
#define SERVER_HOST_NAME "192.168.4.1"		//"192.168.76.34"
#define TCP_PORT 6666						//6666
#define DNS_PORT 53

//Variable NMEA 0183 Stream
const char *udpAddress = "192.168.30.255"; // Set network address for broadcast
const int udpPort = 4444;                 // UDP port

#endif  
