#ifndef __STATE_H__
#define __STATE_H__

#define OUTGPIO0 18
#define OUTGPIO1 19
#define OUTGPIO2 22
#define OUTGPIO3 23

enum State {IDLE, BACK0, BACK1, BACK2, BACK3, BACK4, BACK5, RFID_FAILED, RFID_SUCCESSFUL, RFID_TOO_SOON, RFID_DONE, LIGHT_TRIGGERED};

void stateInit();
void stateEncode(State state);



#endif // __STATE_H__