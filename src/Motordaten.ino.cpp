# 1 "C:\\Users\\GERRYA~1\\AppData\\Local\\Temp\\tmptn1bm232"
#include <Arduino.h>
# 1 "C:/Users/gerryadmin/Documents/Motordaten/src/Motordaten.ino"
# 17 "C:/Users/gerryadmin/Documents/Motordaten/src/Motordaten.ino"
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include "BoardInfo.h"
#include "configuration.h"
#include "helper.h"
#include <ESP_WiFi.h>
#include <ESPAsyncWebServer.h>
#include "LED.h"
#include <NMEA2000_CAN.h>
#include <N2kMessages.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "web.h"
#include <ESPmDNS.h>
#include <arpa/inet.h>
#include "Analog.h"
#include "uptime.h"
#include "hourmeter.h"

#define ENABLE_DEBUG_LOG 0

#define ADC_Calibration_Value1 250.0
#define ADC_Calibration_Value2 19.0



const unsigned long TransmitMessages[] PROGMEM = {127488L,
                                                  127489L,
                                                  127505L,
                                                  127506L,
                                                  127508L,
                                                  0
                                                 };




volatile uint64_t StartValue = 0;
volatile uint64_t PeriodCount = 0;
unsigned long Last_int_time = 0;
hw_timer_t * timer = NULL;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

DeviceAddress MotorThermometer;

const int ADCpin2 = 35;
const int ADCpin1 = 34;


TaskHandle_t Task1;


const int baudrate = 38400;
const int rs_config = SERIAL_8N1;
void debug_log(char* str);
void IRAM_ATTR handleInterrupt();
void setup();
void GetTemperature( void * parameter);
double ReadRPM();
bool IsTimeToUpdate(unsigned long NextUpdate);
void SetNextUpdate(unsigned long &NextUpdate, unsigned long Period);
void SendN2kDCStatus(double BatteryVoltage, double SoC, double BatCapacity);
void SendN2kBattery(double BatteryVoltage);
void SendN2kTankLevel(double level, double capacity);
void SendN2kExhaustTemp(double temp, double rpm, double hours, double voltage);
void SendN2kEngineRPM(double RPM);
double ReadVoltage(byte pin);
void loop();
#line 78 "C:/Users/gerryadmin/Documents/Motordaten/src/Motordaten.ino"
void debug_log(char* str) {
#if ENABLE_DEBUG_LOG == 1
  Serial.println(str);
#endif
}




void IRAM_ATTR handleInterrupt()
{
  portENTER_CRITICAL_ISR(&mux);
  uint64_t TempVal = timerRead(timer);
  PeriodCount = TempVal - StartValue;
  StartValue = TempVal;
  portEXIT_CRITICAL_ISR(&mux);
  Last_int_time = millis();
}


void setup() {

  uint8_t chipid[6];
  uint32_t id = 0;
  int i = 0;


  Serial.begin(115200);

  Serial.printf("Motordaten setup %s start\n", Version);


 if (!LittleFS.begin(true)) {
  Serial.println("An Error has occurred while mounting LittleFS");
  return;
 }
 Serial.println("Speicher LittleFS benutzt:");
 Serial.println(LittleFS.usedBytes());

 File root = LittleFS.open("/");
   listDir(LittleFS, "/", 3);

 readConfig("/config.json");
  IP = inet_addr(tAP_Config.wAP_IP);
 AP_SSID = tAP_Config.wAP_SSID;
 AP_PASSWORD = tAP_Config.wAP_Password;
 fTempOffset = atof(tAP_Config.wTemp_Offset);
  Serial.println("Configdata :\n AP IP: " + IP.toString() + ", AP SSID: " + AP_SSID + " , Passwort: " + AP_PASSWORD + " , TempOffset: " + fTempOffset);


  LEDInit();


  sBoardInfo = boardInfo.ShowChipIDtoString();





  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPdisconnect();
  if(WiFi.softAP(AP_SSID, AP_PASSWORD, channel, hide_SSID, max_connection)){
    WiFi.softAPConfig(IP, Gateway, NMask);
    Serial.println("\nNetwork " + String(AP_SSID) + " running");
    Serial.println("\nNetwork IP " + IP.toString() + " ,GW: " + Gateway.toString() + " ,Mask: " + NMask.toString() + " set");
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
  Serial.println("Set Hostname with: " + String(WiFi.getHostname()) + " done");

  delay(1000);
  WiFiDiag();

 if (!MDNS.begin(AP_SSID)) {
  Serial.println("Error setting up MDNS responder!");
  while (1) {
   delay(1000);
  }
 }
Serial.println("mDNS responder started\n");


 server.begin();
 Serial.println("TCP server started\n");


 MDNS.addService("http", "tcp", 80);


  website();


  pinMode(Eingine_RPM_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Eingine_RPM_Pin), handleInterrupt, FALLING);
  timer = timerBegin(0, 80, true);



  timerStart(timer);


  sensors.begin();
  Serial.print("Found device ");
  Serial.print(sensors.getDeviceCount(), DEC);
  sOneWire_Status = String(sensors.getAddress(MotorThermometer, 0));
  Serial.println(", Adresse " + sOneWire_Status);


  if (!sensors.getAddress(MotorThermometer, 0)) Serial.println("Unable to find address for Device 0");


  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANReceiveFrameBufSize(250);
  NMEA2000.SetN2kCANSendFrameBufSize(250);

  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));


  NMEA2000.SetProductInformation("MD01",
                                 100,
                                 "MD Sensor Module",
                                 "2.0.0.0 (2024-06-07)",
                                 "1.0.2.0 (2023-05-30)"
                                );

  NMEA2000.SetDeviceInformation(id,
                                132,
                                25,
                                2046
                               );



  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text);

  preferences.begin("nvs", false);
  NodeAddress = preferences.getInt("LastNodeAddress", 33);
  preferences.end();
  Serial.printf("NodeAddress=%d\n", NodeAddress);

  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, NodeAddress);
  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.Open();

 xTaskCreatePinnedToCore(
    GetTemperature,
    "Task1",
    10000,
    NULL,
    0,
    &Task1,
    0);

  delay(200);

  printf("Setup end\n");
}


void GetTemperature( void * parameter) {
  float tmp = 0;
  for (;;) {
    sensors.requestTemperatures();
    vTaskDelay(100);
    tmp = sensors.getTempCByIndex(0) + fTempOffset;
    if (tmp != -127) ExhaustTemp = tmp;
    vTaskDelay(100);
  }
}


double ReadRPM() {
  double RPM = 0;

  portENTER_CRITICAL(&mux);
  if (PeriodCount != 0) {
    RPM = 1000000.00 / PeriodCount;
  }
  portEXIT_CRITICAL(&mux);
  if (millis() > Last_int_time + 200) RPM = 0;
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



void SendN2kDCStatus(double BatteryVoltage, double SoC, double BatCapacity) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, BatteryDCStatusSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Voltage     : %3.1f V\n", BatteryVoltage);
    Serial.printf("SoC         : %3.1f %\n", SoC);
    Serial.printf("Capacity    : %3.1f Ah\n", BatCapacity);

    SetN2kDCStatus(N2kMsg, 1, 2, N2kDCt_Battery, SoC, 0, N2kDoubleNA, BatteryVoltage, AhToCoulomb(55));
    NMEA2000.SendMsg(N2kMsg);
  }
}

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

void SendN2kTankLevel(double level, double capacity) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, TankSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Fuel Level  : %3.1f %\n", level);

    SetN2kFluidLevel(N2kMsg, 0, N2kft_Fuel, level, capacity );
    NMEA2000.SendMsg(N2kMsg);
  }
}

void SendN2kExhaustTemp(double temp, double rpm, double hours, double voltage) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, EngineSendOffset);
  tN2kMsg N2kMsg;
  tN2kEngineDiscreteStatus1 Status1;
  tN2kEngineDiscreteStatus2 Status2;
  Status1.Bits.OverTemperature = temp > 90;
  Status1.Bits.LowSystemVoltage = voltage < 11;
  Status2.Bits.EngineShuttingDown = rpm < 100;
  EngineOn = !Status2.Bits.EngineShuttingDown;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Engine Temp : %3.1f °C \n", temp);
    Serial.printf("Engine Hours: %3.1f hrs \n", hours);
    Serial.printf("Over Temp   : %s  \n", Status1.Bits.OverTemperature ? "Yes" : "No");
    Serial.printf("Engine Off  : %s  \n", Status2.Bits.EngineShuttingDown ? "Yes" : "No");



    SetN2kEngineDynamicParam(N2kMsg, 0, N2kDoubleNA, CToKelvin(temp), N2kDoubleNA, N2kDoubleNA, N2kDoubleNA, hours ,N2kDoubleNA ,N2kDoubleNA, N2kInt8NA, N2kInt8NA, Status1, Status2);

    NMEA2000.SendMsg(N2kMsg);
  }
}

void SendN2kEngineRPM(double RPM) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, RPMSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Engine RPM  : %4.0f RPM \n", RPM);

    SetN2kEngineParamRapid(N2kMsg, 0, RPM, N2kDoubleNA, N2kInt8NA);

    NMEA2000.SendMsg(N2kMsg);
  }
}



double ReadVoltage(byte pin) {
  double reading = analogRead(pin);
  if (reading < 1 || reading > 4095) return 0;

  return (-0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089) * 1000;
}


void loop() {


  LEDflash(LED(Green));






 bConnect_CL = WiFi.status() == WL_CONNECTED ? 1 : 0;



  BordSpannung = ((BordSpannung * 15) + (ReadVoltage(ADCpin2) * ADC_Calibration_Value2 / 4096)) / 16;

  FuelLevel = ((FuelLevel * 15) + (ReadVoltage(ADCpin1) * ADC_Calibration_Value1 / 4096)) / 16;

  EngineRPM = ((EngineRPM * 5) + ReadRPM() * RPM_Calibration_Value) / 6 ;

  BatSoC = (BordSpannung - 10.5) * (100.0 - 0.0) / (14.9 - 10.5) + 0.0;


  EngineHours(EngineOn);

  SendN2kTankLevel(FuelLevel, FuelLevelMax);
  SendN2kExhaustTemp(ExhaustTemp, EngineRPM, Counter, BordSpannung);
  SendN2kEngineRPM(EngineRPM);
  SendN2kBattery(BordSpannung);
  SendN2kDCStatus(BordSpannung, BatSoC, Bat1Capacity);

  NMEA2000.ParseMessages();
  int SourceAddress = NMEA2000.GetN2kSource();
  if (SourceAddress != NodeAddress) {
    NodeAddress = SourceAddress;
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }


  if ( Serial.available() ) {
    Serial.read();
  }

freeHeapSpace();


 ArduinoOTA.handle();


fTemp = ExhaustTemp;
fBordSpannung = BordSpannung;
fDrehzahl = EngineRPM;
sCL_Status = sWifiStatus(WiFi.status());
sAP_Station = WiFi.softAPgetStationNum();

if (IsRebootRequired) {
  Serial.println("Rebooting ESP32: ");
  delay(1000);
  ESP.restart();
  }

}