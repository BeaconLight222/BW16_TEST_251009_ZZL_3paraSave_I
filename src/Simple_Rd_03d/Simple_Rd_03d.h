#ifndef SIMPLE_RD_03D_H
#define SIMPLE_RD_03D_H
#include <Arduino.h>

#define UART_MUX_A_PIN PA27
#define UART_MUX_B_PIN PA30

typedef struct {
  int distance;
  int x;
  int y;
  int speed;
  int resolution;
} radarObject_t;

// A frame is 30 bytes
//  See Section 3.6 in https://aithinker-static.oss-cn-shenzhen.aliyuncs.com/docs/media/Radar/Guide/Rd-03D_V2quick_start_guide.pdf for the Serial Protocol specification 
#define RARDAR_RECEIVE_BUFFER_SIZE (30)

class Simple_Rd_03D {
public:
  Simple_Rd_03D();
  boolean begin(uint8_t _serial_mux = 0);

  uint8_t serial_mux;

  int checkRadarData(int timeout = 10);
  void activeSerialMux();
  void disableRadarStreaming();
  void enableRadarStreaming();
  void debugDumpBuffers();

  uint8_t radarRecvBuffer[RARDAR_RECEIVE_BUFFER_SIZE];
  int radarRecvBufferIndex;
  int radarDataLength;

  radarObject_t radarObject[3];
  int radarObjectCount;
};

#endif
