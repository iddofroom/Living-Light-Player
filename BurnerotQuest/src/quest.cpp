
#include "quest.h"
#include "rfid.h"

byte nuidPICC[4] = {0x0, 0x0, 0x0, 0x0};
byte *p_nuidPICC = nuidPICC;

QuestState handleQuestLogic(MFRC522 &rfid)
{
    bool nuid_check_success = rfidReadNuid(rfid, p_nuidPICC, sizeof(nuidPICC));
    if (!nuid_check_success)
    {
        return QUEST_STATE_IDLE;
    }

    bool open_comm_success = openComm(rfid);
    if (!open_comm_success)
    {
        Serial.println(F("open comm failed"));
        clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
        return QUEST_STATE_FAILED;
    }

    byte stationIdFromChip;
    bool read_success = readStationIDfromChip(rfid, &stationIdFromChip);
    if (!read_success)
    {
        Serial.println(F("read failed"));
        clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
        return QUEST_STATE_FAILED;
    }
    Serial.print("read stationIdFromChip is: ");
    Serial.println(stationIdFromChip);

    if (stationIdFromChip == MY_LEVEL - 1)
    {
        // happy path
        bool write_success = writeStationIdToChip(rfid, MY_LEVEL);
        if (!write_success)
        {
            Serial.println(F("write failed"));
            clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
            return QUEST_STATE_FAILED;
        }
        else
        {
            Serial.print("wrote station ID to chip: ");
            Serial.println(MY_LEVEL);
            Serial.println("playing sound - good");
            closeComm(rfid);
            return QUEST_STATE_SUCCESSFUL;
        }
    }
    else
    {
        if (stationIdFromChip < (MY_LEVEL - 1))
        {
            // play sound - go back
            Serial.println("playing sound - bad, you missed a station");
            closeComm(rfid);
            return QUEST_STATE_TOO_SOON;
        }
        else
        {
            // play you were here before - no need to come back
            Serial.println("playing sound - bad, already visited");
            closeComm(rfid);
            return QUEST_STATE_DONE;
        }
    }
}

void clearQuestCurrUid () {
    clearStoredUid (p_nuidPICC, sizeof(nuidPICC));
}
