#pragma once

typedef struct {
    unsigned long timestamp;
    char name[10];
    char value[10];
} CsvEntry;

QueueHandle_t csvEntriesQueue = nullptr;

void queue_consumeCSVQueue(void * pvParameters) {
    String bufferToWrite = "";
    CsvEntry entry;
    auto entriesCollected = 0;
    while (true) {
        if (csvEntriesQueue == nullptr) {
            delay(50);
            continue;
        }
        int ret = xQueueReceive(csvEntriesQueue, &entry, portMAX_DELAY);
        if (ret == pdPASS) {
            entriesCollected++;
            bufferToWrite += "\n" + String(entry.timestamp) + ";" + entry.name + ";" + entry.value;
            if (entriesCollected > 50) {
                appendToLogFile(bufferToWrite.c_str());
                bufferToWrite = "";
                entriesCollected = 0;
            }
        } else if (ret == pdFALSE) {
            ui_updateWarningLabel("Unable to consume from CSV queue");
            Serial.println("Unable to consume from CSV queue");
        }
    }
}

void queue_addToCSVQueue(const char *name, const char *value) {
    CsvEntry entry;
    entry.timestamp = millis();
    strncpy(entry.name, name, 10);
    strncpy(entry.value, value, 10);
    int publishResult = xQueueSend(csvEntriesQueue, (void *)&entry, 0);
    if (publishResult == pdTRUE) {
        // The message was successfully sent.
    } else if (publishResult == errQUEUE_FULL) {
        ui_updateWarningLabel("SD queue full!");
        Serial.println("SD queue full!");
    } else {
        ui_updateWarningLabel("Failed to send to CSV queue!");
        Serial.println("Failed to send to CSV queue!");
    }
}

void queue_addToCSVQueue(const char *name, const float value) {
    queue_addToCSVQueue(name, String(value).c_str());
}

void queue_addToCSVQueue(const char *name, const int value) {
    queue_addToCSVQueue(name, String(value).c_str());
}


void queue_setup() {
    csvEntriesQueue = xQueueCreate(1000, sizeof(CsvEntry)); // 100 elements max with 100 chars each
    if (csvEntriesQueue == NULL) {
        Serial.println("Queue could not be created. Halt.");
        while (1) {
            delay(1000);  // Halt at this point as is not possible to continue
        }
    }
    xTaskCreatePinnedToCore(queue_consumeCSVQueue, "ConsumeQueueTask", 4096, NULL, 1, NULL, 1);
}
