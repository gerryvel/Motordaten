#ifndef _HOURMETER_H_
#define _HOURMETER_H_

/**
 * @file hourmeter.h
 * @author Gerry Sebb
 * @brief Betriebstundenzähler
 * @version 1.0
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <Arduino.h>
#include "configuration.h"

Preferences bsz1;

static unsigned long lastRun, CounterOld, milliRest;
int state1 = LOW, laststate1 = LOW;

/**
 * @brief Betriebstundenzähler
 * Berechnet Betriebstunden, wenn Anlage eingeschaltet ist
 * @param CountOn 
 * @return unsigned long 
 */
unsigned long EngineHours(bool CountOn = 0) {
    unsigned long now = millis();
    milliRest += now - lastRun;
    if (CountOn == 1) {
        while (milliRest >= 1000) {
            Counter++;
            milliRest -= 1000;
        }
    } else {
        milliRest = 0;
    }
    lastRun = now;

    state1 = CountOn;
    if (laststate1 == HIGH && state1 == LOW) { // speichern bei Flanke negativ
        bsz1.begin("bsz", false); // NVS nutzen, BSZ erstellen, lesen und schreiben (false)
        CounterOld = bsz1.getUInt("Start", 0); // Speicher auslesen
        Counter = CounterOld + Counter; // Laufzeit alt + aktuell
        bsz1.putUInt("Start", Counter); // Speicher schreiben
        bsz1.end(); // Preferences beenden
    }
    laststate1 = state1; // Aktualisiere laststate1
    return Counter;
}

#endif