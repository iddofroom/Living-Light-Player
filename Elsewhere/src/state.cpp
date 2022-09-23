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

    case KEY1:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case KEY2:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case BACK2:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case BACK3:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case BACK4:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case BACK5:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case BACK6:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, LOW);
      break;

    case BACK7:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case BACK8:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case BACK9:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case BACK10:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, LOW);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case BACK11:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case BACK12:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, LOW);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case BACK13:
      digitalWriteFast(OUTGPIO0, LOW);
      digitalWriteFast(OUTGPIO1, HIGH);
      digitalWriteFast(OUTGPIO2, HIGH);
      digitalWriteFast(OUTGPIO3, HIGH);
      break;

    case STOP:
      digitalWriteFast(OUTGPIO0, HIGH);
      digitalWriteFast(OUTGPIO1, HIGH);
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
