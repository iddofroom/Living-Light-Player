#ifndef __RFID_H__
#define __RFID_H__

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "state.h"

static const enum State RFID_STATE_MAP[] = {RFID_CHIP};
static const byte UID_TABLE[] = {0x56, 0xE5, 0x74, 0x45,        // Card group 1
                                 0xD6, 0xBF, 0x5A, 0x46,        // Card group 1
                                 0xF6, 0x3A, 0x77, 0x45,        // Card group 1
                                 0x16, 0x32, 0x35, 0x46,        // Card group 2
                                 0xE6, 0xC7, 0xDE, 0x45,        // Card group 2
                                 0x96, 0x05, 0x3A, 0x46,        // Card group 2
                                 0x46, 0xDF, 0x39, 0x46,        // Card group 3
                                 0xB6, 0xD6, 0x43, 0x46,        // Card group 3
                                 0xF5, 0x89, 0xDE, 0x5E,        // Card group 3
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
 * return true if the UID read is new, NUID are written to nuidPICC[],
 * return false if no card present or UID detected is not new
 */
bool rfidReadNuid(MFRC522 rfid, byte *nuidPICC, byte nuidSize);

/**
 * Check for the given UID in the UID_TABLE, if found returns the corresponding State from the RFID_STATE_MAP
 * otherwise return RFID_KIVSEE state as a generic RFID state
 */
enum State checkUidTable(byte UID[]);

void activateRec(MFRC522 rfid);

void clearInt(MFRC522 rfid);

uint8_t random(uint8_t min, uint8_t max);

#endif // __RFID_H__