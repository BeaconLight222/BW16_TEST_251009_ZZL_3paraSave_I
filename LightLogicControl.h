#ifndef LIGHT_LOGIC_CONTROL_H
#define LIGHT_LOGIC_CONTROL_H

#include "src/Simple_misc_sensor/beacon_pcb.h"

#include "uv_lamp_parameter.h"
#include "src/Simple_MLX90640/Simple_MLX90640.h"
#include "src/Simple_Rd_03d/Simple_Rd_03d.h"
//#include "src/Simple_misc_sensor/Simple_misc_sensor.h"
#include <Arduino.h>

#include "ToolsArray.h"


#define SCHEDULE_BYTE                 42

//#define SCHEDULE_SELECT                 SCHEDULE_ALL_TURN_ON
//#define SCHEDULE_SELECT               SCHEDULE_ALL_TURN_OFF
#define SCHEDULE_SELECT               SCHEDULE_FROM_EEPROM     
#define SCHEDULE_ALL_TURN_OFF         0
#define SCHEDULE_ALL_TURN_ON          1
#define SCHEDULE_FROM_EEPROM          2

#define  LAMP_ON_RECORD_MANUAL        0
#define  LAMP_ON_RECORD_EMERGENCY     1
#define  LAMP_ON_RECORD_HIT8HOUR      2 

#define LAMP_ON_TIMESTAMPS_COUNT      5

#define SCHEDULE_SIZE  42
#define UV_LAMP_JULES_SCALE (100LLu)

#define EEPROM_SCHEDULE_START_ADDRESS             0x000           
#define EEPROM_SCHEDULE_SIZE                      (42)
#define EEPROM_SCHEDULE_END_ADDRESS               (EEPROM_SCHEDULE_START_ADDRESS + EEPROM_SCHEDULE_SIZE  )





#define EEPROM_LIGHT_LOG_DATA_SIZE 4
#define EEPROM_LIGHT_LOG_ENTRIES 4
#define EEPROM_LIGHT_LOG_START_ADDRESS 0x030


#define EEPROM_16LEVEL_LOG_START_ADDRESS          0x200
#define EEPROM_16LEVEL_LOG_ENTRIES                2
#define EEPROM_16LEVEL_LOG_ENTRY_SIZE             (4+240)
#define EEPROM_16LEVEL_LOG_END_ADDRESS (EEPROM_16LEVEL_LOG_START_ADDRESS + EEPROM_16LEVEL_LOG_ENTRIES * EEPROM_16LEVEL_LOG_ENTRY_SIZE)

#define EEPROM_UV_LAMP_MODE                       (EEPROM_TIME_ZONE_OFFSET_MINUTES -4) 
#define EEPROM_TIME_ZONE_OFFSET_MINUTES           ((EEPROM_LASTERROR_TIMESTAMP_4) -4)
#define EEPROM_LASTERROR_TIMESTAMP_4              0x3F7           
#define EEPROM_LASTERROR_TIMESTAMP_3              0x3F8
#define EEPROM_LASTERROR_TIMESTAMP_2              0x3F9
#define EEPROM_LASTERROR_TIMESTAMP_1              0x3FA
#define EEPROM_STATUS_ADDRESS                     0x3FB
#define EEPROM_LAMP_CURRENT_STATE_ADDRESS         0x3FC
#define EEPROM_LAMP_STARTUP_STATE_ADDRESS         0x3FD
#define EEPROM_DEFAULT_DETECTION_MODE_ADDRESS     0x3FE
#define EEPROM_MUTEALARMS_ADDRESS                 0x3FF   
#define EEPROM_END_ADDRESS                        0x3FF     //end of eeprom

#define EEPROM_STATUS_BIT_POSITION_WIFI           (0)
#define EEPROM_STATUS_BIT_POSITION_SCHEDULE_EN    (1)
#define EEPROM_STATUS_BIT_POSITION_DUMMY_2        (2)    
#define EEPROM_STATUS_BIT_POSITION_DUMMY_3        (3)        
#define EEPROM_STATUS_BIT_POSITION_DUMMY_4        (4)
#define EEPROM_STATUS_BIT_POSITION_DUMMY_5        (5)
#define EEPROM_STATUS_BIT_POSITION_DUMMY_6        (6)
#define EEPROM_STATUS_BIT_POSITION_DUMMY_7        (7)

#define LED_ON_LOG_INTERVAL (1000*60*15)


const uint8_t scheduleIndexShift[15]={
//0,  1,  2,  3,  4,  5,    6,    7,    8,    9,  10, 11, 12, 13, 14
  0,  2,  4,  6,  8, 10,    12,   14,   16,   18, 20, 22, 24, 26, 28
};

enum {
  UI_RED_LED_OFF = 0 << 4,
  UI_RED_LED_BLINK = 1 << 4,
  UI_RED_LED_ON = 2 << 4,
  UI_BLUE_LED_OFF = 0 << 2,
  UI_BLUE_LED_BLINK = 1 << 2,
  UI_BLUE_LED_ON = 2 << 2,
  UI_WHITE_LED_OFF = 0 << 0,
  UI_WHITE_LED_BLINK = 1 << 0,
  UI_WHITE_LED_ON = 2 << 0,
};

enum { UI_LED_WHITE = 0, UI_LED_BLUE = 1, UI_LED_RED = 2 };

enum { UI_LED_OFF = 0, UI_LED_BLINK = 1, UI_LED_ON = 2 };





enum {
  EXPOSURE_LEVEL_0 = 0,
  EXPOSURE_LEVEL_1 = 1,
  EXPOSURE_LEVEL_2 = 2,
  EXPOSURE_LEVEL_3 = 3
};


#define MINIMAL_DISTANCE_AVERAGE_COUNT (5)

struct lampOnTimeStamps {
  //uint8_t       index;
  uint32_t          lampOnTimeStamps_start;
  uint32_t          lampOnTimeStamps_end;
  float             length;
  String            reason;
  //int                length;
  //int32_t          length;
  //uint32_t          length;
  //enumReason_t  reason;
};
typedef   struct lampOnTimeStamps     lampOnTimeStamps_t;

/// Central controller for UV lamp behavior, sensor fusion, scheduling, and EEPROM logging.
/// Owns radar, thermal, temperature, RTC, and EEPROM drivers and is invoked from the main loop
/// to decide lamp state, UI LED patterns, and accumulated exposure tracking.
class LightLogicControl {
public:
  LightLogicControl();
  void begin();
  void startPeriodicLightProcess();
  void initRadar();
  void checkRadarData(bool printData = false);
  void initThermalSensor();
  void checkThermalSensorData(bool printData = false);
  void initTemperatureSensor();
  float getTemperatureData();
  void initEEPROM();
  void initRTC();
  DateTime getCurrentRtcTime();
  uint32_t get8hourSectionStartTime();
  int eepromInit_16LevelSection(uint32_t sectionStartTime);
  int eepromGetAccumulatedExposure_16Level(uint32_t sectionStartTime);
  int eepromWrite16LevelSection(uint32_t sectionStartTime, int julesLevel, int dataPointIndex);
  int eepromRead16LevelSection(uint32_t sectionStartTime);
  String eepromGetSectionData_16Level(uint32_t sectionStartTime);
  int eepromGetNewestEntryLightOnData();
  uint32_t eepromReadLightOnData();
  int eepromWriteLightOnData(uint32_t lightOnTimeForLoggingInIntervalUnit);
  int processLightControl(bool scheduleEnabled, uint8_t* scheduleData);
  String getSensorDataJson();
  void update();
  void setLightState(bool state);
  bool getLightState();
  void setUiLedState(int ledIndex, int state);
  int processSensorInfo(bool printData = false);
  int julesLevelForDistance(int distance);
  const char *getCompiledTimeStr();

  int       getpersonCount();
//  int getPersonCount();
  String    getDiagnosticsLevelJson_0();
  String    getDiagnosticsLevelJson_1();
  String    getDiagnosticsLevelJson_2();
  String    getDiagnosticsLevelJson_255();


  int       eepromWriteSchedule();
  String    getScheduleStr();
  float     getlength(uint32_t startTime, uint32_t endTime);
  uint8_t   getmotionEvents();
  bool      getsensorHealth();
  int       setBitEeprom(int memAddress, uint8_t bit_position);
  int       clrBitEeprom(int memAddress, uint8_t bit_position);
  bool      getBitEeprom(int memAddress, uint8_t bit_position);
  int       setLongWordEeprom(uint16_t memAddress, uint32_t longData);
  uint32_t  getLongWordEeprom(uint16_t memAddress);
  String    getScheduleStrTad();
  bool      getsensorHealth_print(bool fprint);
  String    getScheduleStrTad_print(bool fprint);
  String    getScheduleStrAll_print(bool fprint, bool fSchedule);
  bool      getscheduleInEffect(uint8_t* scheduleData,bool fprint);
  bool      processSmartMode(bool scheduleEnabled, uint8_t* scheduleData);
  int       getUvLampModeEeprom();  
  bool      getScheduleEnableEeprom(); 
  void      setDetectionModeEeprom();
  uint8_t   findWhichLevelSection(uint32_t sectionStartTime);
  int       eepromClear_16LevelSection(uint8_t index, uint32_t sectionStartTime);
  void      turnOffandRecord();
  String    getDiagnosticsLevelJson_254();


  uint8_t             errorStatus;
  bool                sensorHealth;
  uint8_t             motionEvents;
  bool                fSetStartTime;
  bool                fEmergency;
  uint8_t             lampOnRecord_index;
  lampOnTimeStamps_t  lampOnRecordCache;
  lampOnTimeStamps_t  lampOnRecord[LAMP_ON_TIMESTAMPS_COUNT];
  String          ScheduleStr;
  int             personCount;
  bool            lightState;
  Simple_Rd_03D   radar1, radar2;
  bool            radar1Valid, radar2Valid;
  Simple_MLX90640 thermalSensor;
  bool            thermalSensorValid;
  struct Detection *thermalDetections;
  int             thermalDetectionCount;
  Simple_TMP102   temperatureSensor;
  bool            temperatureSensorValid;
  float           temperatureData;
  PCF85363A       rtc;
  //Simple_DS3231 rtc;
  bool            rtcValid;
  DateTime        currentRtcTime;
  Simple_EEPROM_AT24C08 eeprom;
  bool            eepromValid;
  int             calculatedMinimalDistance;
  int             averagedCalculatedMinimalDistance;
  int             minimalDistanceHistory[MINIMAL_DISTANCE_AVERAGE_COUNT];
  int             historyIndex;
  int             minimalDistanceJulesLevel;
  uint32_t        inProgress8hourSectionStartTime;
  int             processedMinutesCount;
  int             accumulatedExposure;
  int             accumulatedExposureThreshold;
  int             uvLampMode;
  uint32_t        lightOnTimeForLoggingInIntervalUnit;
  uint32_t        lightOnTimeLastCheckMilliseconds;
  uint32_t        lightOnTimeNotLoggedYetMilliseconds;
  bool            lastScheduleInEffect;

};

#endif