# NMEA2000 Battery Voltage, Engine RPM and Exhaust Temp Sender
This repository shows how to measure the Battery Voltage, Engine RPM, Fuel Level and the Exhaust Temeperature and send it as NNMEA2000 meassage.

# Wiring diagram
![grafik](https://github.com/gerryvel/Motordaten/assets/17195231/e9192386-f672-4e20-b8e6-1d4efa33608e)


# PCB Layout
![grafik](https://github.com/gerryvel/NMEA2000-Data-Sender/assets/17195231/95a2bd04-50ca-4ac5-9da0-dfeb12f30040)

![grafik](https://github.com/gerryvel/NMEA2000-Data-Sender/assets/17195231/8f7d6ff9-ccad-4adb-b75d-5e98a85193fb)


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

- Adafruit Huzzah! ESP32 (for programming need USB-Adapter)
- SN65HVD230 [Link](https://www.reichelt.de/high-speed-can-transceiver-1-mbit-s-3-3-v-so-8-sn-65hvd230d-p58427.html?&trstct=pos_0&nbc=1)
- Traco-Power TSR 1-2450 for 12V / 5V [Link](https://www.reichelt.de/dc-dc-wandler-tsr-1-1-w-5-v-1000-ma-sil-to-220-tsr-1-2450-p116850.html?search=tsr+1-24)
- Resistor 3,3 KOhm [Link](https://www.reichelt.de/widerstand-kohleschicht-3-3-kohm-0207-250-mw-5--1-4w-3-3k-p1397.html?search=widerstand+250+mw+3k3) Other resistors are the same type! Click on "5% Carbon film resistors" then two times "+ more filter" to select values.
- Diode 1N4148 [Link](https://www.reichelt.de/schalt-diode-100-v-150-ma-do-35-1n-4148-p1730.html?search=1n4148)
- Zenerdiode 3,3 V [Link](https://www.reichelt.de/zenerdiode-3-3-v-0-5-w-do-35-zf-3-3-p23126.html?&trstct=pos_6&nbc=1)
- H11-L1 [Link](https://www.reichelt.de/optokoppler-1-mbit-s-dil-6-h11l1m-p219351.html?search=H11-l1)
- PCB by Aisler [Link](https://aisler.net/p/JCQLQVHC)

