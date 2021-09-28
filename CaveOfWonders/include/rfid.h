#ifndef __RFID_H__
#define __RFID_H__

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "state.h"

static const enum State RFID_STATE_MAP[] = {RFID_QUEEN, RFID_UNDER, RFID_COME};
static const byte UID_TABLE[] = {0x46, 0xDF, 0x39, 0x46,        // Card 9
                                 0x56, 0xE5, 0x74, 0x45,        // Card 1
                                 0x79, 0xA0, 0x29, 0x52};       // KivSee chain

/**
 * Initialization function, performs self test and returns result, true is OK, false is not
 */
bool rfidInit(MFRC522 rfid);

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
 * return true for new card read, NUID written to nuidPICC[],
 * return false if no card present or no new UID detected
 */
bool rfidReadNuid(MFRC522 rfid, byte *nuidPICC, byte nuidSize);

/**
 * Read RFID card UID after recieving an iterrupt from the reader that a card is present, 
 * return true for new UID read vs nuidPICC[] stored, read UID overwrites current nuidPICC[],
 * return false if UID read is not new, can't be read or not of supported type
 */
bool rfidReadNuidInt(MFRC522 rfid, byte *nuidPICC, byte nuidSize);

/**
 * Check for the given UID in the UID_TABLE, if found returns the corresponding State from the RFID_STATE_MAP
 * otherwise return RFID_KIVSEE state as a generic RFID state
 */
enum State checkUidTable(byte UID[]);

void activateRec(MFRC522 rfid);
void clearInt(MFRC522 rfid);

#endif // __RFID_H__