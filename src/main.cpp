/*
SerialPassthrough sketch

  Some boards, like the Arduino 101, the MKR1000, Zero, or the Micro, have one
  hardware serial port attached to Digital pins 0-1, and a separate USB serial
  port attached to the IDE Serial Monitor. This means that the "serial
  passthrough" which is possible with the Arduino UNO (commonly used to interact
  with devices/shields that require configuration via serial AT commands) will
  not work by default.

  This sketch allows you to emulate the serial passthrough behaviour. Any text
  you type in the IDE Serial monitor will be written out to the serial port on
  Digital pins 0 and 1, and vice-versa.

  On the 101, MKR1000, Zero, and Micro, "Serial" refers to the USB Serial port
  attached to the Serial Monitor, and "Serial1" refers to the hardware serial
  port attached to pins 0 and 1. This sketch will emulate Serial passthrough
  using those two Serial ports on the boards mentioned above, but you can change
  these names to connect any two serial ports on a board that has multiple ports.

  created 23 May 2016
  by Erik Nyquist
*/

#include "ELMduino.h"
#include "Arduino.h"
#include "can.hpp"
#include "ui.hpp"
#include "sd.hpp"


#define SerialELM Serial2
ELM327 elmduino;

// Pierwszy CPU - OBD (non-blocking) i display
struct OBDTask {
    const char *name;
    void (*function)();
    unsigned long interval;
    unsigned long lastRun;
};

unsigned long lastFuelTrimChartUpdate = 0;
float lastStft1 = 0;
float lastStft2 = 0;
float lastLtft1 = 0;
float lastLtft2 = 0;

static OBDTask tasks[7] = {
    OBDTask{"kph", requestKmh, 50, 0},
    OBDTask{"rpm", requestRPM, 50, 0},
    OBDTask{"stft1", requestSTFT1, 50, 0},
    OBDTask{"stft2", requestSTFT2, 50, 0},
    OBDTask{"ltft1", requestLTFT1, 50, 0},
    OBDTask{"ltft2", requestLTFT2, 50, 0},
    OBDTask{"dtc", requestDTC, 5000, 0},
};

void maybeSubmitFuelTrimChartChanges() {
    boolean submitFuelTrimChartUpdate = true;
    for (const auto &task: tasks) {
        const auto isFuelTrimTask = task.name == "stft1" || task.name == "stft2" || task.name == "ltft1" || task.name == "ltft2";
        if (isFuelTrimTask && task.lastRun < lastFuelTrimChartUpdate) {
            submitFuelTrimChartUpdate = false; // We haven't received updates from all tasks yet
            break;
        }
    }
    if (submitFuelTrimChartUpdate) {
        ui_updateFuelTrimChart(lastStft1 + lastLtft1, lastStft2 + lastLtft2);
        lastFuelTrimChartUpdate = millis();
    }
}

void executeTasks() {
    unsigned long currentMillis = millis();
    for (auto &task: tasks) {
        if (currentMillis >= (task.lastRun + task.interval)) {
            task.lastRun = currentMillis;
            task.function();
        }
    }
}

void setup() {
    delay(500);
    Serial.begin(115200);
    SerialELM.begin(38400, SERIAL_8N1, 45, 39);
    ui_setup();
    // sd_setup();
    can_setup();
}

void loop() {
    ui_loop();
    can_loop();
    executeTasks();
    maybeSubmitFuelTrimChartChanges();
}

void onKphUpdated(int kph) {
    ui_setSpeedValue(kph);
}

void onStft1Updated(float trim) {
    lastStft1 = trim;
    ui_updateStft1Label(trim);
}

void onStft2Updated(float trim) {
    lastStft2 = trim;
    ui_updateStft2Label(trim);
}

void onLtft1Updated(float trim) {
    lastLtft1 = trim;
    ui_updateLtft1Label(trim);
}

void onLtft2Updated(float trim) {
    lastLtft2 = trim;
    ui_updateLtft2Label(trim);
}