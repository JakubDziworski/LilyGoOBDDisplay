/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"

String logFileName = "";

int extractFileNumber(const File &file) {
    String numStr = "";
    String fileName = file.name();
    if (!fileName.endsWith(".csv")) {
        return -1;
    }
    auto fileNameNoExtension = fileName.substring(0, fileName.length() - 4);
    for (int i = 0; i < fileNameNoExtension.length(); i++) {
        if (isDigit(fileNameNoExtension[i])) {
            numStr += fileNameNoExtension[i];
        } else {
            return -1;
        }
    }
    return numStr.toInt();
}

int appendToLogFile(const char *message) {
    if (logFileName.isEmpty()) {
        Serial.println("No log file set yet");
        return -1;
    }

    File file = SD.open(logFileName, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return -1;
    }
    if (!file.print(message)) {
        Serial.println("Append failed");
        return -1;
    }
    file.close();
    return 0;
}


void createNewLogFile() {
    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    int highestFileNumber = 0;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            highestFileNumber = max(extractFileNumber(file), highestFileNumber);
        }
        file = root.openNextFile();
    }

    logFileName = String("/") + (highestFileNumber + 1) + ".csv";
    Serial.println("Creating new log: " + logFileName);
    auto newFile = SD.open(logFileName, FILE_WRITE);
    newFile.close();
}

void sd_setup() {
    if (!SD.begin()) {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

    createNewLogFile();
}
