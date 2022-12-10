#ifndef __QUEST_H__
#define __QUEST_H__

#include "MFRC522.h"

// Should be set according to the station level
#define MY_LEVEL 1

enum QuestState {
    QUEST_STATE_IDLE = 0, // the chip should not change anything in the sequence
    QUEST_STATE_FAILED = 1, // we were unable to read / write the chip - should play "try again"
    QUEST_STATE_SUCCESSFUL = 2, // the chip is in the right station according to the quest logic
    QUEST_STATE_TOO_SOON = 3, // the chip needs to go to earlier station
    QUEST_STATE_DONE = 4, // the chip has already passed this station
};

QuestState handleQuestLogic(MFRC522 &rfid);

void clearQuestCurrUid();

#endif // __QUEST_H__