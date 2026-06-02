#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <vector>
#include <Arduino.h>
#include "uv_lamp_parameter.h"

#define JOB_LAMP_STARTUP_OFF          0
#define JOB_LAMP_STARTUP_ON           1
#define JOB_LAMP_STARTUP_LASTKNOWN    2 

enum LAST_COMMAND_STATUS  {
  LAST_COMMAND_SUCCESS   = 0,
  LAST_COMMAND_FAILURE   = 1
};
typedef enum LAST_COMMAND_STATUS    LAST_COMMAND_STATUS_t; 

enum REASON{
  REASON_HIT_8HR_LIMIT        =0,
  REASON_MANUAL_MODE_SHUTOFF  =1,
  REASON_EMERGENCY_SHUTOFF    =2,
}; 
typedef enum REASON                 reason_t;



struct LIFE_OF_LIGHT{
uint32_t              startTimestamp;
uint32_t              endTimestamp;
float                 length;

reason_t                reason;
//String                reason;

};
typedef   struct LIFE_OF_LIGHT      LIFE_OF_LIGHT_t; 

struct SETTING{
  bool                lampEnable;
  int                 detectionMode;
  bool                scheduleEnable;
  bool                factoryReset;
  bool                wifiReset;
  int                 telemetryIntervalSeconds;
  uint32_t            rtcUnixTime;
  bool                schedule[336];
  int                 timeZoneOffsetMinutes;
  LIFE_OF_LIGHT_t     lampOnTimestamps[3];
};
typedef struct SETTING    SETTING_t;


struct PAYLOAD_TELEMETRY_PUB{
  bool                occupied;
  bool                lampStatus;
  SETTING_t           setting;
  String              firmware;
  uint32_t            rtcUnixTime;
  int                 accumulatedExposure;
  int                 personCount;
  float               temperatureData;

  LAST_COMMAND_STATUS_t lastCommandStatus;
  //String              lastCommandStatus; 
  
  uint8_t             motionEvents;
  int                 uptimeSeconds;
  uint32_t            lastErrorTimestamp;
  bool                sensorHealth;
};
typedef   struct PAYLOAD_TELEMETRY_PUB      PAYLOAD_TELEMETRY_PUB_t;



struct PAYLOAD_DEVICECONFIG_SUB{
  int                 rtcUnixTime;

  uint8_t             scheduleData[42];    //42*8 = 336bit,  7*48=336, total 7 day with 1/2hour each record
  //int                 schedule[336];    // eg 0,0,0,1 , 0 represent by byte
  
  int                 timeZoneOffsetMinutes;
  String              configVersion;
  uint8_t             scheduleVersion;

  //unsigned char       defaultDetectionMode;
  uint8_t             defaultDetectionMode;
  
  String              lampStartupState;

  int                   telemetryIntervalSeconds;
  //unsigned int        telemetryIntervalSeconds;

  bool                firmwareAutoUpdate; 
  uint8_t             diagnosticsLevel;
};//end     struct PAYLOAD_DEVICECONFIG_SUB
typedef   struct PAYLOAD_DEVICECONFIG_SUB       PAYLOAD_DEVICECONFIG_SUB_t;

struct PAYLOAD_COMMAND_SUB {
  bool                lampEnable;
  uv_lamp_mode_enum   detectionMode;
  //int               detectionMode;
  bool                scheduleEnable;
  bool                factoryReset;
  bool                wifiReset;
  bool                resetAccumulatedExposureLimit;

  int                 setPersonCountThreshold;
  //unsigned char       setPersonCountThreshold;
  
  bool                enableDiagnosticsMode;
  bool                muteAlarms=false;
  bool                rebootDevice;

  int                   telemetryIntervalSeconds;
  //unsigned int        telemetryIntervalSeconds;
  
  String              setProxyIp;
  bool                proxyEnable;     
  int                 thresAccumulatedExposure; 
};
typedef   struct PAYLOAD_COMMAND_SUB            PAYLOAD_COMMAND_SUB_t;     

class PAYLOAD_MANAGER {
public:
    PAYLOAD_TELEMETRY_PUB_t                     payload_telemetry_pub; 
    PAYLOAD_DEVICECONFIG_SUB_t                  payload_deviceConfig_sub;
    PAYLOAD_COMMAND_SUB_t                       payload_command_sub;
    std::vector<String>                         list_newModeCommand;

    void begin();

};



#endif