#ifndef __MFRC522_FUNC_H__
#define __MFRC522_FUNC_H__

#include <MFRC522.h>

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
/**
 * authenticate the comm channel with the RFID chip, all read/write actions must do this before they can act
 */
bool authenticate(byte trailerBlock, MFRC522::MIFARE_Key *key, MFRC522 &mfrc522) {
    MFRC522::StatusCode status;

    // Authenticate using key A
    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    return true;
}
/**
 * Read a data block of `size` from the RFID chip to a buffer
 */
bool read_block(byte blockAddr, byte buffer[], byte size, MFRC522 &mfrc522) {
    MFRC522::StatusCode status;
    
    // Read data from the block
    // Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    // Serial.println(F(" ..."));
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    // Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    // dump_byte_array(buffer, 16); Serial.println();
    // Serial.println();
    return true;
}
/**
 * Write a dataBlock of `size` to the RFID chip and read it back to verify it was written properly
 */
bool write_and_verify(byte blockAddr, byte dataBlock[], byte buffer[], byte size, MFRC522 &mfrc522) {
    MFRC522::StatusCode status;

    // Write data to the block
    // Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
    // Serial.println(F(" ..."));
    // dump_byte_array(dataBlock, 16); Serial.println();
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    Serial.println();

    // Read data from the block (again, should now be what we have written)
    // Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    // Serial.println(F(" ..."));
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    // Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    // dump_byte_array(buffer, 16); Serial.println();

    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == dataBlock[i])
            count++;
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 16) {
        Serial.println(F("Success :-)"));
        return true;
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
        Serial.println();
        return false;
    }
}
/**
 * Compare two UIDs and check if they are identical
 */
bool UIDcompare (byte prevUID[], byte currUID[], int UIDLen) {
    for (int i = 0; i < UIDLen; i++) {
        if (prevUID[i] != currUID[i])
            return false;
    }
    return true;
}

#endif // __MFRC522_FUNC_H__