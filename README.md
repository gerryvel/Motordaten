# MotorData NMEA2000

## Description
This repository shows how to measure the 
- Battery Voltage
- Engine RPM
- Fuel Level
- Oil and Motor Temperature
- Alarms engine stop and tempertur high
- Enginehours

and send it as NNMEA2000 meassage.
- PGN 127488 // Engine Rapid / RPM
- PGN 127489 // Engine parameters dynamic 
- PGN 127505 // Fluid Level  
- PGN 127506 // Battery
- PGN 127508 // Battery Status

In addition, all data and part of the configuration are displayed as a website. 

Doxygen Documentation [Link](https://github.com/gerryvel/Motordaten/blob/master/doku/html/)

## Based on the work of

[NMEA2000-Data-Sender](https://github.com/AK-Homberger/NMEA2000-Data-Sender) @AK-Homberger

[Timo Lappalainen](https://github.com/ttlappalainen/NMEA2000) @ttlappalainen

This project is part of [OpenBoatProject](https://open-boat-projects.org/)

## Website
![grafik](https://github.com/user-attachments/assets/7f13e1f1-09d9-42a5-9052-dfacef8e9292)
![Screenshot 2024-12-23 005610](https://github.com/user-attachments/assets/a093962c-62a2-446a-813d-38cb818b8de0)
![Screenshot 2024-12-23 005626](https://github.com/user-attachments/assets/9139f854-f28e-4d1d-b924-b2e1e9c9c179)
![Screenshot 2024-12-23 005528](https://github.com/user-attachments/assets/ff8c5bd1-63ca-4ba9-b23a-eec77886b783)
![Screenshot 2024-12-23 005643](https://github.com/user-attachments/assets/614e239a-4707-4759-a179-f3f3933c439a)

![image](https://github.com/user-attachments/assets/19a4b7fd-f4e0-4ad4-b478-bb63acd92f85)


## Wiring diagram

![grafik](https://github.com/user-attachments/assets/3e34b79b-600b-46ec-8f39-7aeec70b3e48)

## PCB Layout
![grafik](https://github.com/user-attachments/assets/23302354-f9c6-4d82-b5b5-6188d9216be6)
![grafik](https://github.com/user-attachments/assets/cad63dee-a283-48bf-8012-41dd0a54c9bb)
![grafik](https://github.com/user-attachments/assets/bf01b423-9d3f-4fee-8e42-c56bea9e6acb)

The project requires the NMEA2000 and the NMEA2000_esp32 libraries from Timo Lappalainen: https://github.com/ttlappalainen.
Both libraries have to be downloaded and installed.

The ESP32 in this project is an Adafruit Huzzah! ESP32. Pin layout for other ESP32 devices might differ.

For the ESP32 CAN bus, I used the "SN65HVD230 Chip from TI" as transceiver. It works well with the ESP32.
The correct GPIO ports are defined in the main sketch. For this project, I use the pins GPIO4 for CAN RX and GPIO5 for CAN TX. 

The 12 Volt is reduced to 5 Volt with a DC Step-Down_Converter. 12V DC comes from the N2k Bus Connector with the M12 Connector.

The Website use LittleFS Filesystem. You must use Partition Schemes "Minimal SPIFFS with APPS and OTA".
The HTML Data upload separately with 
- "ESP 32 Skcetch Data upload" (Arduino IDE) or 
- PlatformIO > Build Filesystem and Upload Filesystem Image (PlatformIO) 
from /data directory.

## Partlist:

- PCB by Aisler [Link](https://aisler.net/p/JCQLQVHC)
  
Assembly: [MD N2k__Assembly.pdf](https://github.com/gerryvel/Motordaten/files/13480525/MD.N2k__Assembly.pdf)

- 1			C1	10µ	    CP_EIA-7343-15_Kemet-W_Pad2.25x2.55mm_HandSolder	1
- 2			C2	22µ	    CP_EIA-7343-15_Kemet-W_Pad2.25x2.55mm_HandSolder	1
- 3			R1	100k	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 4			R2	27k	    R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 5			R3	300R	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 6			R4	10k	    R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 7			R5	1k	    R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 8			R6	4k7	    R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 9			R7	2k	    R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 10		D1	B360	B 360 F Schottkydiode, 60 V, 3 A, DO-214AB/SMC     	1
- 11		D2	LED_RBKG	RGB LED Kingbright     	1
- 12		D3	PESD1CAN	SOT-23 Dual bidirectional TVS diode	1
- 13		D4	ZPD3.3	D_DO-35_SOD27_P10.16mm_Horizontal	1  [Link](https://www.reichelt.de/zenerdiode-3-3-v-0-5-w-do-35-zf-3-3-p23126.html?&trstct=pos_6&nbc=1)
- 14		D5	1N4148	D_DO-35_SOD27_P7.62mm_Horizontal	1  [Link](https://www.reichelt.de/schalt-diode-100-v-150-ma-do-35-1n-4148-p1730.html?search=1n4148)
- 15		D6	P4SMAJ26CA	D_SMA_TVS	1
- 16		U1	TSR_1-2450	Converter_DCDC_TRACO_TSR-1_THT	1   [Link](https://www.reichelt.de/dc-dc-wandler-tsr-1-1-w-5-v-1000-ma-sil-to-220-tsr-1-2450-p116850.html?search=tsr+1-24)
- 17		U2	ESP32-Huzzah	Adafruit_ESP32	1
- 18		U3	SN65HVD230	SOIC-8_3.9x4.9mm_P1.27mm	1  [Link](https://www.reichelt.de/high-speed-can-transceiver-1-mbit-s-3-3-v-so-8-sn-65hvd230d-p58427.html?&trstct=pos_0&nbc=1)
- 19		U4	H11L1	DIP-6_W7.62mm	1  [Link](https://www.reichelt.de/optokoppler-1-mbit-s-dil-6-h11l1m-p219351.html?search=H11-l1)
- 20		FL1	EPCO B82789C0513	B82789C0113N002	1
- 21		J2, J3	Conn_01x04_Pin	PinHeader_1x04_P2.54mm_Vertical	2
- 22		J1	Conn_01x03_Pin	PinHeader_1x03_P2.54mm_Vertical	1
- 23        Wago-Case: [Link](https://www.wago.com/de/zubehoer/gehaeuse-55-mm/p/789-120)

## Changes

- Version 2.4 add Doxygen 
- Version 2.3 add Temperatur: Motor(Water)temp and OilTemp (2x OneWire), add Alarm Watertemp
- Version 2.2 add Motorparameter: EngineHours and Alarms (Oiltemp max / Engine Stop)
- Version 2.1 Minor updates website, change Engine Parameter to PGN127489 (Oil Temp)
- Version 2.0
    - update Website (code and html files)
    - change Hardware layout, add protection's and C's on Voltage input, add protection's for CanBus
    - change Webinterface, add calibration-offset for temperature
  
