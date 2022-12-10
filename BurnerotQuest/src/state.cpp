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
    case RFID_FAILED:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    // RFID state
    case RFID_SUCCESSFUL:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // RFID state
    case RFID_TOO_SOON:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    // RFID state
    case RFID_DONE:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    default:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
  }
}
