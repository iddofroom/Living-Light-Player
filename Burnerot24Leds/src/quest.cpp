
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

    questLoggerFile = SD.open("nerot24.log", FILE_WRITE);
}

QuestState handleQuestLogic(MFRC522 &rfid)
{
    bool nuid_check_success = rfidReadNuid(rfid, p_nuidPICC, sizeof(nuidPICC));
    if (!nuid_check_success)
    {
        return QUEST_STATE_IDLE;
    }

    questLoggerFile.print("chip unique ID: ");
    for (byte i = 0; i < 4; i++)
    {
        questLoggerFile.print(nuidPICC[i] < 0x10 ? " 0" : " ");
        questLoggerFile.print(nuidPICC[i], HEX);
    }
    questLoggerFile.println(" identified.");
    questLoggerFile.flush();

    return QUEST_STATE_DONE;
}

void clearQuestCurrUid()
{
    clearStoredUid(p_nuidPICC, sizeof(nuidPICC));
}
