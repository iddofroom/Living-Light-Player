#ifndef __STATE_OUT_H__
#define __STATE_OUT_H__


#define OUTGPIO0 1
#define OUTGPIO1 2
#define OUTGPIO2 3
#define OUTGPIO3 4

enum State {IDLE, BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, RFID, SUNRISE, SUNRISE_ACID, SUNRISE_MUSHROOM, SUNSET, SUNSET_ACID, SUNSET_MUSHROOM};

void stateInitOutput();
void stateEncode(State state);


#endif // __STATE_OUT_H__