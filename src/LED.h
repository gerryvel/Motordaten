/**
 * @file LED.h
 * @author Gerry Sebb
 * @brief LED Ansteuerung
 * @version 2.1
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <arduino.h>
#include "task.h"

//Configuration LED
//const int LEDBoard = 2;  //DevModule
//const int LEDBoard = 13;   //Adafruit Huzzah32

enum LED {
  Red = 25, 
  Green = 26, 
  Blue = 33,
  LEDBoard = 13 //Adafruit Huzzah32
  };

void LEDblink(int PIN = LED()){
 taskBegin();
   while(1)   // blockiert dank der TaskPause nicht 
   {
      digitalWrite(PIN,HIGH);  // LED ein
      taskPause(250);   // gibt Rechenzeit ab         
      digitalWrite(PIN,LOW);   // LED aus
      taskPause(1000);   // gibt Rechenzeit ab         
   }
   taskEnd();   
}

void LEDflash(int PIN = LED()){
   taskBegin();
   while(1)   // blockiert dank der TaskPause nicht 
   {
      digitalWrite(PIN,HIGH);  // LED ein
      delay (5);
      //taskPause(2);   // gibt Rechenzeit ab    
      digitalWrite(PIN,LOW);   // LED aus
      taskPause(3000);   // gibt Rechenzeit ab    
   }
   taskEnd();   
}

void flashLED(int PIN = LED()) {
  if (millis() % 1000 > 500) {
    digitalWrite(PIN, HIGH);
  } else {
    digitalWrite(PIN, LOW);
  }
}


void LEDInit() {                        // Start Initialisierung
  pinMode(LED(Red),   OUTPUT);
  pinMode(LED(Blue),  OUTPUT);
  pinMode(LED(Green), OUTPUT);
  digitalWrite(LED(Red), HIGH);
  delay(250);
  digitalWrite(LED(Red), LOW);
  digitalWrite(LED(Blue), HIGH);
  delay(250);
  digitalWrite(LED(Blue), LOW);
  digitalWrite(LED(Green), HIGH);
  delay(250);
  digitalWrite(LED(Green), LOW);
}

void LEDon(int PIN = LED()) {
  digitalWrite(PIN, HIGH);    
}

void LEDoff(int PIN = LED()) {
  digitalWrite(PIN, LOW);
}

void LEDoff_RGB() {
  digitalWrite(LED(Blue), LOW);
  digitalWrite(LED(Green),LOW);
  digitalWrite(LED(Red), LOW);
}

