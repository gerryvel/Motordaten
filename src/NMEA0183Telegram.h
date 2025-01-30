/**
 * @file NMEA0183Telegram.h
 * @author Gerry Sebb
 * @brief NMEA0183 Telegrame senden
 * @version 1.0
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <Arduino.h>
#include <WString.h>        // Needs for structures
#include "configuration.h"

/**
 * @brief Checksum calculation for NMEA
 * 
 * @param NMEAData 
 * @return char 
 */

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


  RPM - Revolutions

        1 2 3   4   5 6
        | | |   |   | |
 $--RPM,a,x,x.x,x.x,A*hh<CR><LF> 

    Field Number: 
      1) Sourse, S = Shaft, E = Engine
      2) Engine or shaft number
      3) Speed, Revolutions per minute
      4) Propeller pitch, % of maximum, "-" means astern
      5) Status, A means data is valid
      6) Checksum 

*/

/**
 * @brief Send NMEA0183
 * Send XDR Sensor data
 * @return String 
 */

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

/**
 * @brief Send NMEA0183
 * Send RPM Sensor data
 * @return String 
 */

String sendRPM()
{   
  String HexCheckSum;
  String NMEASensor;
  String SendSensor;
  
    NMEASensor = "IIRPM,E,1,";  //NMEASensor = "IIXDR,E,1," + String(SensorID);  
    NMEASensor += String(fGaugeDrehzahl);
    NMEASensor += ",15,A";

  // Build CheckSum
  HexCheckSum = String(CheckSum(NMEASensor), HEX);
  // Build complete NMEA string
  SendSensor = "$" + NMEASensor;
  SendSensor += "*";
  SendSensor += HexCheckSum;

  Serial.println(SendSensor);
  
  return SendSensor;
}
