#include <Arduino.h>
#include "state_in.h"


void stateInitInput() {
    pinMode(INGPIO0, INPUT_PULLUP);
    pinMode(INGPIO1, INPUT_PULLUP);
    pinMode(INGPIO2, INPUT_PULLUP);
    pinMode(INGPIO3, INPUT_PULLUP);
    stateDecode();
    Serial.print("State input pins set to: "); 
    Serial.print(INGPIO0); Serial.print(" "); Serial.print(INGPIO1); Serial.print(" "); Serial.print(INGPIO2); Serial.print(" "); Serial.println(INGPIO3);
}

State stateDecode() {
  u_int8_t reading = (digitalRead(INGPIO3) * 8) + (digitalRead(INGPIO2) * 4) + (digitalRead(INGPIO1) * 2) + digitalRead(INGPIO0);
  switch (reading) {
    case 0:
      return IDLE;
    case 1:
      return BACK0;
    case 2:
      return BACK1;
    case 3:
      return BACK2;
    case 4:
      return BACK3;
    case 5:
      return BACK4;
    case 6:
      return BACK5;
    case 7:
      return RFID;
    case 8:
      return SUNRISE;
    case 9:
      return SUNRISE_ACID;
    case 10:
      return SUNRISE_MUSHROOM;
    case 11:
      return SUNSET;
    case 12:
      return SUNSET_ACID;
    case 13:
      return SUNSET_MUSHROOM;
    default:
      return IDLE;
  }
}