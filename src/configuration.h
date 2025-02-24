#ifndef __configuration__H__
#define __configuration__H__

/**
 * @file configuration.h
 * @author Gerry Sebb
 * @brief Konfiguration für GPIO und Variable
 * @version 2.3
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <Arduino.h>
#include <Preferences.h>

// Versionierung
#define Version "V2.6 vom 23.02.2025"  // Version

/**
 * @brief Config NMEA2000
 * 
 */
#define ESP32_CAN_TX_PIN GPIO_NUM_4  // Set CAN TX port to 4 
#define ESP32_CAN_RX_PIN GPIO_NUM_5  // Set CAN RX port to 5
#define N2K_SOURCE 15
int NodeAddress;                     // To store Last Node Address
Preferences preferences;             // Nonvolatile storage on ESP32 - To store LastDeviceAddress
uint8_t chipid[6];
uint32_t id = 0;
int i = 0;
#define EngineSendOffset 0
#define TankSendOffset 40
#define RPMSendOffset 80
#define BatteryDCSendOffset 120
#define BatteryDCStatusSendOffset 160
#define SlowDataUpdatePeriod 1000  // Time between CAN Messages sent


//Configuration Website
#define PAGE_REFRESH 10 // x Sec.
#define WEB_TITEL "Motordaten"
String sHeapspace = "";

//Configuration mit Webinterface
struct Web_Config
{
	char wAP_IP[20];
	char wAP_SSID[64];
	char wAP_Password[12];
	char wMotor_Offset[6];
	char wCoolant_Offset[6];
	char wFuellstandmax[6];
	char wADC1_Cal[6];
	char wADC2_Cal[6];
};
Web_Config tAP_Config;

//Configuration AP 
#define HostName        "Motordaten"
const int   channel        = 10;                // WiFi Channel number between 1 and 13
const bool  hide_SSID      = false;             // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
const int   max_connection = 2;                 // Maximum simultaneous connected clients on the AP

// Variables for WIFI-AP
IPAddress IP = IPAddress(192, 168, 15, 30);
IPAddress Gateway = IPAddress(192, 168, 15, 30);
IPAddress NMask = IPAddress(255, 255, 255, 0);
const char* AP_SSID = "Motordaten";
const char* AP_PASSWORD  = "12345678";
IPAddress AP_IP;
IPAddress CL_IP;
IPAddress SELF_IP;
String sAP_Station = "";

//Configuration Client (Network Data Windsensor)
#define CL_SSID      "NoWa"					//Windmesser
#define CL_PASSWORD  "12345678"		
int iSTA_on = 0;                            // Status STA-Mode
int bConnect_CL = 0;
bool bClientConnected = 0;

// Calibration data variable definition for ADC1 and ADC2 Input
double ADC_Calibration_Value1 = 170.0; /**< For resistor measure 5 Volt and 180 Ohm equals 100% plus 1K resistor. Old Value 250.0 */
double ADC_Calibration_Value2 = 19.0;  /**< The real value depends on the true resistor values for the ADC input (100K / 27 K). Old value 34.3 */

//Confuration Sensors I2C
#define I2C_SDA 21                      //Standard 21
#define I2C_SCL 22                      //Standard 22
#define SEALEVELPRESSURE_HPA (1013.25)  //1013.25
float fbmp_temperature = 0;
float fbmp_pressure = 0;
float fbmp_altitude = 0;
String sI2C_Status = "";
bool bI2C_Status = 0;

// Global Data Sonar
const int iMaxSonar = 35;			//Analoginput 
int iDistance = 0;

// Global Data Motordata Sensor
float FuelLevel = 0;
float FuelLevelMax = 30;
float CoolantTemp = 0;
float MotorTemp = 0;
float EngineRPM = 0;
float BordSpannung = 0;
bool EngineOn = false;
String motorErrorReported = "";
String coolantErrorReported = "";
static unsigned long Counter;			// Enginehours
enum EngineStatus { Off = 0,  On = 1, };
#define RPM_Calibration_Value 4.0 // Translates Generator RPM to Engine RPM 
#define Eingine_RPM_Pin 19  // Engine RPM is measured as interrupt on GPIO 23

// Global Data Battery
int Bat1Capacity = 55; 	// Starterbatterie
int Bat2Capacity = 90;	// Versorgerbatterie
int SoCError = 0;
float BatSoC = 0;

// Data wire for teperature (Dallas DS18B20) 
#define ONE_WIRE_BUS 14			// Data wire for teperature (Dallas DS18B20) is plugged into GPIO 13
String sOneWire_Status = "";

// Variables Website
float fDrehzahl = 0;
float fGaugeDrehzahl = 0;
float fBordSpannung = 0;
float fCoolantTemp = 0;
float fMotorTemp = 0;
float fCoolantOffset = 0;
float fMotorOffset = 0;
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
#define SERVER_HOST_NAME "192.168.30.15"		//"192.168.76.34"
#define TCP_PORT 6666						//6666
#define DNS_PORT 53

//Variable NMEA 0183 Stream
const char *udpAddress = "192.168.30.255"; // Set network address for broadcast
const int udpPort = 4444;                 // UDP port

#endif  
