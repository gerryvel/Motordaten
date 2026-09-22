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

static unsigned long lastRun, milliRest;
static bool hourmeterInitialized = false;
int state1 = LOW, laststate1 = LOW;

/**
 * @brief Betriebstundenzähler
 * Berechnet Betriebstunden, wenn Anlage eingeschaltet ist
 * @param CountOn 
 * @return unsigned long 
 */
unsigned long EngineHours(bool CountOn = 0) {
    unsigned long now = millis();

    if (!hourmeterInitialized) {
        bsz1.begin("bsz", false);
        Counter = bsz1.getULong("Start", 0);
        bsz1.end();

        lastRun = now;
        laststate1 = CountOn;
        state1 = CountOn;
        hourmeterInitialized = true;
        return Counter;
    }

    if (CountOn) {
        milliRest += now - lastRun;
        Counter += milliRest / 1000;
        milliRest %= 1000;
    } else {
        milliRest = 0;
    }

    state1 = CountOn;
    if (laststate1 == HIGH && state1 == LOW) { // speichern bei Flanke negativ
        bsz1.begin("bsz", false);
        bsz1.putULong("Start", Counter);
        bsz1.end();
    }
    lastRun = now;
    laststate1 = state1; // Aktualisiere laststate1
    return Counter;
}

#endif