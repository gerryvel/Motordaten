#ifndef _UPTIME_H_
#define _UPTIME_H_

#include <Arduino.h>
#include <Preferences.h>

Preferences BszMemory; 

unsigned long StartTime = 0;
unsigned long DurationTime = 0;
unsigned long DurationTimeOld = 0;
unsigned long Engineminutes = (DurationTime / 60000);  // Millisekunde in Minuten
unsigned long Enginehours = (DurationTime / 3600000);  // Millisekunde in Stunden
int state = LOW;
int laststate = LOW;

void BSZ(bool EngineOn = 0){                        // BetriebStundenZähler
    state = EngineOn;
    if (laststate == LOW && state == HIGH){         // Flanke positiv erkennen
        StartTime = millis();                       // Zeit auslesen
    }
    DurationTime = millis()-StartTime;                  // Laufzeit berechnen
    state = LOW;                                      // Bit zrücksetzen

    if (laststate == HIGH && state == LOW){         // speichern bei Flanke negativ
        BszMemory.begin("bsz", false);              // NVS nutzen, BSZ erstellen
        DurationTimeOld = BszMemory.getUInt("Start", 0);    // Speicher auslesen
        DurationTime = DurationTimeOld + DurationTime;          // Laufzeit alt + aktuell
        BszMemory.putUInt("Start", DurationTime);       // Speicher Schreiben
        BszMemory.end();                            // Preferences beenden
        state = LOW;                        
}
}

#endif   