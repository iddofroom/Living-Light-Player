#include <Arduino.h>
#include "state_out.h"


void stateInitOutput() {
    pinMode(OUTGPIO0, OUTPUT);
    pinMode(OUTGPIO1, OUTPUT);
    pinMode(OUTGPIO2, OUTPUT);
    pinMode(OUTGPIO3, OUTPUT);
    stateEncode(IDLE);
    Serial.print("State output pins set to: "); 
    Serial.print(OUTGPIO0); Serial.print(" "); Serial.print(OUTGPIO1); Serial.print(" "); Serial.print(OUTGPIO2); Serial.print(" "); Serial.println(OUTGPIO3);
}

// NOTE: logic is inverted due to relays on the way, we send HIGH to get LOW
void stateEncode (enum State state) {
  Serial.print("Changing to state: "); Serial.println(state);
  switch (state) {
    // default state that does nothing is required, do not use
    case IDLE:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // Background index 0
    case BACK0:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // Background index 1
    case BACK1:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // Background index 2
    case BACK2:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // Background index 3
    case BACK3:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // Background index 4
    case BACK4:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // Background index 5
    case BACK5:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // RFID state
    case RFID:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, HIGH);
      break;

    // SUNRISE state
    case SUNRISE:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // SUNRISE state
    case SUNRISE_ACID:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // SUNRISE state
    case SUNRISE_MUSHROOM:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // SUNSET state
    case SUNSET:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, LOW);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // SUNSET state
    case SUNSET_ACID:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      break;

    // SUNSET state
    case SUNSET_MUSHROOM:
      digitalWrite(OUTGPIO0, LOW);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, LOW);
      digitalWrite(OUTGPIO3, LOW);
      break;

    default:
      digitalWrite(OUTGPIO0, HIGH);
      digitalWrite(OUTGPIO1, HIGH);
      digitalWrite(OUTGPIO2, HIGH);
      digitalWrite(OUTGPIO3, HIGH);
  }
}
