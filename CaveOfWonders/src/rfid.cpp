#include "rfid.h"

/**
 * Initialize SPI and RFID card reader, read the reader version, 
 * return true for supported types, false otherwise
 */
bool rfidInit(MFRC522 rfid)
{
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  /* read and printout the MFRC522 version (valid values 0x91 & 0x92)*/
  Serial.print(F("RFID Reader version: 0x"));
  byte readReg = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.println(readReg, HEX);
  if ((readReg == 0x91) || (readReg == 0x92))
  {
    return true;
  }
  return false;
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
bool rfidReadNuid(MFRC522 rfid, byte *nuidPICC, byte nuidSize)
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

  Serial.println(F("This code scans the MIFARE Classic NUID only."));
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3])
  {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < nuidSize; i++)
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    return true;
  }
  else
    Serial.println(F("Card read previously."));

  // Saving time, not Halting PICC as we are a single PICC so can stay active, no StopCrypto as we don't authenticate to read data
  // Halt PICC
  // rfid.PICC_HaltA();
  // Stop encryption on PCD
  // rfid.PCD_StopCrypto1();
  return false;
}

/**
 * Check if UID is in UID_TABLE
 * if found, return the State from the RFID_STATE_MAP corresponding to UID, 
 * Otherwise return RFID_KIVSEE State
 */
enum State checkUidTable(byte UID[])
{
  for (byte i = 0; i < (sizeof(UID_TABLE) / sizeof(UID_TABLE[0])); i = i + 4)
  {
    if (UID[0] == UID_TABLE[i] &&
        UID[1] == UID_TABLE[i + 1] &&
        UID[2] == UID_TABLE[i + 2] &&
        UID[3] == UID_TABLE[i + 3])
    {
      return RFID_STATE_MAP[i >> 2];
    }
  }
  return RFID_KIVSEE;
}

/**
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRec(MFRC522 rfid)
{
  rfid.PCD_WriteRegister(rfid.FIFODataReg, rfid.PICC_CMD_REQA);
  rfid.PCD_WriteRegister(rfid.CommandReg, rfid.PCD_Transceive);
  rfid.PCD_WriteRegister(rfid.BitFramingReg, 0x87);
}

/**
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 rfid)
{
  rfid.PCD_WriteRegister(rfid.ComIrqReg, 0x7F);
}

/**
 * Helper function to get a random number within range
 */
uint8_t random(uint8_t min, uint8_t max) {
   return (uint8_t) (min + rand() / (RAND_MAX / (max - min + 1) + 1));
}
