#include "mcp2515.h"
#include "Arduino.h"


MCP2515 mcp2515(SS);
can_frame canFrame;

void can_setup() {
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS);
    // mcp2515.setLoopbackMode();
    mcp2515.setNormalMode();
}

// void sendTestMessage() {
//     struct can_frame canMsg1;
//     canMsg1.can_id  = 0x0F6;
//     canMsg1.can_dlc = 8;
//     canMsg1.data[0] = 0x8E;
//     canMsg1.data[1] = 0x87;
//     canMsg1.data[2] = 0x32;
//     canMsg1.data[3] = 0xFA;
//     canMsg1.data[4] = 0x26;
//     canMsg1.data[5] = 0x8E;
//     canMsg1.data[6] = 0xBE;
//     canMsg1.data[7] = 0x86;
//     mcp2515.sendMessage(&canMsg1);
//     Serial.println("Can message sent");
// }

void requestOBD(byte pid) {
    can_frame canMsg = {};
    canMsg.can_id  = 0x7DF;  // OBD-II functional request ID
    canMsg.can_dlc = 8;
    canMsg.data[0] = 0x02;   // 2 data bytes
    canMsg.data[1] = 0x01;   // Mode 01 (current data)
    canMsg.data[2] = pid;    // Requested PID
    canMsg.data[3] = 0x00;
    canMsg.data[4] = 0x00;
    canMsg.data[5] = 0x00;
    canMsg.data[6] = 0x00;
    canMsg.data[7] = 0x00;

    mcp2515.sendMessage(&canMsg);
    Serial.print("Requested PID: 0x");
    Serial.println(pid, HEX);
}

void can_loop() {
    if (mcp2515.readMessage(&canFrame) == MCP2515::ERROR_OK) {
        Serial.print(canFrame.can_id, HEX); // print ID
        Serial.print(" ");
        Serial.print(canFrame.can_dlc, HEX); // print DLC
        Serial.print(" ");

        for (int i = 0; i<canFrame.can_dlc; i++)  {  // print the data
            Serial.print(canFrame.data[i],HEX);
            Serial.print(" ");
        }

        Serial.println();

        if ((canFrame.can_id&0x7E8) == 0x7E8) {
            auto pid = canFrame.data[2];
            if (pid == 0x0C) {
                const auto rpm = ((unsigned int)canFrame.data[4])>>2 + ((unsigned int)canFrame.data[3])<<6;
                Serial.print("rpm is: ");
                Serial.println(rpm);
            }
            if (pid == 0x0D) {
                const auto kph = canFrame.data[3];
                Serial.print("KPH is: ");
                Serial.println(kph);
            }
        }
    }

    if ((millis() % 50) == 0) {
        requestOBD(0x0C);
        requestOBD(0x0D);
    }
}

