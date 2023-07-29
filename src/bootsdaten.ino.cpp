# 1 "C:\\Users\\GERRYA~1\\AppData\\Local\\Temp\\tmppfzfsskn"
#include <Arduino.h>
# 1 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/Motordaten/src/bootsdaten.ino"







#include <Arduino.h>
#include <ArduinoOTA.h>
#include "BoardInfo.h"
#include "configuration.h"
#include "helper.h"
#include <esp.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ESP_WiFi.h>
#include <ESPAsyncWebServer.h>
#include "LED.h"
#include <SparkFun_MMA8452Q.h>
#include <Wire.h>
#include "Analog.h"
#include <LittleFS.h>
#include "NMEA0183Telegram.h"
#include <NMEA2000_CAN.h>
#include <NMEA2000.h>
#include <N2kMessages.h>


AsyncWebServer server(80);


String sBoardInfo;
BoardInfo boardInfo;


Preferences preferences;


MMA8452Q mma;


String sCL_Status = sWifiStatus(WiFi.status());
String replaceVariable(const String& var);
String processor(const String& var);
void SendNMEA0183Message(String var);
bool IsTimeToUpdate(unsigned long NextUpdate);
void SendN2kPitch(double Yaw, double Pitch, double Roll);
void setup();
void loop();
void debug_log(char* str);
void IRAM_ATTR handleInterrupt();
void setup();
void GetTemperature( void * parameter);
double ReadRPM();
bool IsTimeToUpdate(unsigned long NextUpdate);
void SetNextUpdate(unsigned long &NextUpdate, unsigned long Period);
void SendN2kBattery(double BatteryVoltage);
void SendN2kTankLevel(double level, double capacity);
void SendN2kExhaustTemp(double temp);
void SendN2kEngineRPM(double RPM);
double ReadVoltage(byte pin);
void loop();
#line 43 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/Motordaten/src/bootsdaten.ino"
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
   if (var == "sI2C_Status")return String(sI2C_Status);
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


WiFiUDP udp;

void SendNMEA0183Message(String var) {
  udp.beginPacket(udpAddress, udpPort);
  udp.println(var);
  udp.endPacket();
}


const unsigned long TransmitMessages[] PROGMEM = {127257L,
                                                  0
                                                 };

bool IsTimeToUpdate(unsigned long NextUpdate) {
    return (NextUpdate < millis());
 }
 unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset = 0) {
    return millis() + Period + Offset;
 }
 void SetNextUpdate(unsigned long &NextUpdate, unsigned long Period) {
    while ( NextUpdate < millis() ) NextUpdate += Period;
 }


void SendN2kPitch(double Yaw, double Pitch, double Roll) {
static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod);
tN2kMsg N2kMsg;

   if ( IsTimeToUpdate(SlowDataUpdated) ) {
     SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

     Serial.printf("Krängung     : %3.0f ", Pitch);
     Serial.println("°");

    SetN2kPGN127257(N2kMsg, 0, Yaw, Pitch, Roll);
    NMEA2000.SendMsg(N2kMsg);
  }
}


void setup()
{

 Serial.begin(115200);


 if (!LittleFS.begin(true)) {
  Serial.println("An Error has occurred while mounting LittleFS");
  return;
 }
 Serial.println("Speicher LittleFS benutzt:");
 Serial.println(LittleFS.usedBytes());

 File root = LittleFS.open("/");
   listDir(LittleFS, "/", 3);

 readConfig("/config.json");
 AP_SSID = tAP_Config.wAP_SSID;
 AP_PASSWORD = tAP_Config.wAP_Password;
 fKielOffset = atof(tAP_Config.wKiel_Offset);

 pinMode(iMaxSonar, INPUT);


   Wire.begin(I2C_SDA, I2C_SCL);
   I2C_scan();


   LEDInit();
   LEDoff();


   sBoardInfo = boardInfo.ShowChipIDtoString();


 bool MMAbegin = mma.init();

 switch (MMAbegin) {
 case 0:
  Serial.println("\nMMA could not start!");
  sI2C_Status = "Keinen Sensor gefunden!";
  bI2C_Status = 0;
  break;
 case 1:
  Serial.println("\nMMA found!");
  mma.init(SCALE_2G);
  Serial.print("Range = "); Serial.print(2 << mma.available());
  Serial.println("G");
  sI2C_Status = "Sensor gefunden!";
  bI2C_Status = 1;
 }


 if (!WiFi.setHostname(HostName))
  Serial.println("\nSet Hostname success");
 else
  Serial.println("\nSet Hostname not success");


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

 server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/x-icon");
 });
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

   writeConfig(p->value());
  }
  request->send(200, "text/plain", "Daten gespeichert");
 });




 server.begin();
 Serial.println("TCP server started");


 MDNS.addService("http", "tcp", 80);


   NMEA2000.SetN2kCANMsgBufSize(8);
   NMEA2000.SetN2kCANReceiveFrameBufSize(250);
   NMEA2000.SetN2kCANSendFrameBufSize(250);

   esp_efuse_mac_get_default(chipid);
   for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));


   NMEA2000.SetProductInformation("1",
                                 100,
                                 "BoatData Sensor Module",
                                 "1.0.2.25 (2023-05-30)",
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


void loop()
{

 bConnect_CL = WiFi.status() == WL_CONNECTED ? 1 : 0;



 LEDoff();
 LEDflash(LED(LEDBoard));
 if (bI2C_Status == 0) LEDblink(LED(Red));


 ArduinoOTA.handle();


  Serial.printf("Stationen mit AP verbunden = %d\n", WiFi.softAPgetStationNum());
  Serial.printf("Soft-AP IP address = %s\n", WiFi.softAPIP().toString());
  sCL_Status = sWifiStatus(WiFi.status());


 Serial.printf("X: %f °\n", mma.getX() / 11.377);
 fKraengung = mma.getX() / 11.377;
 fKraengung = (abs(fKraengung));
 fGaugeKraengung = mma.getX() / 11.377;
 Serial.printf("Y: %f °\n", mma.getY() / 11.377);
 Serial.printf("Z: %f °\n", mma.getZ() / 11.377);


 if (mma.getX() < -1)
  sSTBB = "Backbord";
 else sSTBB = "Steuerbord";
 Serial.printf("Kraengung nach: %s %f °\n", sSTBB, fKraengung);


 bool bSFM = 0;
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


 iDistance = analogRead(iMaxSonar);
 Serial.printf("Analogeingang: %i\n", iDistance);
 int Err = 0;
 fSStellung = analogInScale(iDistance, 3800, 300, 7.0, 80.0, Err);
 Serial.printf("Schwert: %f cm\n", fSStellung);
 fAbsTief = fSStellung + fKielOffset;
 Serial.printf("Tiefgang: %f cm\n", fAbsTief);


 readConfig("/config.json");
 fKielOffset = atof(tAP_Config.wKiel_Offset);


 SendNMEA0183Message(sendXDR());

 SendN2kPitch(0, fKraengung, 0);

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
}
# 1 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/Motordaten/src/Motordaten.ino"
# 17 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/Motordaten/src/Motordaten.ino"
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
#include <memory>
#include <N2kMessages.h>
#include <DallasTemperature.h>
#include "web.h"
#include <ESPmDNS.h>

#define ENABLE_DEBUG_LOG 0



AsyncWebServer server(80);

#define ADC_Calibration_Value1 250.0
#define ADC_Calibration_Value2 18.9

int NodeAddress;

Preferences preferences;


const unsigned long TransmitMessages[] PROGMEM = {127505L,
                                                  130311L,
                                                  127488L,
                                                  127508L,
                                                  0
                                                 };




#define RPM_Calibration_Value 1.0

#define Eingine_RPM_Pin 23

volatile uint64_t StartValue = 0;
volatile uint64_t PeriodCount = 0;
unsigned long Last_int_time = 0;
hw_timer_t * timer = NULL;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;



#define ONE_WIRE_BUS 34

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

DeviceAddress MotorThermometer;


#define TempSendOffset 0
#define TankSendOffset 40
#define RPM_SendOffset 80
#define BatterySendOffset 100
#define SlowDataUpdatePeriod 1000

const int ADCpin2 = 35;






float ExhaustTemp = 0;
float EngineRPM = 0;
float BatteryVolt = 0;


TaskHandle_t Task1;


const int baudrate = 38400;
const int rs_config = SERIAL_8N1;


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


 if (!LittleFS.begin(true)) {
  Serial.println("An Error has occurred while mounting LittleFS");
  return;
 }
 Serial.println("Speicher LittleFS benutzt:");
 Serial.println(LittleFS.usedBytes());

 File root = LittleFS.open("/");
   listDir(LittleFS, "/", 3);

 readConfig("/config.json");
 AP_SSID = tAP_Config.wAP_SSID;
 AP_PASSWORD = tAP_Config.wAP_Password;
 fTempOffset = atof(tAP_Config.wTemp_Offset);


  LEDInit();


   sBoardInfo = boardInfo.ShowChipIDtoString();


 if (!WiFi.setHostname(HostName))
  Serial.println("\nSet Hostname success");
 else
  Serial.println("\nSet Hostname not success");


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


 server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/x-icon");
 });
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

   writeConfig(p->value());
  }
  request->send(200, "text/plain", "Daten gespeichert");
 });


 server.begin();
 Serial.println("TCP server started");


 MDNS.addService("http", "tcp", 80);



  pinMode(Eingine_RPM_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Eingine_RPM_Pin), handleInterrupt, FALLING);
  timer = timerBegin(0, 80, true);



  timerStart(timer);



  sensors.begin();


  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");


  if (!sensors.getAddress(MotorThermometer, 0)) Serial.println("Unable to find address for Device 0");




  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANReceiveFrameBufSize(250);
  NMEA2000.SetN2kCANSendFrameBufSize(250);

  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));


  NMEA2000.SetProductInformation("1",
                                 100,
                                 "MD Sensor Module",
                                 "1.0.2.25 (2023-05-30)",
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
}


void GetTemperature( void * parameter) {
  float tmp = 0;
  for (;;) {
    sensors.requestTemperatures();
    vTaskDelay(100);
    tmp = sensors.getTempCByIndex(0);
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



void SendN2kBattery(double BatteryVoltage) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, BatterySendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Voltage     : %3.1f ", BatteryVoltage);
    Serial.println("V");

    SetN2kDCBatStatus(N2kMsg, 0, BatteryVoltage, N2kDoubleNA, N2kDoubleNA, 1);
    NMEA2000.SendMsg(N2kMsg);
  }
}


void SendN2kTankLevel(double level, double capacity) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, TankSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Fuel Level  : %3.1f ", level);
    Serial.println("%");

    SetN2kFluidLevel(N2kMsg, 0, N2kft_Fuel, level, capacity );
    NMEA2000.SendMsg(N2kMsg);
  }
}


void SendN2kExhaustTemp(double temp) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, TempSendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Exhaust Temp: %3.1f °C \n", temp);



    SetN2kEnvironmentalParameters(N2kMsg, 0, N2kts_ExhaustGasTemperature, CToKelvin(temp),
                                  N2khs_Undef, N2kDoubleNA, N2kDoubleNA);





    NMEA2000.SendMsg(N2kMsg);
  }
}


void SendN2kEngineRPM(double RPM) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, RPM_SendOffset);
  tN2kMsg N2kMsg;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    Serial.printf("Engine RPM  :%4.0f RPM \n", RPM);

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


  LEDblink(LED(Green));
  if (!sensors.getAddress(MotorThermometer, 0)) LEDflash(LED(Red));


 bConnect_CL = WiFi.status() == WL_CONNECTED ? 1 : 0;


  Serial.printf("Stationen mit AP verbunden = %d\n", WiFi.softAPgetStationNum());
  Serial.printf("Soft-AP IP address = %s\n", WiFi.softAPIP().toString());
  sCL_Status = sWifiStatus(WiFi.status());

  unsigned int size;

  BatteryVolt = ((BatteryVolt * 15) + (ReadVoltage(ADCpin2) * ADC_Calibration_Value2 / 4096)) / 16;



  EngineRPM = ((EngineRPM * 5) + ReadRPM() * RPM_Calibration_Value) / 6 ;





  SendN2kExhaustTemp(ExhaustTemp);
  SendN2kEngineRPM(EngineRPM);
  SendN2kBattery (BatteryVolt);

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

}