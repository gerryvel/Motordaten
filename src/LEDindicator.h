/**
 * @file LEDindicator.h
 * @author Gerry Sebb
 * @brief LED Betriebsanzeige
 * @version 1.0
 * @date 2025-02-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _LEDINDICATOR_
#define _LEDINDICATOR_

# include <Arduino.h>
# include "configuration.h"
# include "LED.h"


/**
 * @brief Sensor failure
 * switch LED green/red
 */
bool ErrorOff = false;
bool ErrorOn = false;

void LoopIndicator(){
  // Reset Error flags
  ErrorOff = false;
  ErrorOn = false;

if (MotorTemp != -5.0 && CoolantTemp != -5.0){  
  ErrorOff = true;
}
if (MotorTemp == -5.0 || CoolantTemp == -5.0){
  ErrorOn = true;
}
if (ErrorOff == true ){
  LEDflash(LED(Green)); // flash for loop run without temp-failure
}
if (ErrorOn == true){
  LEDblink(LED(Red));
}
}

#endif   