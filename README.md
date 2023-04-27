# Bootsdaten

The ESP32 in this project is an Adafruit Huzzah! ESP32. This is a small module without USB connector.

With the ESP32 and 2 Sensors for gyro (MMA8452) and distance (Sharp GP2Y0A21) it's possible
monitoring the varaible Kiel and the Inclined position (Krängung, Roll). 
The Roll-data are available as NMEA0183 UDP-Stream over Wlan.

The 12 Volt is reduced to 5 Volt with a DC Step-Down_Converter. 12V DC comes from the N2k Bus Connector with the M12 Connector.

- Adafruit Huzzah! ESP32 (for programming need USB-Adapter)
- Traco-Power TSR 1-2450 for 12V / 5V
- RGB LED Kingbright L-154A4SURKQBDZGW
- Waveshare SN65HVD230 Can Board (adapted, cutted the (also without) connector)
- Case Wago 789


The Website use SPIFFS Filesystem. You must use Partition Schemes "Minimal SPIFFS with APPS and OTA".
The HTML Data upload separately with "ESP 32 Scetch Data upload" from /data directory.

![Schaltplan PDF](https://github.com/gerryvel/Bootsdaten/blob/main/ESP32%20Bootselektronik.pdf)

![Schematics](https://github.com/gerryvel/Bootsdaten/blob/main/Zwischenablage01.jpg)

![Schematics](https://github.com/gerryvel/Bootsdaten/blob/main/Zwischenablage02.jpg)

![Schematics](https://github.com/gerryvel/Bootsdaten/blob/main/Zwischenablage03.jpg)

![Schematics](https://github.com/gerryvel/Bootsdaten/blob/main/Zwischenablage04.jpg)
