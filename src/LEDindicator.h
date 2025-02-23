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
if (motorErrorReported == "Aus" && coolantErrorReported == "Aus"){
  LEDflash(LED(Green)); // flash for loop run
  ErrorOff = true;
}
if (motorErrorReported == "Ein" || coolantErrorReported == "Ein"){
  LEDblink(LED(Red));
  ErrorOn = true;
}
if (!(ErrorOff && ErrorOn)){
      LEDflash(LED(Green));
      LEDflash(LED(Red));
}

/**
 * @brief WLAN failure
 * switch Blue led
 */
if (bConnect_CL == 0){
    LEDblink(LED(Blue));
} 
}
#endif   