
/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file Motordaten.ino
 * @author Gerry Sebb
 * @brief Motordaten NMEA2000
 * @version 2.4
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <Arduino.h>
#include "configuration.h"
#include <Preferences.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP_WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include <N2kMessages.h>
#include <ESPmDNS.h>
#include <arpa/inet.h>
#include "BoardInfo.h"
#include "helper.h"
#include "LED.h"
#include "web.h"
#include "hourmeter.h"

#define ENABLE_DEBUG_LOG 0 // Debug log

/**
 * @brief ADC calibration
 * Calibration data variable definition for ADC1 and ADC2 Input
 */
#define ADC_Calibration_Value1 250.0 /**< For resistor measure 5 Volt and 180 Ohm equals 100% plus 1K resistor. */
#define ADC_Calibration_Value2 19.0  /**< The real value depends on the true resistor values for the ADC input (100K / 27 K). Old value 34.3 */

/**
 *  Set the information for other bus devices, which PGN messages we support
 */ 
const unsigned long TransmitMessages[] PROGMEM = {127488L, // Engine Rapid / RPM
                                                  127489L, // Engine parameters dynamic
                                                  127505L, // Fluid Level
                                                  127506L, // Battery 
                                                  127508L, // Battery Status
                                                  0
                                                 };


/** 
 * RPM data. Generator RPM is measured on connector "W"
*/

volatile uint64_t StartValue = 0;                 /**< First interrupt value */
volatile uint64_t PeriodCount = 0;                /**< period in counts of 0.000001 of a second */
unsigned long Last_int_time = 0;
hw_timer_t * timer = NULL;                        /**< pointer to a variable of type hw_timer_t */
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  /**< synchs between maon cose and interrupt? */

/** 
 *  Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
 */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);  /**< Pass our oneWire reference to Dallas Temperature. */
// DeviceAddress MotorThermometer;    /**< arrays to hold device addresses
uint8_t MotorCoolant[8] = { 0x28, 0xD3, 0x81, 0xCF, 0x0F, 0x0, 0x0, 0x79 }; /**< DeviceAddress Coolant */
uint8_t MotorOil[8] = { 0x28, 0xB0, 0x3C, 0x1A, 0xF, 0x0, 0x0, 0xC0 };      /**< DeviceAddress Engine Oil */

const int ADCpin2 = 35; /**< Voltage measure is connected GPIO 35 (Analog ADC1_CH7) */
const int ADCpin1 = 34; /**< Tank fluid level measure is connected GPIO 34 (Analog ADC1_CH6) */

/** Task handle for OneWire read (Core 0 on ESP32) */ 
TaskHandle_t Task1;

/**  Serial port 2 config (GPIO 16)  */
const int baudrate = 38400;
const int rs_config = SERIAL_8N1;

void debug_log(char* str) {
#if ENABLE_DEBUG_LOG == 1
  Serial.println(str);
#endif
}

/**
 * @brief RPM Event Interrupt
 * Enters on falling edge
 * @return * void 
 */
//=======================================
void IRAM_ATTR handleInterrupt()
{
  portENTER_CRITICAL_ISR(&mux);
  uint64_t TempVal = timerRead(timer);        // value of timer at interrupt
  PeriodCount = TempVal - StartValue;         // period count between rising edges in 0.000001 of a second
  StartValue = TempVal;                       // puts latest reading as start for next calculation
  portEXIT_CRITICAL_ISR(&mux);
  Last_int_time = millis();
}

/******************************************* Setup *******************************************************/
void setup() {

  // Init USB serial port
  Serial.begin(115200);

  Serial.printf("Motordaten setup %s start\n", Version);

  /**
   * @brief Filesystem prepare for Webfiles
   * 
   */
	if (!LittleFS.begin(true)) {
		Serial.println("An Error has occurred while mounting LittleFS");
		return;
	}
	Serial.println("\nBytes LittleFS used:" + String(LittleFS.usedBytes()));

	File root = LittleFS.open("/");
  listDir(LittleFS, "/", 3);

	/**
	 * @brief file exists, reading and loading config file
	 * 
	 */
  readConfig("/config.json");
    IP = inet_addr(tAP_Config.wAP_IP);
    AP_SSID = tAP_Config.wAP_SSID;
    AP_PASSWORD = tAP_Config.wAP_Password;
    fMotorOffset = atof(tAP_Config.wMotor_Offset);
    fCoolantOffset = atof(tAP_Config.wCoolant_Offset);
    FuelLevelMax = atof(tAP_Config.wFuellstandmax);
    Serial.println("\nConfigdata : AP IP: " + IP.toString() + ", AP SSID: " + AP_SSID + " , Passwort: " + AP_PASSWORD + " , MotorTOffset: " + fMotorOffset + " , CoolantTOffset: " + fCoolantOffset + " read from file");

  // LED
  LEDInit();

  // Boardinfo	
  /**
   * @brief 
   * Read Boardinfo for output 
   */
    sBoardInfo = boardInfo.ShowChipIDtoString();

	//Wifi
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPdisconnect();
  if(WiFi.softAP(AP_SSID, AP_PASSWORD, channel, hide_SSID, max_connection)){
    WiFi.softAPConfig(IP, Gateway, NMask);
    Serial.println("\nAccesspoint " + String(AP_SSID) + " running");
    Serial.println("\nSet IP " + IP.toString() + " ,Gateway: " + Gateway.toString() + " ,NetMask: " + NMask.toString() + " ready");
    LEDon(LED(Green));
    delay(1000);
  } else {
      Serial.println("Starting AP failed.");
      LEDoff(LED(Green));
      LEDon(LED(Red));  
      delay(1000); 
      ESP.restart();
  }
  
  WiFi.setHostname(HostName);
  Serial.println("Set Hostname " + String(WiFi.getHostname()) + " done\n");

  delay(1000);
  WiFiDiag();

	if (!MDNS.begin(AP_SSID)) {
		Serial.println("Error setting up MDNS responder!");
		while (1) {
			delay(1000);
		}
	}
  Serial.println("mDNS responder started\n");

// Start TCP (HTTP) server
	server.begin();
	Serial.println("TCP server started\n");

// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);

// Webconfig laden
  website();

/**
 * @brief Construct a new pin Mode object
 * 
 */
  pinMode(Eingine_RPM_Pin, INPUT_PULLUP);                                            // sets pin high
  attachInterrupt(digitalPinToInterrupt(Eingine_RPM_Pin), handleInterrupt, FALLING); // attaches pin to interrupt on Falling Edge
  timer = timerBegin(0, 80, true);                                                // this returns a pointer to the hw_timer_t global variable
  // 0 = first timer
  // 80 is prescaler so 80MHZ divided by 80 = 1MHZ signal ie 0.000001 of a second
  // true - counts up
  timerStart(timer);                                                              // starts the timer

/**
 * @brief Start OneWire
 * 
 */
  sensors.begin();
  oneWire.reset();
    Serial.print("OneWire: Found ");
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial.println(" devices.");
    Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");
  sOneWire_Status = String(sensors.getDeviceCount(), DEC);
  
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  
  Serial.print("Looking for 1-Wire devices...\n\r");
  while(oneWire.search(addr)) {
    Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return;
    }
  }
  Serial.print("\n\rNo more sensors!\n\r");
  oneWire.reset_search();
  delay(250);

// search for devices on the bus and assign based on an index
  if (!sensors.getAddress(MotorOil, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(MotorCoolant, 1)) Serial.println("Unable to find address for Device 1");

  

// Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANReceiveFrameBufSize(250);
  NMEA2000.SetN2kCANSendFrameBufSize(250);

  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

/**
 * @brief Set NMEA2000 product information
 * 
 */
  NMEA2000.SetProductInformation("MD01", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                 "MD Sensor Module",  // Manufacturer's Model ID
                                 "2.3.0.0 (2024-12-20)",  // Manufacturer's Software version code
                                 "2.0.0.0 (2023-05-30)" // Manufacturer's Model version
                                );
// Set device information
  NMEA2000.SetDeviceInformation(id, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );

// If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below

  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.

  preferences.begin("nvs", false);                          // Open nonvolatile storage (nvs)
  NodeAddress = preferences.getInt("LastNodeAddress", 33);  // Read stored last NodeAddress, default 33
  preferences.end();
  Serial.printf("NodeAddress=%d\n", NodeAddress);

  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, NodeAddress);
  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.Open();

 xTaskCreatePinnedToCore(
    GetTemperature, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */

  delay(200);

/**
 * @brief OTA
 * 
 */
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
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

  ArduinoOTA.begin();
 
  printf("Setup end\n");
}

/**
 * @brief Get the Temperature object
 * This task runs isolated on core 0 because sensors.requestTemperatures() is slow and blocking for about 750 ms
 * With error on Sensor set output to -5°C
 * @param parameter 
 */

void GetTemperature( void * parameter) {
  float tmp0 = 0;
  float tmp1 = 0;
  for (;;) {
    sensors.requestTemperatures();                       // Send the command to get temperatures
    vTaskDelay(100);
    tmp0 = sensors.getTempC(MotorOil);
    if (tmp0 == DEVICE_DISCONNECTED_C) {
       if (!motorErrorReported) {                        // Nur einmal melden
        Serial.print("Error read Motor Temp\n");
        motorErrorReported = true;}
    MotorTemp = -5.0;
    } else {
        MotorTemp = tmp0 + fMotorOffset;
        motorErrorReported = false;                      // Fehler wurde behoben
    }
    vTaskDelay(100);
    tmp1 = sensors.getTempC(MotorCoolant);
    if (tmp1 == DEVICE_DISCONNECTED_C) {
       if (!coolantErrorReported) {                      // Nur einmal melden
        Serial.print("Error read Coolant Temp\n");
        coolantErrorReported = true;}
    CoolantTemp = -5.0;
    } else {
        CoolantTemp = tmp1 + fCoolantOffset;
        coolantErrorReported = false;                     // Fehler wurde behoben
    }
    vTaskDelay(100);
  }
  
  }

/**
 * @brief Calculate engine RPM from number of interupts per time
 * 
 * @return double 
 */
double ReadRPM() {
  double RPM = 0;

  portENTER_CRITICAL(&mux);
  if (PeriodCount != 0) {                            // 0 means no signals measured
    RPM = 1000000.00 / PeriodCount;                  // PeriodCount in 0.000001 of a second  
  }  
  portEXIT_CRITICAL(&mux);
  if (millis() > Last_int_time + 200) RPM = 0;       // No signals RPM=0;
  return (RPM);
}
 

bool IsTimeToUpdate(unsigned long NextUpdate) {
  return (NextUpdate < millis());
}
unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset = 0) {
  return millis() + Period + Offset;
}

void SetNextUpdate(unsigned long &NextUpdate, unsigned long Period) {
  while ( NextUpdate < millis() ) NextUpdate += Period;
}

/************************ n2k Datenfunktionen ***************************/
/**
 * @brief Send PGN127506
 * 
 * @param BatteryVoltage 
 * @param SoC 
 * @param BatCapacity 
 */
void SendN2kDCStatus(double BatteryVoltage, double SoC, double BatCapacity) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, BatteryDCStatusSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Voltage     : %3.1f V\n", BatteryVoltage);
    Serial.printf("SoC         : %3.1f %\n", SoC);
    Serial.printf("Capacity    : %3.1f Ah\n", BatCapacity);
    // SetN2kDCStatus(N2kMsg,1,1,N2kDCt_Battery,56,92,38500,0.012, AhToCoulomb(420));
    SetN2kDCStatus(N2kMsg, 1, 2, N2kDCt_Battery, SoC, 0,  N2kDoubleNA, BatteryVoltage, AhToCoulomb(55));
    NMEA2000.SendMsg(N2kMsg);
  }
}

/**
 * @brief Send PGN127508
 * 
 * @param BatteryVoltage 
 */
void SendN2kBattery(double BatteryVoltage) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, BatteryDCSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Voltage     : %3.1f V\n", BatteryVoltage);

    SetN2kDCBatStatus(N2kMsg, 2, BatteryVoltage, N2kDoubleNA, N2kDoubleNA, 1);
    NMEA2000.SendMsg(N2kMsg);
  }
}

/**
 * @brief Send PGN 127505
 * 
 * @param level 
 * @param capacity 
 */
void SendN2kTankLevel(double level, double capacity) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, TankSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Fuel Level   : %3.1f %%\n", level);
    Serial.printf("Fuel Capacity: %3.1f l\n", capacity);

    SetN2kFluidLevel(N2kMsg, 0, N2kft_Fuel, level, capacity );
    NMEA2000.SendMsg(N2kMsg);
  }
}

/**
 * @brief Send PGN 127489
 * 
 * @param Oiltemp 
 * @param Coolanttemp 
 * @param rpm 
 * @param hours 
 * @param voltage 
 */
void SendN2kEngineData(double Oiltemp, double Coolanttemp, double rpm, double hours, double voltage) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, EngineSendOffset);
  tN2kMsg N2kMsg;
  tN2kEngineDiscreteStatus1 Status1;
  tN2kEngineDiscreteStatus2 Status2;
  Status1.Bits.OverTemperature = Oiltemp > 90;      // Alarm Motor over temp
  Status1.Bits.LowCoolantLevel = Coolanttemp > 90;    // Alarm low cooling
  Status1.Bits.LowSystemVoltage = voltage < 11;
  Status2.Bits.EngineShuttingDown = rpm < 100;      // Alarm Motor off
  EngineOn = !Status2.Bits.EngineShuttingDown;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Oil Temp    : %3.1f °C \n", Oiltemp);
    Serial.printf("Coolant Temp: %3.1f °C \n", Coolanttemp);
    Serial.printf("Engine Hours: %3.1f hrs \n", hours);
    Serial.printf("Overtemp Oil: %s  \n", Status1.Bits.OverTemperature ? "Yes" : "No");
    Serial.printf("Overtemp Mot: %s  \n", Status1.Bits.LowCoolantLevel ? "Yes" : "No");
    Serial.printf("Engine Off  : %s  \n", Status2.Bits.EngineShuttingDown ? "Yes" : "No");

    // SetN2kTemperatureExt(N2kMsg, 0, 0, N2kts_ExhaustGasTemperature, CToKelvin(temp), N2kDoubleNA);   // PGN130312, uncomment the PGN to be used

    SetN2kEngineDynamicParam(N2kMsg, 0, N2kDoubleNA, CToKelvin(Oiltemp), CToKelvin(Coolanttemp), N2kDoubleNA, N2kDoubleNA, hours ,N2kDoubleNA ,N2kDoubleNA, N2kInt8NA, N2kInt8NA, Status1, Status2);

    NMEA2000.SendMsg(N2kMsg);
  }
}

/**
 * @brief Send PGN 127488
 * 
 * @param RPM 
 */
void SendN2kEngineRPM(double RPM) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, RPMSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Engine RPM  : %4.0f RPM \n", RPM);

    SetN2kEngineParamRapid(N2kMsg, 0, RPM, N2kDoubleNA,  N2kInt8NA);

    NMEA2000.SendMsg(N2kMsg);
  }
}

/**
 * @brief ReadVoltage is used to improve the linearity of the ESP32 ADC
 * see: https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
 * @param pin 
 * @return double 
 */
double ReadVoltage(byte pin) {
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if (reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return (-0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089) * 1000;
} // Added an improved polynomial, use either, comment out as required

/************************************ Loop ***********************************/
void loop() {

  // LED
  LEDflash(LED(Green)); // flash for loop run

  // if (!sensors.getAddress(MotorThermometer, 0)) LEDflash(LED(Red));  // search for device on the bus and unable to find
  // sensors.requestTemperatures(); // Send the command to get temperatures
  // ExhaustTemp = sensors.getTempCByIndex(0) + fTempOffset;

  //Wifi variables
	bConnect_CL = WiFi.status() == WL_CONNECTED ? 1 : 0;

  // unsigned int size;

  BordSpannung = ((BordSpannung * 15) + (ReadVoltage(ADCpin2) * ADC_Calibration_Value2 / 4096)) / 16; // This implements a low pass filter to eliminate spike for ADC readings

  FuelLevel = ((FuelLevel * 15) + (ReadVoltage(ADCpin1) * ADC_Calibration_Value1 / 4096)) / 16; // This implements a low pass filter to eliminate spike for ADC readings

  EngineRPM = ((EngineRPM * 5) + ReadRPM() * RPM_Calibration_Value) / 6 ; // This implements a low pass filter to eliminate spike for RPM measurements

  BatSoC = (BordSpannung - 10.5) * (100.0 - 0.0) / (14.9 - 10.5) + 0.0;
  // float BatSoC = analogInScale(BordSpannung, 15, 10, 100.0, 0.0, SoCError);
  
  EngineHours(EngineOn);
  
  SendN2kTankLevel(FuelLevel, FuelLevelMax);  // Adjust max tank capacity
  SendN2kEngineData(MotorTemp, CoolantTemp, EngineRPM, Counter, BordSpannung);
  SendN2kEngineRPM(EngineRPM);
  SendN2kBattery(BordSpannung);
  SendN2kDCStatus(BordSpannung, BatSoC, Bat1Capacity);
  
  NMEA2000.ParseMessages();
  int SourceAddress = NMEA2000.GetN2kSource();
  if (SourceAddress != NodeAddress) { // Save potentially changed Source Address to NVS memory
    NodeAddress = SourceAddress;      // Set new Node Address (to save only once)
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }

  // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
  if ( Serial.available() ) {
    Serial.read();
  }
	

// OTA	
	ArduinoOTA.handle();

/**
 * @brief Actual Website Data
 * 
 */
    fCoolantTemp = CoolantTemp;
    fMotorTemp = MotorTemp;
    fBordSpannung = BordSpannung;
    fDrehzahl = EngineRPM;
    sCL_Status = sWifiStatus(WiFi.status());
    sAP_Station = WiFi.softAPgetStationNum();
    freeHeapSpace();
    /**
     * @brief Construct a new if object
     * Reboot from Website
     */
  if (IsRebootRequired) {
      Serial.println("Rebooting ESP32: "); 
      delay(1000); // give time for reboot page to load
      ESP.restart();
      }


}