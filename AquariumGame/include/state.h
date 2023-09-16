#ifndef __STATE_H__
#define __STATE_H__

#define OUTGPIO0 18
#define OUTGPIO1 19
#define OUTGPIO2 22
#define OUTGPIO3 23

enum State {IDLE,  BACK0, RFID, COIN, ON_ON, STAGE1, STAGE2, STAGE3, STAGE4};

void stateInit();
void stateEncode(State state);



#endif // __STATE_H__