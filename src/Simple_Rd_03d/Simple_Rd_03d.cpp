#include "Simple_Rd_03d.h"

Simple_Rd_03D::Simple_Rd_03D() {
  serial_mux = 0;
  radarRecvBufferIndex = 0;
  radarDataLength = 0;
  radarObjectCount = 0;
}

boolean Simple_Rd_03D::begin(uint8_t _serial_mux) {
  serial_mux = _serial_mux;

  Serial1.flush();
  delay(1);

  pinMode(UART_MUX_A_PIN, OUTPUT);
  pinMode(UART_MUX_B_PIN, OUTPUT);
  digitalWrite(UART_MUX_A_PIN, (serial_mux & 0x01) ? HIGH : LOW);
  digitalWrite(UART_MUX_B_PIN, (serial_mux & 0x02) ? HIGH : LOW);

  Serial1.begin(256000);
  delay(1);
  while (millis() < 700) {
    // wait for radar port to be ready
  }

  int initAttempts = 0;

  while ((initAttempts++) < 3) {
    while (Serial1.available()) {
      Serial1.read();
    }
    // unsigned char dataMulti[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x90,
    // 0x00, 0x04, 0x03, 0x02, 0x01}; Serial1.write(dataMulti,
    // sizeof(dataMulti));
    unsigned char dataSingle[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00,
                                  0x80, 0x00, 0x04, 0x03, 0x02, 0x01};
    Serial1.write(dataSingle, sizeof(dataSingle));
    Serial1.flush();
    unsigned long startTime = millis();
    // expecting response: FD FC FB FA 04 00 90 01 01 00 04 03 02 01
    // uint8_t dataResponseExpected[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04,
    // 0x00, 0x90, 0x01, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
    uint8_t dataResponseExpected[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04,
                                            0x00, 0x80, 0x01, 0x01, 0x00,
                                            0x04, 0x03, 0x02, 0x01};
    int bytesExpected = 14;
    uint8_t recvBuffer[14];
    int recvBufferIndex = 0;
    while (millis() - startTime < 100) {
      if (Serial1.available()) {
        char c = Serial1.read();
        if (recvBufferIndex < (int)(sizeof(recvBuffer))) {
          recvBuffer[recvBufferIndex++] = c;
          if (recvBufferIndex >= bytesExpected) {
            recvBufferIndex = 0;
          }
        }
        // Serial.print(c, HEX);
        // Serial.print(" ");
        // check if we have received the expected response
        uint8_t dataReceivedAsExpected = 1;
        int checkPtr = recvBufferIndex;
        for (int i = 0; i < (int)(sizeof(dataResponseExpected)); i++) {
          if (i != 8) {
            // byte 8 can be 0x00 or 0x01, so we skip the check
            if (recvBuffer[checkPtr] != dataResponseExpected[i]) {
              dataReceivedAsExpected = 0;
              break;
            }
          }
          checkPtr++;
          if (checkPtr >= bytesExpected) {
            checkPtr = 0;
          }
        }
        if (dataReceivedAsExpected) {
          // Serial.println("Received expected response");
          return true;
        }
      }
    }
  }

  return false;
}

void Simple_Rd_03D::activeSerialMux() {
  bool muxA = (serial_mux & 0x01) ? true : false;
  bool muxB = (serial_mux & 0x02) ? true : false;
  bool readedMuxA = digitalRead(UART_MUX_A_PIN);
  bool readedMuxB = digitalRead(UART_MUX_B_PIN);
  if (readedMuxA != muxA || readedMuxB != muxB) {
    Serial1.flush();
    delay(1);
    digitalWrite(UART_MUX_A_PIN, (serial_mux & 0x01) ? HIGH : LOW);
    digitalWrite(UART_MUX_B_PIN, (serial_mux & 0x02) ? HIGH : LOW);
    // clear input buffer
    while (Serial1.available()) {
      Serial1.read();
    }
  } else {
    return; // no need to change the mux
  }
}

void Simple_Rd_03D::disableRadarStreaming() {
  // drop the current consumption to 125mA
  // send command to disable radar streaming
  // try FD FC FB FA 04 00 FF 00 01 00 04 03 02 01
  unsigned char dataMulti[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF,
                               0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
  Serial1.write(dataMulti, sizeof(dataMulti));
  Serial1.flush();
}

void Simple_Rd_03D::enableRadarStreaming() {
  // increase the current consumption to 127mA
  // send command to enable radar streaming
  // try FD FC FB FA 02 00 FE 00 04 03 02 01
  unsigned char dataMulti[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00,
                               0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};
  Serial1.write(dataMulti, sizeof(dataMulti));
  Serial1.flush();
}

// This is the best guide I can find for the Serial Protocol specification of the Rd-03D radar sensor:
// https://aithinker-static.oss-cn-shenzhen.aliyuncs.com/docs/media/Radar/Guide/Rd-03D_V2quick_start_guide.pdf
int Simple_Rd_03D::checkRadarData(int timeout) {
  // check if the mux changed
  // do digitalRead, then clear the buffer, etc, etc, TODO
  // A frame has the following format
  // FRAME HEADER: 0xAA 0xFF 0x03 0x00
  // In-frame Data (24 bytes): 3 objects, each object has 8 bytes
  //   Target X Coordinant: Signed int16, highest bit 1 corresponds to positive coordinate, 0 corresponds to negative coordinate, remaning 15 bits represent the absolute value of the x coordinate in millimeters
  //   Target Y Coordinant: Signed int16, highest bit 1 corresponds to positive coordinate, 0 corresponds to negative coordinate, remaning 15 bits represent the absolute value of the y coordinate in millimeters
  //          Target Speed: Signed int16, highest bit 1 corresponds to positive speed, 0 corresponds to negative speed, remaning 15 bits represent the absolute value of the speed in cm/s
  //     Target Resolution: Unsigned int16, single pixel distance value, in mm
  // FRAME TAIL: 0x55 0xCC

  radarObjectCount = 0; // reset the object count before checking for new data

  unsigned long startTime = millis();
  while ((int)(millis() - startTime) < timeout) {
    if (Serial1.available()) {
      char c = Serial1.read();
      radarRecvBuffer[radarRecvBufferIndex++] = c;
      radarDataLength++;
      if (radarRecvBufferIndex >= (int)(sizeof(radarRecvBuffer))) {
        radarRecvBufferIndex = 0;
      }
      if (radarDataLength >= 30) {
        if (radarRecvBuffer[(radarRecvBufferIndex + 0) % 30] == 0xAA &&
            radarRecvBuffer[(radarRecvBufferIndex + 1) % 30] == 0xFF &&
            radarRecvBuffer[(radarRecvBufferIndex + 2) % 30] == 0x03 &&
            radarRecvBuffer[(radarRecvBufferIndex + 3) % 30] == 0x00) {
          // check if the last two bytes are 0x55 and 0xCC
          if (radarRecvBuffer[(radarRecvBufferIndex + 28) % 30] == 0x55 &&
              radarRecvBuffer[(radarRecvBufferIndex + 29) % 30] == 0xCC) {
            int detectedObjectCount = 0;
            for (int i = 0; i < 3; i++) {
              int offset = (radarRecvBufferIndex + 4 + i * 8) % 30;
              // we are going to check 4 of the 8 bytes, each 2 bytes is little
              // endian int16 type x is mm, y is mm, speed is cm/s, resolution
              // is mm need more optimization

              // 
              uint16_t raw_x = (radarRecvBuffer[(offset + 1) % 30] << 8) |
                                radarRecvBuffer[(offset + 0) % 30];
              int16_t x = ((raw_x & 0x8000) ? 1 : -1) * (raw_x & 0x7FFF);

              uint16_t raw_y = (radarRecvBuffer[(offset + 3) % 30] << 8) |
                                radarRecvBuffer[(offset + 2) % 30];
              int16_t y = ((raw_y & 0x8000) ? 1 : -1) * (raw_y & 0x7FFF);

              uint16_t raw_speed = (radarRecvBuffer[(offset + 5) % 30] << 8) |
                              radarRecvBuffer[(offset + 4) % 30];
              int16_t speed = ((raw_speed & 0x8000) ? 1 : -1) * (raw_speed & 0x7FFF);

              uint16_t resolution = (radarRecvBuffer[(offset + 7) % 30] << 8) |
                                     radarRecvBuffer[(offset + 6) % 30];
              // check if the x, y, speed are all 0
              if (x == 0 && y == 0 && speed == 0) {
                // no target
                // Serial.println("No target");
              } else {
                radarObject_t *radarObjectPtr =
                    &radarObject[detectedObjectCount];
                radarObjectPtr->x = x;
                radarObjectPtr->y = y;
                radarObjectPtr->speed = speed;
                radarObjectPtr->resolution = resolution;
                detectedObjectCount++;
                if (detectedObjectCount >= 3) {
                  break;
                }
              }
            }

            radarObjectCount = detectedObjectCount;
            return 1;
          }
        }
      }
    }
  }
  radarObjectCount = -1; // no data received, sensor might be error
  return 0;
}

void Simple_Rd_03D::debugDumpBuffers() {
  Serial.print("Radar Dump: ");
  Serial.print(serial_mux);
  Serial.print(", radarObjectCount: ");
  Serial.print(radarObjectCount);
  Serial.print(", Index: ");
  Serial.print(radarRecvBufferIndex);
  Serial.print(", Data Length: ");
  Serial.println(radarDataLength);
  for (int i = 0; i < 30; i++) {
    Serial.print(radarRecvBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
} 