#include <Arduino.h>
#include "state.h"


void stateInit() {
    pinMode(OUTGPIO0, OUTPUT);
    pinMode(OUTGPIO1, OUTPUT);
    pinMode(OUTGPIO2, OUTPUT);
    pinMode(OUTGPIO3, OUTPUT);
    stateEncode(IDLE);
    Serial.print("State output pins set to: "); 
    Serial.print(OUTGPIO0); Serial.print(" "); Serial.print(OUTGPIO1); Serial.print(" "); Serial.print(OUTGPIO2); Serial.print(" "); Serial.println(OUTGPIO3);
}


void stateEncode (enum State state) {
  Serial.print("Changing to state: "); Serial.println(state);
  switch (state) {
    // default state that does nothing is required, do not use
    case IDLE:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // Background index 0
    case BACK0:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // Background index 1
    case BACK1:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // Background index 2
    case BACK2:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // Background index 3
    case BACK3:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // Background index 4
    case BACK4:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // Background index 5
    case BACK5:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // RFID state
    case RFID:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // SUNRISE state
    case SUNRISE:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // SUNRISE state
    case SUNRISE_ACID:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // SUNRISE state
    case SUNRISE_MUSHROOM:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // SUNSET state
    case SUNSET:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // SUNSET state
    case SUNSET_ACID:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // SUNSET state
    case SUNSET_MUSHROOM:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    default:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
  }
}

State stateDecode() {
  int reading = (digitalReadFast(INGPIO3) * 8) + (digitalReadFast(INGPIO2) * 4) + (digitalReadFast(INGPIO1) * 2) + digitalReadFast(INGPIO0);
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