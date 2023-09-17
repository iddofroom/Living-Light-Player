#ifndef __STATE_H__
#define __STATE_H__

#define INGPIO0 18
#define INGPIO1 19
#define INGPIO2 22
#define INGPIO3 23

#define OUTGPIO0 1
#define OUTGPIO1 2
#define OUTGPIO2 3
#define OUTGPIO3 4

enum State {IDLE, BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, RFID, SUNRISE, SUNRISE_ACID, SUNRISE_MUSHROOM, SUNSET, SUNSET_ACID, SUNSET_MUSHROOM};

void stateInit();
void stateEncode(State state);
State stateDecode();



#endif // __STATE_H__