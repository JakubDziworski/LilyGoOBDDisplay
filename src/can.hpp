#include "mcp2515.h"
#include "Arduino.h"


MCP2515 mcp2515(SS);
can_frame canFrame;

#define RPM_PID 0x0C
#define KMH_PID 0x0D
#define STFT1_PID 0x06
#define LTFT1_PID 0x07
#define STFT2_PID 0x08
#define LTFT2_PID 0x09

void onKphUpdated(int kph);
void onStft1Updated(float trim);
void onStft2Updated(float trim);
void onLtft1Updated(float trim);
void onLtft2Updated(float trim);

void can_setup() {
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS);
    // mcp2515.setLoopbackMode();
    mcp2515.setNormalMode();
    mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7F8); // Mask for 0x7E8-0x7EF
    mcp2515.setFilter(MCP2515::RXF0, false, 0x7E8);
}

// https://www.rfwireless-world.com/Terminology/OBD2-Frame-format.html
void requestOBD(byte pid, byte mode = 0x01, byte length = 0x02) {
    can_frame canMsg = {};
    canMsg.can_id  = 0x7DF;  // OBD-II functional request ID
    canMsg.can_dlc = 8;
    canMsg.data[0] = length;  // 2 data bytes for 01 mode or 1 byte for mode 3 (DTCs)
    canMsg.data[1] = mode;   // Mode 01 (current data) or 03 (DTC)
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

void requestKmh() {
    requestOBD(KMH_PID);
}

void requestRPM() {
    requestOBD(RPM_PID);
}

void requestSTFT1() {
    requestOBD(STFT1_PID);
}

void requestLTFT1() {
    requestOBD(LTFT1_PID);
}

void requestSTFT2() {
    requestOBD(STFT2_PID);
}

void requestLTFT2() {
    requestOBD(LTFT2_PID);
}

void requestDTC() {
    requestOBD(0x03, 0x00, 0x01);
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

        if ((canFrame.can_id&0x7E8) == 0x7E8) {
            auto pid = canFrame.data[2];
            if (pid == RPM_PID) {
                const auto rpm = ((unsigned int)canFrame.data[4])>>2 + ((unsigned int)canFrame.data[3])<<6;
                Serial.print("rpm is: ");
                Serial.println(rpm);
            }
            if (pid == KMH_PID) {
                const auto kph = canFrame.data[3];
                Serial.print("KPH is: ");
                Serial.println(kph);
                onKphUpdated(kph);
            }
            if (pid >= STFT1_PID && pid <= LTFT2_PID) {
                auto trim = 1.28f*canFrame.data[3] - 100;
                Serial.print(pid);
                Serial.print(" fuel trim is: ");
                Serial.println(trim);
                if (pid == STFT1_PID) onStft1Updated(trim);
                if (pid == STFT2_PID) onStft1Updated(trim);
                if (pid == LTFT1_PID) onLtft1Updated(trim);
                if (pid == LTFT2_PID) onLtft2Updated(trim);
            }
        }

        Serial.println();
    }
}
//
// void parseDTCFrame(byte* data) {
//     data[]
//
//
//     for (int i = 0; i < codesFound; i++)
//             {
//                 memset(temp, 0, sizeof(temp));
//                 memset(codeNumber, 0, sizeof(codeNumber));
//
//                 codeType = *idx;            // Get first digit of second byte
//                 codeNumber[0] = *(idx + 1); // Get second digit of second byte
//                 codeNumber[1] = *(idx + 2); // Get first digit of third byte
//                 codeNumber[2] = *(idx + 3); // Get second digit of third byte
//
//                 switch (codeType) // Set the correct type prefix for the code
//                 {
//                 case '0':
//                     strcat(temp, "P0");
//                     break;
//
//                 case '1':
//                     strcat(temp, "P1");
//                     break;
//
//                 case '2':
//                     strcat(temp, "P2");
//                     break;
//                 case '3':
//                     strcat(temp, "P3");
//                     break;
//
//                 case '4':
//                     strcat(temp, "C0");
//                     break;
//
//                 case '5':
//                     strcat(temp, "C1");
//                     break;
//
//                 case '6':
//                     strcat(temp, "C2");
//                     break;
//
//                 case '7':
//                     strcat(temp, "C3");
//                     break;
//
//                 case '8':
//                     strcat(temp, "B0");
//                     break;
//
//                 case '9':
//                     strcat(temp, "B1");
//                     break;
//
//                 case 'A':
//                     strcat(temp, "B2");
//                     break;
//
//                 case 'B':
//                     strcat(temp, "B3");
//                     break;
//
//                 case 'C':
//                     strcat(temp, "U0");
//                     break;
//
//                 case 'D':
//                     strcat(temp, "U1");
//                     break;
//
//                 case 'E':
//                     strcat(temp, "U2");
//                     break;
//
//                 case 'F':
//                     strcat(temp, "U3");
//                     break;
//
//                 default:
//                     break;
//                 }
//
//                 strcat(temp, codeNumber);            // Append the code number to the prefix
//                 strcpy(DTC_Response.codes[i], temp); // Add the fully parsed code to the list (array)
//                 idx = idx + 8;                       // reset idx to start of next code
//
//                 if (debugMode)
//                 {
//                     Serial.print(F("ELMduino: Found code: "));
//                     Serial.println(temp);
//                 }
//             }
// }

