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
#include "sd.hpp"
#include "ui.hpp"
#include "queue.hpp"


#define DEBUG_WITH_SIMULATED_CAR false

#define SerialELM Serial2
ELM327 elmduino;

// Pierwszy CPU - OBD (non-blocking) i display
struct OBDTask {
    const char *name;

    void (*function)();

    unsigned long interval;
    unsigned long lastRun;
    bool saveToSDCard;
};

OBDTask *currentTask = nullptr;
unsigned long lastFuelTrimChartUpdate = 0;
float lastStft1 = 0;
float lastStft2 = 0;
float lastLtft1 = 0;
float lastLtft2 = 0;

void kphTask() {
    auto kph = elmduino.kph();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        ui_setSpeedValue(kph);
        queue_addToCSVQueue("kph", kph);
    }
}

void rpmTask() {
    auto rpm = elmduino.rpm();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        queue_addToCSVQueue("rpm", rpm);
    }
}

void ectTask() {
    auto ect = elmduino.engineCoolantTemp();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        queue_addToCSVQueue("ect", ect);
    }
}

void absLoadTask() {
    auto absLoad = elmduino.absLoad();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        queue_addToCSVQueue("absload", absLoad);
    }
}

void engineLoadTask() {
    auto engineLoad = elmduino.engineLoad();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        queue_addToCSVQueue("engineload", engineLoad);
    }
}

void stft1Task() {
    lastStft1 = elmduino.shortTermFuelTrimBank_1();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        ui_updateStft1Label(lastStft1);
        queue_addToCSVQueue("stft1", lastStft1);
    }
}

void stft2Task() {
    lastStft2 = elmduino.shortTermFuelTrimBank_2();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        ui_updateStft2Label(lastStft2);
        queue_addToCSVQueue("stft2", lastStft2);
    }
}

void ltft1Task() {
    lastLtft1 = elmduino.longTermFuelTrimBank_1();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        ui_updateLtft1Label(lastLtft1);
        queue_addToCSVQueue("ltft1", lastLtft1);
    }
}

void ltft2Task() {
    lastLtft2 = elmduino.longTermFuelTrimBank_2();
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        ui_updateLtft2Label(lastLtft2);
        queue_addToCSVQueue("ltft2", lastLtft2);
    }
}

void dtcTask() {
    elmduino.currentDTCCodes(false);
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        ui_updateWarningLabel("");
        for (const auto code: elmduino.DTC_Response.codes) {
            ui_updateWarningLabel(code, true);
        }
        ui_updateWarningLabel("DTCS: ", true);
    }
}

void testTask() {
    auto kph = random(0, 100);
    lastStft1 = random(-10,10);
    lastStft2 = random(-10,10);
    lastLtft1 = random(-10,10);
    lastLtft2 = random(-10,10);
    auto dtc = random(0, 200);

    ui_setSpeedValue(kph);
    ui_updateWarningLabel("DTC: P"+ dtc);
    ui_updateStft1Label(lastStft1);
    ui_updateStft2Label(lastStft2);
    ui_updateLtft1Label(lastLtft1);
    ui_updateLtft2Label(lastLtft2);
    ui_updateFuelTrimChart(lastStft1+lastLtft1, lastStft2 + lastLtft2);
    queue_addToCSVQueue("stft1", lastStft1);
    queue_addToCSVQueue("stft2", lastStft2);
    queue_addToCSVQueue("ltft1", lastLtft1);
    queue_addToCSVQueue("ltft2", lastLtft2);
}

#ifdef DEBUG_WITH_SIMULATED_CAR
static OBDTask tasks[1] = {
    OBDTask{"test", testTask, 50, 0},
    // OBDTask{"stft1", stft1Task, 50, 0},
    // OBDTask{"stft2", stft2Task, 50, 0},
    // OBDTask{"ltft1", ltft1Task, 50, 0},
    // OBDTask{"ltft2", ltft2Task, 50, 0},
    // OBDTask{"kph", kphTask, 100, 0},
    // OBDTask{"rpm", rpmTask, 100, 0},
    // OBDTask{"ect", ectTask, 100, 0},
    // OBDTask{"engineload", engineLoadTask, 100, 0},
    // OBDTask{"absLoadTask", absLoadTask, 100, 0},
    // OBDTask{"dtc", dtcTask, 5000, 0},
};
#else
static OBDTask tasks[10] = {
    OBDTask{"stft1", stft1Task, 50, 0},
    OBDTask{"stft2", stft2Task, 50, 0},
    OBDTask{"ltft1", ltft1Task, 50, 0},
    OBDTask{"ltft2", ltft2Task, 50, 0},
    OBDTask{"kph", kphTask, 100, 0},
    OBDTask{"rpm", rpmTask, 100, 0},
    OBDTask{"ect", ectTask, 100, 0},
    OBDTask{"engineload", engineLoadTask, 100, 0},
    OBDTask{"absLoadTask", absLoadTask, 100, 0},
    OBDTask{"dtc", dtcTask, 5000, 0},
};
#endif

void finalizeTaskIfDone() {
    if (elmduino.nb_rx_state == ELM_SUCCESS) {
        currentTask->lastRun = millis();
        currentTask = nullptr;
    } else if (elmduino.nb_rx_state != ELM_GETTING_MSG) {
        const auto nb_rx_state = elmduino.nb_rx_state;
        String error_message = "";
        if (nb_rx_state == ELM_SUCCESS)
            error_message = "ELM_SUCCESS";
        else if (nb_rx_state == ELM_NO_RESPONSE)
            error_message = "ERROR: ELM_NO_RESPONSE";
        else if (nb_rx_state == ELM_BUFFER_OVERFLOW)
            error_message = "ERROR: ELM_BUFFER_OVERFLOW";
        else if (nb_rx_state == ELM_UNABLE_TO_CONNECT)
            error_message = "ERROR: ELM_UNABLE_TO_CONNECT";
        else if (nb_rx_state == ELM_NO_DATA)
            error_message = "ERROR: ELM_NO_DATA";
        else if (nb_rx_state == ELM_STOPPED)
            error_message = "ERROR: ELM_STOPPED";
        else if (nb_rx_state == ELM_TIMEOUT)
            error_message = "ERROR: ELM_TIMEOUT";
        else if (nb_rx_state == ELM_BUFFER_OVERFLOW)
            error_message = "ERROR: BUFFER OVERFLOW";
        else if (nb_rx_state == ELM_GENERAL_ERROR)
            error_message = "ERROR: ELM_GENERAL_ERROR";
        else
            error_message = "No error detected";

        error_message = String("Task '") + currentTask->name + "' failed  - " + error_message;
        ui_updateWarningLabel(error_message.c_str());
        ui_loop();
        elmduino.printError();
        currentTask->lastRun = millis();
        currentTask = nullptr;
        delay(2000);
    }
}

void maybeSubmitFuelTrimChartChanges() {
    boolean submitFuelTrimChartUpdate = true;
    for (const auto &task: tasks) {
        const auto isFuelTrimTask = task.name == "stft1" || task.name == "stft2" || task.name == "ltft1" || task.name == "ltft2";
        if (isFuelTrimTask &&  task.lastRun < lastFuelTrimChartUpdate) {
            submitFuelTrimChartUpdate = false; // We haven't received updates from all tasks yet
            break;
        }
    }
    if (submitFuelTrimChartUpdate) {
        ui_updateFuelTrimChart(lastStft1 + lastLtft1, lastStft2 + lastLtft2);
        lastFuelTrimChartUpdate = millis();
    }
}

void executeOrPickNextTask() {
    unsigned long currentMillis = millis();
    if (currentTask != nullptr) {
        currentTask->function();
        finalizeTaskIfDone();
    } else {
        // pick next task
        unsigned long maxOverdue = 0;
        for (auto &task: tasks) {
            unsigned long expectedTime = task.lastRun + task.interval;
            if (currentMillis >= expectedTime) {
                unsigned long overdue = currentMillis - expectedTime;
                if (overdue > maxOverdue) {
                    maxOverdue = overdue;
                    currentTask = &task;
                }
            }
        }
        // ui_updateWarningLabel((String("TASK: ") + currentTask->name).c_str());
    }
}

bool connectOBD() {
    Serial.println("Connecting");
    if (!elmduino.begin(SerialELM, true, 5000)) {
        Serial.println("Couldn't connect to OBD scanner");
        return false;
    };
    Serial.println("Connected to OBD scanner");
    return true;
}

void setup() {
    delay(500);
    Serial.begin(115200);
    SerialELM.begin(38400, SERIAL_8N1, 15, 14);
    ui_setup();
    sd_setup();
    queue_setup();
}

bool connected = false;

void loop() {
    ui_loop();
    if (!connected && !DEBUG_WITH_SIMULATED_CAR) {
        ui_updateWarningLabel("Connecting...");
        ui_loop();
        connected = connectOBD();
        if (connected) {
            ui_updateWarningLabel("");
        }
        delay(200);
        return;
    }

    executeOrPickNextTask();
    maybeSubmitFuelTrimChartChanges();
}
