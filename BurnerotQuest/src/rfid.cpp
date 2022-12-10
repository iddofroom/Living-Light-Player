
#include "rfid.h"
#include <MFRC522_func.h>

#define QUEST_BYTE_INDEX 1
#define RFID_BLOCK_SIZE 18
#define EVENT_ID_BIT 3   // burnerot 2022 is bit 3, count starts at 0
#define RFID_BLOCK_IN_USE 4

MFRC522::MIFARE_Key authKey;
byte blockAddr      = RFID_BLOCK_IN_USE;
byte buffer[RFID_BLOCK_SIZE];
byte size = sizeof(buffer);
byte trailerBlock   = 7;



/**
 * Initialize SPI and RFID card reader, read the reader version, 
 * return true for supported types, false otherwise
 */
bool rfidInit(MFRC522 &rfid)
{
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  for (byte i = 0; i < 6; i++) {
    authKey.keyByte[i] = 0xFF;
  }
  /* read and printout the MFRC522 version (valid values 0x91 & 0x92)*/
  Serial.print(F("RFID Reader version: 0x"));
  byte readReg = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.println(readReg, HEX);
  if ((readReg == 0x91) || (readReg == 0x92) || (readReg == 0xB2))
  {
    return true;
  }
  return false;
}

/* Allow selected irq to be propagated to IRQ pin */
void allowRfidRxInt(MFRC522 &rfid) 
{
  byte regVal = 0xA0; // select rx irq
  rfid.PCD_WriteRegister(rfid.ComIEnReg, regVal);
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

/**
 * Check for card presence, read its UID and compare it with nuidPICC stored
 * return true if card read is new UID, false in all other cases
 */
bool rfidReadNuid(MFRC522 &rfid, byte *nuidPICC, byte nuidSize)
{
  // Skip RFID if no new card present on the sensor/reader. This saves the entire process when idle.
  // if (!rfid.PICC_IsNewCardPresent()) {
  //   Serial.println(F("No card presence detected."));
  //   return false;
  // }

  // Verify if the NUID has been read
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println(F("Can't read card UID."));
    return false;
  }

  // Serial.println(F("This code scans the MIFARE Classic NUID only."));
  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
  }

  // check the new UID is not the same as the previous one
  bool uidMatch = true;
  for (int i = 0; i < nuidSize; i++) {
      if (nuidPICC[i] != rfid.uid.uidByte[i])
          uidMatch = false;
  }

  if (uidMatch) {
    Serial.println(F("Card previously read."));
    return false;
  }
  else {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < nuidSize; i++)
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    // Serial.println(F("The NUID tag is:"));
    // Serial.print(F("In hex: "));
    // printHex(rfid.uid.uidByte, rfid.uid.size);
    // Serial.println();
    // Serial.print(F("In dec: "));
    // printDec(rfid.uid.uidByte, rfid.uid.size);
    // Serial.println();
    return true;
  }

  // Saving time, not Halting PICC as we are a single PICC so can stay active, no StopCrypto as we don't authenticate to read data
  // Halt PICC
  // rfid.PICC_HaltA();
  // Stop encryption on PCD
  // rfid.PCD_StopCrypto1();
}

void clearStoredUid (byte *uid, byte nuidSize) {
  for (byte i = 0; i < nuidSize; i++)
    {
      uid[i] = 0x0;
    }
}

/**
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRfidReception(MFRC522 &rfid)
{
  rfid.PCD_WriteRegister(rfid.FIFODataReg, rfid.PICC_CMD_REQA);
  rfid.PCD_WriteRegister(rfid.CommandReg, rfid.PCD_Transceive);
  rfid.PCD_WriteRegister(rfid.BitFramingReg, 0x87);
}

/**
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearRfidInt(MFRC522 &rfid)
{
  rfid.PCD_WriteRegister(rfid.ComIrqReg, 0x7F);
}

/**
 * Helper function to get a random number within range
 */
uint8_t random(uint8_t min, uint8_t max) {
   return (uint8_t) (min + rand() / (RAND_MAX / (max - min + 1) + 1));
}

bool openComm(MFRC522 &rfid) {
 // perform authentication to open communication
  bool auth_success = authenticate(trailerBlock, &authKey, rfid);
  if (!auth_success) {
    closeComm(rfid);
    return false;
  }

  return true;
}

void closeComm(MFRC522 &rfid) {
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
}

/**
 * given an RFID instance with an opened comm, read the station ID byte from the chip
 */
bool readStationIDfromChip(MFRC522 &rfid, byte *stationIdFromChip) {

  // read the tag to get coded information to buffer
  bool read_success = read_block(blockAddr, buffer, size, rfid);
  if (!read_success) {
    closeComm(rfid);
    return false;
  }

  byte stationId = buffer[QUEST_BYTE_INDEX];

  // handle uninitialized rfid chips 
  if(stationId == 0xff) {
    stationId = 0x00;
  }

  *stationIdFromChip = stationId;

  return true;
}

bool writeStationIdToChip(MFRC522 &rfid, byte stationId) {

  byte newChipData[RFID_BLOCK_SIZE];
  memcpy(newChipData, buffer, RFID_BLOCK_SIZE);

  newChipData[QUEST_BYTE_INDEX] = stationId;

  byte eventTracker = newChipData[15]; // event tracker is at byte 15, counting from 0
  eventTracker |= (1 << EVENT_ID_BIT);
  newChipData[15] = eventTracker;

  bool write_success = write_and_verify(blockAddr, newChipData, buffer, size, rfid);
  if(!write_success) {
    closeComm(rfid);
    return false;
  }

  return true;
}
