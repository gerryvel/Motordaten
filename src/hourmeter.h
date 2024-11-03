#ifndef _HOURMETER_H_
#define _HOURMETER_H_

#include <Arduino.h>
#include "configuration.h"

Preferences bsz1;

static unsigned long lastRun, CounterOld, milliRest;
int state1 = LOW;
int laststate1 = LOW;

unsigned long EngineHours(bool CountOn = 0){
    {
    long now = millis();
    milliRest += now - lastRun;
        if (CountOn == 1)
        {
            while (milliRest>=1000){
                Counter++;
                milliRest-=1000;
            }
        }
            else milliRest=0;
            lastRun = now;
            return Counter;
    }
    state1 = CountOn;
        if (laststate1 == HIGH && state1 == LOW){               // speichern bei Flanke negativ
            bsz1.begin("bsz", false);                               // NVS nutzen, BSZ erstellen
            CounterOld = preferences.getUInt("Start", 0);           // Speicher auslesen
            Counter = CounterOld + Counter;                         // Laufzeit alt + aktuell
            bsz1.putUInt("Start", Counter);                         // Speicher Schreiben
            bsz1.end();                                             // Preferences beenden
            state1 = LOW;                        
    }
        
}

#endif   