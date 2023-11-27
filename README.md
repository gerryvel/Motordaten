# MotorData N2k
This repository shows how to measure the Battery Voltage, Engine RPM, Fuel Level and the Exhaust Temeperature and send it as NNMEA2000 meassage.
According to the idea of  [NMEA2000-Data-Sender](https://github.com/AK-Homberger/NMEA2000-Data-Sender) .

# Wiring diagram
![grafik](https://github.com/gerryvel/Motordaten/assets/17195231/200a603d-1bbc-480a-a8bb-c428463898cd)

# PCB Layout
![grafik](https://github.com/gerryvel/Motordaten/assets/17195231/152cf410-2b51-408b-b6c6-68b826d2239b)
![grafik](https://github.com/gerryvel/Motordaten/assets/17195231/f2e08ff9-fd00-447a-ac95-1913529f3cf8)

The project requires the NMEA2000 and the NMEA2000_esp32 libraries from Timo Lappalainen: https://github.com/ttlappalainen.
Both libraries have to be downloaded and installed.

The ESP32 in this project is an Adafruit Huzzah! ESP32. Pin layout for other ESP32 devices might differ.

For the ESP32 CAN bus, I used the "SN65HVD230 Chip from TI" as transceiver. It works well with the ESP32.
The correct GPIO ports are defined in the main sketch. For this project, I use the pins GPIO4 for CAN RX and GPIO5 for CAN TX. 

The 12 Volt is reduced to 5 Volt with a DC Step-Down_Converter (Traco-Power TSR 1-2450).

The following values are measured and transmitted to the NMEA2000 bus:

- The Exhaust Temperature is measured with a DS18B20 Sensor (the DallasTemperature library has to be installed with the Arduiono IDE Library Manager).
- The Engine RPM is measured on connection "W" of the generator/alternator. The Engine RPM is detected with a H11L1 optocoupler device (or alternatively a PC900v). This device plus the 2K resistor and the 1N4148 diode translates the signal from "W" connection of the generator to ESP32 GPIO pin 33. The diode is not critical an can be replaced with nearly any another type.
There is a RPM difference between generator and diesel engine RPM. The calibration value has to be set in the program.
- The Battery Voltage is measured at GPIO pin 35 (check calibration value with regards to the real resistor values of R4/R5).

The following PGNs are sent to the NMEA 2000 Bus:
- 127505 Fluid Level
- 130311 Temperature (or alternatively 130312, 130316)
- 127488 Engine Rapid / RPM
- 127508 Battery Status

Change the PGNs if your MFD can not show a certain PGN.
BTW: The full list of PGNs is defined in this header file of the NMEA 2000 library: https://github.com/ttlappalainen/NMEA2000/blob/master/src/N2kMessages.h

# Partlist:

- PCB by Aisler [Link](https://aisler.net/p/JCQLQVHC)
  
Assembly: [MD N2k__Assembly.pdf](https://github.com/gerryvel/Motordaten/files/13480525/MD.N2k__Assembly.pdf)

- 1			C1	10µ	CP_EIA-7343-15_Kemet-W_Pad2.25x2.55mm_HandSolder	1
- 2			C2	22µ	CP_EIA-7343-15_Kemet-W_Pad2.25x2.55mm_HandSolder	1
- 3			R1	100k	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 4			R2	27k	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 5			R3	300R	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 6			R4	10k	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 7			R5	1k	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 8			R6	4k7	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 9			R7	2k	R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal	1
- 10			D1	B360	D_SMC	1
- 11			D2	LED_RBKG	PinHeader_1x04_P2.00mm_LED RGB	1
- 12			D3	PESD1CAN	SOT-23	1
- 13			D4	ZPD3.3	D_DO-35_SOD27_P10.16mm_Horizontal	1  [Link](https://www.reichelt.de/zenerdiode-3-3-v-0-5-w-do-35-zf-3-3-p23126.html?&trstct=pos_6&nbc=1)
- 14			D5	1N4148	D_DO-35_SOD27_P7.62mm_Horizontal	1  [Link](https://www.reichelt.de/schalt-diode-100-v-150-ma-do-35-1n-4148-p1730.html?search=1n4148)
- 15			D6	P4SMAJ26CA	D_SMA_TVS	1
- 16			U1	TSR_1-2450	Converter_DCDC_TRACO_TSR-1_THT	1   [Link](https://www.reichelt.de/dc-dc-wandler-tsr-1-1-w-5-v-1000-ma-sil-to-220-tsr-1-2450-p116850.html?search=tsr+1-24)
- 17			U2	ESP32-Huzzah	Adafruit_ESP32	1
- 18			U3	SN65HVD230	SOIC-8_3.9x4.9mm_P1.27mm	1  [Link](https://www.reichelt.de/high-speed-can-transceiver-1-mbit-s-3-3-v-so-8-sn-65hvd230d-p58427.html?&trstct=pos_0&nbc=1)
- 19			U4	H11L1	DIP-6_W7.62mm	1  [Link](https://www.reichelt.de/optokoppler-1-mbit-s-dil-6-h11l1m-p219351.html?search=H11-l1)
- 20			FL1	EPCO B82789C0513	B82789C0113N002	1
- 21			J2, J3	Conn_01x04_Pin	PinHeader_1x04_P2.54mm_Vertical	2
- 22			J1	Conn_01x03_Pin	PinHeader_1x03_P2.54mm_Vertical	1
- Wago-Case: [Link](https://www.wago.com/de/zubehoer/gehaeuse-55-mm/p/789-120)
