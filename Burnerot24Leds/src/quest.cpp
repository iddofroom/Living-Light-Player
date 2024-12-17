
#include "quest.h"
#include "rfid.h"
#include "SD.h"

byte nuidPICC[4] = {0x0, 0x0, 0x0, 0x0};
byte *p_nuidPICC = nuidPICC;

File questLoggerFile;

void initSdWriter()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println(F("SD card begin() failed"));
        return;
    }
    Serial.println(F("SD card begin() success"));

    questLoggerFile = SD.open("questlog.txt", FILE_WRITE);
}

QuestState handleQuestLogic(MFRC522 &rfid)
{
    bool nuid_check_success = rfidReadNuid(rfid, p_nuidPICC, sizeof(nuidPICC));
    if (!nuid_check_success)
    {
        return QUEST_STATE_IDLE;
    }
    // NO NEED TO OPEN COMM, WE DON'T READ CHIP DATA
    // bool open_comm_success = openComm(rfid);
    // if (!open_comm_success)
    // {
    //     Serial.println(F("open comm failed"));
    //     clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
    //     return QUEST_STATE_FAILED;
    // }

    // byte stationIdFromChip;
    // bool read_success = readStationIDfromChip(rfid, &stationIdFromChip);
    // if (!read_success)
    // {
    //     Serial.println(F("read failed"));
    //     clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
    //     return QUEST_STATE_FAILED;
    // }
    // Serial.print("read stationIdFromChip is: ");
    // Serial.println(stationIdFromChip);

    // questLoggerFile.print("read chip with level " + String(stationIdFromChip) + " at station " + String(MY_LEVEL) + " chip unique ID: ");
    questLoggerFile.print("chip unique ID: ");
    for (byte i = 0; i < 4; i++)
    {
        questLoggerFile.print(nuidPICC[i] < 0x10 ? " 0" : " ");
        questLoggerFile.print(nuidPICC[i], HEX);
    }

    // if (stationIdFromChip == MY_LEVEL - 1)
    // {
    //     // happy path
    //     bool write_success = writeStationIdToChip(rfid, MY_LEVEL);
    //     if (!write_success)
    //     {
    //         Serial.println(F("write failed"));
    //         clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
    //         questLoggerFile.println(" tried to write station id" + String(MY_LEVEL) + " to chip. it FAILED");
    //         questLoggerFile.flush();
    //         return QUEST_STATE_FAILED;
    //     }
    //     else
    //     {
    //         Serial.print("wrote station ID to chip: ");
    //         Serial.println(MY_LEVEL);
    //         Serial.println("playing sound - good");
    //         questLoggerFile.println(" wrote station id " + String(MY_LEVEL) + " to chip. this is good");
    //         questLoggerFile.flush();
    //         closeComm(rfid);
    //         return QUEST_STATE_SUCCESSFUL;
    //     }
    // }
    // else
    // {
    //     if (stationIdFromChip < (MY_LEVEL - 1))
    //     {
    //         // play sound - go back
    //         Serial.println("playing sound - bad, you missed a station");
    //         questLoggerFile.println(" did not write to chip. chip is too far back");
    //         questLoggerFile.flush();
    //         closeComm(rfid);
    //         return QUEST_STATE_TOO_SOON;
    //     }
    //     else
    //     {
    //         // play you were here before - no need to come back
    //         Serial.println("playing sound - bad, already visited");
    //         questLoggerFile.println(" did not write to chip. chip was already in this station");
    //         questLoggerFile.flush();
    //         closeComm(rfid);
    //         return QUEST_STATE_DONE;
    //     }
    // }
    
    return QUEST_STATE_DONE;
}

void clearQuestCurrUid()
{
    clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
}
