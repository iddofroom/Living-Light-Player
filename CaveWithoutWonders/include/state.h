#ifndef __STATE_H__
#define __STATE_H__

#define OUTGPIO0 18
#define OUTGPIO1 19
#define OUTGPIO2 22
#define OUTGPIO3 23

enum State {IDLE, BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, BACK6, BACK7, BACK8, BACK9, BACK10, BACK11, BACK12, BACK13, BACK14};


void stateInit();
void stateEncode (State state);



#endif // __STATE_H__