#ifndef __STATE_H__
#define __STATE_H__

#define OUTGPIO0 29
#define OUTGPIO1 30
#define OUTGPIO2 31
#define OUTGPIO3 32

enum State {IDLE, KEY1, KEY2, BACK2, BACK3, BACK4, BACK5, BACK6, BACK7, BACK8, BACK9, BACK10, BACK11, BACK12, BACK13, STOP};


void stateInit();
void stateEncode (State state);



#endif // __STATE_H__