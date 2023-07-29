#include <Arduino.h>
#include <WString.h>        // Needs for structures
#include "configuration.h"


// Checksum calculation for NMEA
char CheckSum(String NMEAData) {
  char checksum = 0;
  // Iterate over the string, XOR each byte with the total sum
  for (int c = 0; c < NMEAData.length(); c++) {
    checksum = char(checksum ^ NMEAData.charAt(c));
  } 
  // Return the result
  return checksum;
}

/*
XDR
Transducer Values
            1 2   3 4       n
|   |   |   |       | \\
*  $--XDR,a,x.x,a,c--c, ..... *hh<CR><LF> \\

    Field Number:
    1) Transducer Type
    2) Measurement Data
    3) Units of measurement
    4) Name of transducer
    x) More of the same
    n) Checksum

    Example:
    Temperatur $IIXDR,C,19.52,C,TempAir*19
    Druck      $IIXDR,P,1.02481,B,Barometer*29
    Kraengung  $IIXDR,A,0,x.x,ROLL*hh<CR><LF>
*/

// Send Sensor data
String sendXDR()
{   
  String HexCheckSum;
  String NMEASensor;
  String SendSensor;
  
    NMEASensor = "IIXDR,A,";  //NMEASensor = "IIXDR,A," + String(SensorID);  
    //NMEASensorKraeng += ",";
    NMEASensor += String(fGaugeDrehzahl);
    NMEASensor += ",D,ROLL";

  // Build CheckSum
  HexCheckSum = String(CheckSum(NMEASensor), HEX);
  // Build complete NMEA string
  SendSensor = "$" + NMEASensor;
  SendSensor += "*";
  SendSensor += HexCheckSum;

  Serial.println(SendSensor);
  
  return SendSensor;
}

