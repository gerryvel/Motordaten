#ifndef _UPTIME_H_
#define _UPTIME_H_

#include <Arduino.h>
#include "configuration.h"

Preferences bsz;

unsigned long StartTime = 0;
unsigned long DurationTime = 0;
unsigned long DurationTimeOld = 0;
unsigned long Engineminutes = (DurationTime / 60000);  // Millisekunde in Minuten
unsigned long Enginehours = (DurationTime / 3600000);  // Millisekunde in Stunden
int state = LOW;
int laststate = LOW;

void BSZ(bool PinOn = 0){                            // BetriebStundenZähler
    state = PinOn;
    if (laststate == LOW && state == HIGH){             // Flanke positiv erkennen
        StartTime = millis();                           // Zeit auslesen
    }
    DurationTime = millis()-StartTime;                  // Laufzeit berechnen
    state = LOW;                                        // Bit zrücksetzen

    if (laststate == HIGH && state == LOW){             // speichern bei Flanke negativ
        bsz.begin("bsz", false);                  // NVS nutzen, BSZ erstellen
        DurationTimeOld = preferences.getUInt("Start", 0);    // Speicher auslesen
        DurationTime = DurationTimeOld + DurationTime;      // Laufzeit alt + aktuell
        bsz.putUInt("Start", DurationTime);       // Speicher Schreiben
        bsz.end();                                // Preferences beenden
        state = LOW;                        
    }
  Serial.printf("Betriebszeit: %d min\n", Engineminutes);
  Serial.printf("Betriebszeit: %d h\n", Enginehours);
}

#endif   