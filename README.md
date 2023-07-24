# Bootsdaten

The ESP32 in this project is an Adafruit Huzzah! ESP32. This is a small module without USB connector.

With the ESP32 and 2 Sensors for gyro (MMA8452) and distance (Sharp GP2Y0A21) it's possible
monitoring the varaible Kiel and the Inclined position (Krängung, Roll). 
The Roll-data are available as NMEA0183 UDP-Stream over Wlan.

The 12 Volt is reduced to 5 Volt with a DC Step-Down_Converter. 12V DC comes from the N2k Bus Connector with the M12 Connector.

The Website use LittleFS Filesystem. You must use Partition Schemes "Minimal SPIFFS with APPS and OTA".
The HTML Data upload separately with 
- "ESP 32 Skcetch Data upload" (Arduino IDE) or 
- PlatformIO > Build Filesystem and Upload Filesystem Image (PlatformIO) 
from /data directory.

# Partlist:

- Adafruit Huzzah! ESP32 (for programming need USB-Adapter)[Link](https://www.exp-tech.de/plattformen/internet-of-things-iot/9350/adafruit-huzzah32-esp32-breakout-board)
- SN65HVD230 [Link](https://www.reichelt.de/high-speed-can-transceiver-1-mbit-s-3-3-v-so-8-sn-65hvd230d-p58427.html?&trstct=pos_0&nbc=1)
- Traco-Power TSR 1-2450 for 12V / 5V [Link](https://www.reichelt.de/dc-dc-wandler-tsr-1-1-w-5-v-1000-ma-sil-to-220-tsr-1-2450-p116850.html?search=tsr+1-24)
- Gyro MMA8452Q [Link](https://www.reichelt.de/entwicklerboards-beschleunigungsmesser-board-mma8452q-debo-sens-acc3-p284397.html)
- Case Wago 789
- Resistor 200Ohm , 10kOhm


# Wiring diagram

![BootsdatenKiCAD.pdf](https://github.com/gerryvel/Bootsdaten/files/12145147/BootsdatenKiCAD.pdf)

# PCB

![Bootsdaten](https://github.com/gerryvel/Bootsdaten/assets/17195231/b4be1809-5393-4396-8dcf-747c5ca8a09e)
PCB by Aisler [Link](https://aisler.net/p/NZFHAMAJ)

# Webinterface

![Zwischenablage01](https://user-images.githubusercontent.com/17195231/234933514-95c5519c-ce94-45df-af15-64128691161c.jpg)

![Zwischenablage02](https://user-images.githubusercontent.com/17195231/234933530-b59c1f4c-b747-41a3-8f6a-eba9062560dc.jpg)

![Zwischenablage03](https://user-images.githubusercontent.com/17195231/234933552-55ede022-9682-486e-8518-c98acedd2c1a.jpg)

![Zwischenablage04](https://user-images.githubusercontent.com/17195231/234933563-c5276110-f2e7-4a71-a5f1-1a3fbc7df484.jpg)
