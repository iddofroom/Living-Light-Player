#ifndef __RFID_H__
#define __RFID_H__

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "state.h"

/**
 * Initialization function, performs self test and returns result, true is OK, false is not
 */
bool rfidInit(MFRC522 &rfid);

/* Allow selected irq to be propagated to IRQ pin */
void allowRfidRxInt(MFRC522 &rfid);

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize);

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize);

/**
 * Check for RFID card presence and read its UID, 
 * return true if the UID read is new, NUID are written to nuidPICC[],
 * return false if no card present or UID detected is not new
 */
bool rfidReadNuid(MFRC522 &rfid, byte *nuidPICC, byte nuidSize);

void activateRfidReception(MFRC522 &rfid);

void clearRfidInt(MFRC522 &rfid);

void clearStoredUid(byte *nuidPICC, byte nuidSize);

uint8_t random(uint8_t min, uint8_t max);

bool openComm(MFRC522 &rfid);
void closeComm(MFRC522 &rfid);
bool readStationIDfromChip(MFRC522 &rfid, byte *stationIdFromChip);
bool writeStationIdToChip(MFRC522 &rfid, byte stationId);


#endif // __RFID_H__