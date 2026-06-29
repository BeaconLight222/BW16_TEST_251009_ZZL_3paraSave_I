#ifndef AWS_MQTT_H
#define AWS_MQTT_H

//#include "src/ArduinoJson-7.4.1/ArduinoJson-v7.4.1.h"

#include <Arduino.h>

#undef max
#undef min
#include "OtaProcessor.h"
#include <vector>

/*
#define SCHEDULE_ALL_TURN_OFF 0
#define SCHEDULE_ALL_TURN_ON  1
#define SCHEDULE_FROM_EEPROM  2
#define SCHEDULE_UNDEFINED    255
#define SCHEDULE_SELECTION    SCHEDULE_FROM_EEPROM  
*/

struct MQTTMessageData {
  String topic;
  String payload;
};

/// Manages AWS IoT MQTT connectivity, job processing, OTA coordination, and schedule state.
/// Receives cloud commands and forwards mode changes to LightLogicControl via payload_manager.
class AwsMqtt {
public:
  // AwsMqtt(const char* mqttServer, int mqttPort, const char* clientId, const
  // char* rootCA, const char* privateKey, const char* certificate);
  void begin();
  void loop();
  void reconnect();
  void serverCallback(char *topic, char *payload, unsigned int length);
  void jobsCallback(char *topic, char *payload, unsigned int length);
  void logSensorToServer(char *payload);
  void sendTelemetryToServer(char *payload);
  // bool publish(const char* topic, const char* payload);
  // bool subscribe(const char* topic, std::function<void(char*, uint8_t*,
  // unsigned int)> callback);

  bool        fConnectingMqtt;

  int         thresAccumulatedExposure;
  bool        wifiReset;
  bool        factoryReset;
  uint8_t     diagnosticsLevel;
  bool        firmwareAutoUpdate;           
  //uint32_t    telemetryIntervalSeconds;
  String      lampStartupState;
  //unsigned char   defaultDetectionMode;
  uint8_t           defaultDetectionMode;
  uint8_t     scheduleVersion;
  //char*     configVersion;
  String    configVersion;
  int       scheduleTimezoneOffsetMinutes;
  uint8_t   scheduleData[42]; //42*8bits = 336bits, enough for 7 days schedule with 48 half-hour slots per day
  //JsonArray scheduleArr;
  

  int       rtcUnixTime;
  bool      proxyEnabled;
  String    setProxyIp;
  int       telemetryIntervalSeconds;
  bool      rebootDevice;
  bool      muteAlarms;
  bool      enableDiagnosticsMode;
  int       setPersonCountThreshold;
  bool      resetAccumulatedExposureLimit;

  // std::vector<String> jobsToDo;
  String processingJobName;
  String processingJobContent;
  String processingOTAStreamName;
  int processingOTAFileSize;
  String processingOTASignature;
  int processingOTAReadedBytes;
  int processingOTARequestedBytes;
  int jobState;
  int prevJobState;
  uint32_t jobStateChangeTimeMillis;
  std::vector<MQTTMessageData> jobsMessages;
  std::vector<String> jobsList;

  OtaProcessor otaProcessor;

  uint32_t initOtaJobTimeMessageMillis;
  uint32_t initOtaJobTimeMessageUnixTime;

  /// @brief newModeCommand pass the mode command from the server to the light control logic in the main loop
  String newModeCommand;

 
  bool scheduleEnabled;
  


private:
  // const char* mqttServer;
  // int mqttPort;
  // const char* clientId;
  // const char* rootCA;
  // const char* privateKey;
  // const char* certificate;

  // WiFiClientSecure wifiClient;
  // PubSubClient mqttClient;
};

#endif // AWS_MQTT_H