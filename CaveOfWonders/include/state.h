#ifndef __STATE_H__
#define __STATE_H__

#define OUTGPIO0 29
#define OUTGPIO1 30
#define OUTGPIO2 31
#define OUTGPIO3 32

enum State {IDLE, BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, BACK6, BACK7, BACK8, BACK9, RFID_CHIP};


void stateInit();
void stateEncode (State state);



#endif // __STATE_H__