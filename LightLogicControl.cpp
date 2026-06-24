#include "WString.h"
#include <stdint.h>
#include <stdlib.h>
#include <cstdio>
/// @file LightLogicControl.cpp
/// @brief Light logic control for the BW16 project, also manages radar, thermal sensor, temperature sensor, RTC, and EEPROM interactions.

#include "EnergyEngineer.h"
#include "payload.h"
//#include "ioExpander.h"
#include "LightLogicControl.h"
#include <GTimer.h>
#include "src/mlx90640YoloProcessor/mlx90640YoloProcessor.h"
#include "NonvolatileDataManager.h"

extern EnergyEngineer beaconEnergyEngineer;

extern uint32_t program_StartUpTime;
extern BEACON_PCB beacon_pcb;
extern PAYLOAD_MANAGER payload_manager;

const char compiledTimeStr[] = __DATE__ " " __TIME__;

/// @brief Exposure levels in micro Joules per minute, divided into 4 intensity levels.
const int exposureLevelToJules[4] = {
  0, 20, 90, 360
};  // Jules for exposure levels 0, 1, 2, 3
// level3 < 1M
// level2 < 2.5M
// level1 further

// exposure from Andrea slack message
// 0.0127 meters/ 0.042 Feet/ .5 inches: 2.4mW/sec
// .25m / 0.8ft: .023
// .50m / 1.7ft: .006
// 1 meter/3.3 Feet: 0.0035
// 1.5 Meters/4.9 Ft:	0.0015
// 2 Meters/6.6 Feet: 0.001

// distance is ranged in <1m, <2.5m
// for <1m, lets use 0.006mW/sec, for a minute it is 0.36mJ, which is 360uJ
// for <2.5m, lets use 0.0015mW/sec, for a minute it is 0.09mJ, which is 90uJ

// We are using the lower threshold eye TLV of 161 mJ/cm2., that would be 161000
// uJ/cm2 write it to accumulatedExposureThreshold

#define GTIMER0LEDSTATE_RED_LED_LOCATION 4
#define GTIMER0LEDSTATE_BLUE_LED_LOCATION 2
#define GTIMER0LEDSTATE_WHITE_LED_LOCATION 0

#include "AwsMqtt.h"
extern AwsMqtt awsMqtt;

volatile int gTimer0Counter = 0;
volatile int gTimer0LedState = 0;

void LightLogicControl::turnOffandRecord() {
  setLightState(false);  // Turn off the light
  lampOnRecord[LAMP_ON_RECORD_EMERGENCY].lampOnTimeStamps_start = lampOnRecordCache.lampOnTimeStamps_start;
  lampOnRecord[LAMP_ON_RECORD_EMERGENCY].lampOnTimeStamps_end = lampOnRecordCache.lampOnTimeStamps_end;
  lampOnRecord[LAMP_ON_RECORD_EMERGENCY].length = lampOnRecordCache.length;
  lampOnRecord[LAMP_ON_RECORD_EMERGENCY].reason = "Emergency shut off";

}  //end    void    LightLogicControl::turnOffandRecord()

void LightLogicControl::setDetectionModeEeprom() {
  int _uvLampMode = uvLampMode;
  uint8_t byteData[4] = { 0 };
  memcpy(byteData, &_uvLampMode, sizeof(uvLampMode));
  Serial.print("Before write uvLampMode:");
  Serial.println(uvLampMode);
  //Serial.print("Before write uvLampMode:");Serial.println(byteData);
  eeprom.write(EEPROM_UV_LAMP_MODE, byteData, sizeof(byteData));
  getUvLampModeEeprom();
}  //end  void    LightLogicControl::setDetectionModeEeprom

/*
* Function :
* 1) get    scheduleEnabled bit from eeprom
* 2) update awsMqtt.scheduleEnabled from the eeprom 
*
*/
bool LightLogicControl::getScheduleEnableEeprom() {
  bool fResult = getBitEeprom(EEPROM_STATUS_ADDRESS, EEPROM_STATUS_BIT_POSITION_SCHEDULE_EN);

  if (true) {
    //if(true){
    Serial.print("EEPROM_STATUS_BIT_POSITION_SCHEDULE_EN_________________________________:");
    Serial.println(fResult);
  }
  awsMqtt.scheduleEnabled = fResult;
  return awsMqtt.scheduleEnabled;
}  //end    bool      LightLogicControl::getScheduleEnableEeprom()


/*
* Function : 
* 1) read uvLampMode from eeprom into   lightControl.uvLampMode 
* 
*/
int LightLogicControl::getUvLampModeEeprom()
//void  LightLogicControl::getUvLampModeEeprom()
{

  uint8_t checkByte[4] = { 0 };
  int checking_uvLampMode = 0;

  if (true) {
    Serial.print("Before checking_uvLampMode_______________________________:");
    Serial.println(checking_uvLampMode);
  }  //end

  eeprom.read(EEPROM_UV_LAMP_MODE, checkByte, sizeof(checkByte));
  memcpy(&checking_uvLampMode, checkByte, sizeof(checkByte));

  if (true) {
    Serial.print("After checking_uvLampMode:");
    Serial.println(checking_uvLampMode);
  }
  uvLampMode = checking_uvLampMode;
  return uvLampMode;
}  //end

uint32_t LightLogicControl::getLongWordEeprom(uint16_t memAddress) {
  uint8_t byteData[4] = { 0 };

  Serial.print("Before LightLogicControl::getLongWordEeprom:");
  Serial.println(memAddress, HEX);
  int ret = eeprom.read(memAddress, byteData, sizeof(byteData));
  Serial.print("After LightLogicControl::getLongWordEeprom:");
  Serial.println(memAddress, HEX);

  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address:");
    Serial.println(memAddress);
    return -1;
    //return ""; // Return empty string on read failure
  }

  uint32_t longWordResult;
  memcpy(&longWordResult, byteData, sizeof(longWordResult));

  return longWordResult;
}  //end      uint32_t    getLongWordEeprom

int LightLogicControl::setLongWordEeprom(uint16_t memAddress, uint32_t longData) {
  uint8_t byteData[4];

  memcpy(byteData, &longData, sizeof(longData));

  Serial.print("byteData[0]:");
  Serial.println(byteData[0], HEX);  //LSB
  Serial.print("byteData[1]:");
  Serial.println(byteData[1], HEX);
  Serial.print("byteData[2]:");
  Serial.println(byteData[2], HEX);
  Serial.print("byteData[3]:");
  Serial.println(byteData[3], HEX);  //MSB

  int ret = eeprom.write(memAddress, byteData, sizeof(byteData));
  return ret;
}  //end    void    LightLogicControl::eepromWriteLong

bool LightLogicControl::getBitEeprom(int memAddress, uint8_t bit_position) {
  bool fResult;
  uint8_t status = 0x00;

  eeprom.read(memAddress, &status, sizeof(status));
  Serial.print("memAddress, &status:");
  Serial.println(status);

  status &= (1 << bit_position);

  if (status == 0) {
    fResult = false;
  } else {
    fResult = true;
  }
  Serial.print("fResult:");
  Serial.println(fResult);

  return fResult;

}  //end    bool     LightLogicControl::getBitEeprom

int LightLogicControl::clrBitEeprom(int memAddress, uint8_t bit_position) {
  uint8_t status = 0x00;

  int ret = eeprom.read(memAddress, &status, sizeof(status));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(memAddress);
    return -1;  // Return -1 on read failure
  }


  status &= ~(1 << bit_position);
  //  status |= (uint8_t)(1<<bit_position);

  ret = eeprom.write(memAddress, &status, sizeof(status));
  if (ret < 0) {
    Serial.print("Failed to write EEPROM at address ");
    Serial.println(memAddress);
    return -1;  // Return -1 on write failure
  }
  return ret;


}  //end      void      LightLogicControl::setEeprom


int LightLogicControl::setBitEeprom(int memAddress, uint8_t bit_position) {
  uint8_t status = 0x00;

  int ret = eeprom.read(memAddress, &status, sizeof(status));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(memAddress);
    return -1;  // Return -1 on read failure
  }


  status &= ~(1 << bit_position);
  status |= (uint8_t)(1 << bit_position);

  ret = eeprom.write(memAddress, &status, sizeof(status));
  if (ret < 0) {
    Serial.print("Failed to write EEPROM at address ");
    Serial.println(memAddress);
    return -1;  // Return -1 on write failure
  }
  return ret;
}  //end      void      LightLogicControl::setEeprom

bool LightLogicControl::getsensorHealth_print(bool fprint) {
  bool _sensorHealth = false;
  if (radar1Valid && radar2Valid && thermalSensorValid && temperatureSensorValid && rtcValid && eepromValid) {
    _sensorHealth = true;
    if (fprint) {
      Serial.println("ALL sensor is healthy");
    }
  } else {
    if (fprint) {
      Serial.print("radar1Valid:");
      Serial.println(radar1Valid);
      Serial.print("radar2Valid:");
      Serial.println(radar2Valid);
      Serial.print("thermalSensorValid:");
      Serial.println(thermalSensorValid);
      Serial.print("temperatureSensorValid:");
      Serial.println(temperatureSensorValid);
      Serial.print("rtcValid:");
      Serial.println(rtcValid);
      Serial.print("eepromValid:");
      Serial.println(eepromValid);
    }  //end if(fprint)
  }    //end else

  sensorHealth = _sensorHealth;
  return sensorHealth;

}  //end  bool      LightLogicControl::getsensorHealth_print

bool LightLogicControl::getsensorHealth() {
  bool _sensorHealth = false;
  if (radar1Valid && radar2Valid && thermalSensorValid && temperatureSensorValid && rtcValid && eepromValid) {
    _sensorHealth = true;
    Serial.println("ALL sensor is healthy");
  } else {
    Serial.print("radar1Valid:");
    Serial.println(radar1Valid);
    Serial.print("radar2Valid:");
    Serial.println(radar2Valid);
    Serial.print("thermalSensorValid:");
    Serial.println(thermalSensorValid);
    Serial.print("temperatureSensorValid:");
    Serial.println(temperatureSensorValid);
    Serial.print("rtcValid:");
    Serial.println(rtcValid);
    Serial.print("eepromValid:");
    Serial.println(eepromValid);
  }
  sensorHealth = _sensorHealth;
  return sensorHealth;

}  //end    bool      LightLogicControl::getsensorHealth

uint8_t LightLogicControl::getmotionEvents() {
  uint8_t _motionEvents = 0;
  int radar1Count = radar1.radarObjectCount;
  int radar2Count = radar2.radarObjectCount;

  for (int i = 0; i < radar1Count; i++) {
    if (radar1.radarObject[i].speed > 0) {
      _motionEvents = _motionEvents + 1;
    }
  }  //end      for (int i = 0; i < radar1Count; i++)

  for (int i = 0; i < radar2Count; i++) {
    if (radar2.radarObject[i].speed > 0) {
      _motionEvents = _motionEvents + 1;
    }
  }  //end      for (int i = 0; i < radar1Count; i++)

  motionEvents = _motionEvents;

  return motionEvents;
}  //end      uint8_t   LightLogicControl::getmotionEvents


int LightLogicControl::getpersonCount() {
  int noOfPerson = 0;

  if (radar1Valid) { noOfPerson = radar1.radarObjectCount; }
  if ((radar2.radarObjectCount > noOfPerson) && radar2Valid) { noOfPerson = radar2.radarObjectCount; }
  if (thermalDetectionCount > noOfPerson && thermalSensorValid) { noOfPerson = thermalDetectionCount; }
  personCount = noOfPerson;

  return personCount;
}  //end    int   LightLogicControl::getpersonCount()

/// @brief Timer interrupt handler for LED blinking
void gTimer0handler(uint32_t data) {
  (void)data;

  gTimer0Counter++;
  int toggleCounter = ~(gTimer0Counter);

  int redLedState = gTimer0LedState & (3 << GTIMER0LEDSTATE_RED_LED_LOCATION);
  //  int redLedState = gTimer0LedState & (3 << 4);
  if (redLedState == UI_RED_LED_BLINK) {
    GPIO_WriteBit(_PB_3, toggleCounter & 1);  //prevent toggle with blue-led at same time
    //  GPIO_WriteBit(_PB_3, gTimer0Counter & 1);
  } else if (redLedState == UI_RED_LED_ON) {
    GPIO_WriteBit(_PB_3, 1);
  } else {
    GPIO_WriteBit(_PB_3, 0);
  }

  int blueLedState = gTimer0LedState & (3 << GTIMER0LEDSTATE_BLUE_LED_LOCATION);
  //  int blueLedState = gTimer0LedState & (3 << 2);
  if (blueLedState == UI_BLUE_LED_BLINK) {
    GPIO_WriteBit(_PA_14, gTimer0Counter & 1);
  } else if (blueLedState == UI_BLUE_LED_ON) {
    GPIO_WriteBit(_PA_14, 1);
  } else {
    GPIO_WriteBit(_PA_14, 0);
  }

  int whiteLedState = gTimer0LedState & (3 << GTIMER0LEDSTATE_WHITE_LED_LOCATION);
  //  int whiteLedState = gTimer0LedState & (3 << 0);
  if (whiteLedState == UI_WHITE_LED_BLINK) {
    GPIO_WriteBit(_PA_13, gTimer0Counter & 1);
  } else if (whiteLedState == UI_WHITE_LED_ON) {
    GPIO_WriteBit(_PA_13, 1);
  } else {
    GPIO_WriteBit(_PA_13, 0);
  }
}  //end void gTimer0handler

LightLogicControl::LightLogicControl() {
  lightState = false;
  radar1Valid = false;
  radar2Valid = false;
  thermalSensorValid = false;
  eepromValid = false;
  thermalDetections = NULL;
  thermalDetectionCount = 0;
  temperatureSensorValid = false;
  temperatureData = 0.0f;
  calculatedMinimalDistance = INT_MAX;
  averagedCalculatedMinimalDistance = 0;
  historyIndex = 0;
  for (int i = 0; i < 5; i++) {
    // Initialize to 0 distance so that a new detections must bring the average up in value
    //  This should ensure the lamp stays off (due to the proximity threshold) until enough
    //  detections have been made to ensure the true average is above the threhsold 
    minimalDistanceHistory[i] = 0;
  }
  minimalDistanceJulesLevel = EXPOSURE_LEVEL_0;
  inProgress8hourSectionStartTime = 0;
  processedMinutesCount = -1;
  accumulatedExposure = 0;
  uvLampMode = UV_MODE_SMART;
  accumulatedExposureThreshold = 161000;

  lightOnTimeForLoggingInIntervalUnit = 0;
  lightOnTimeLastCheckMilliseconds = 0;
  lightOnTimeNotLoggedYetMilliseconds = 0;
  lastScheduleInEffect = false;
}


/*
* Function:
*  select       #define SCHEDULE_SELECT               SCHEDULE_FROM_EEPROM
* 1) read schedule from eeprom into the local-buffer scheduleData[42]
* 2) output:     payload_manager.payload_deviceConfig_sub.scheduleData[i]  = scheduleData[i];
* 3) output:     awsMqtt.scheduleData[i]                                   = scheduleData[i];
* 4) output:     lightControl.ScheduleStr {1,0,0....} and  return this string
*/

void LightLogicControl::begin() {
  pinMode(PA12, OUTPUT);
  update();

  errorStatus = ERROR_NO;
  fSetStartTime = false;

  //fEmergency = false;
  lampOnRecord_index = 0;


  int schedule_select = SCHEDULE_SELECT;
  switch (schedule_select) {
    case SCHEDULE_ALL_TURN_OFF:
      //getScheduleStrAll_print(false, false);
      getScheduleStrAll_print(true, false);
      break;

    case SCHEDULE_ALL_TURN_ON:
      //getScheduleStrAll_print(false, true);
      getScheduleStrAll_print(true, true);
      break;

    case SCHEDULE_FROM_EEPROM:
      //    getScheduleStrTad_print(false);
      getScheduleStrTad_print(true);
      break;

    default:
      break;
  }

  getUvLampModeEeprom();
  getScheduleEnableEeprom();

}  //end      void LightLogicControl::begin

String LightLogicControl::getScheduleStrAll_print(bool fprint, bool fSchedule) {
  //(lightState? true :false)
  for (int i; i < 42; i++) {
    awsMqtt.scheduleData[i] = fSchedule ? 0xff : 0x00;
    payload_manager.payload_deviceConfig_sub.scheduleData[i] = fSchedule ? 0xff : 0x00;
  }

  if (fprint) {
    Serial.println("payload_manager.payload_deviceConfig_sub.scheduleData:");
    for (int i = 0; i < 42; i++) {
      Serial.print(payload_manager.payload_deviceConfig_sub.scheduleData[i], HEX);
      Serial.print(" ");
      if (i % 8 == 0) { Serial.println(" "); }
    }
    Serial.println("");

    Serial.println("awsMqtt.scheduleData[i]:");
    for (int i = 0; i < 42; i++) {
      Serial.print(awsMqtt.scheduleData[i], HEX);
      Serial.print(" ");
      if (i % 8 == 0) { Serial.println(" "); }
    }


  }  //end    if(fprint)

  uint8_t scheduleData[42] = { 0 };

  int dataByteLength = EEPROM_SCHEDULE_SIZE;
  String textData = "";
  textData.reserve(dataByteLength * 16);  //8bit +8comma

  for (int i = 0; i < 42; i++) {
    scheduleData[i] = payload_manager.payload_deviceConfig_sub.scheduleData[i];
  }

  uint8_t temp;

  for (uint8_t x = 0; x < 42; x++) {
    for (uint8_t y = 0; y < 8; y++) {
      temp = scheduleData[x] & (1 << y);

      if (x == 41 && y == 7) {
        if (temp == 0) {
          textData = textData + '0';
        } else {
          textData = textData + '1';
        }
      } else {
        if (temp == 0) {
          textData = textData + '0' + ',';
        } else {
          textData = textData + '1' + ',';
        }

      }  //end    if(x==41 && y==7)

    }  //end  for(uint8_t y=0
  }    //end    for(uint8_t   x=0

  ScheduleStr = textData;

  if (fprint) {
    Serial.print("ScheduleStr:");
    Serial.println(ScheduleStr);
  }  //end    if(fprint)

  return ScheduleStr;

}  //end      String LightLogicControl::getScheduleStrAll_print

/*
* Function:
* 1) read schedule from eeprom into the local-buffer scheduleData[42]
* 2) output:     payload_manager.payload_deviceConfig_sub.scheduleData[i]  = scheduleData[i];
* 3) output:     awsMqtt.scheduleData[i]                                   = scheduleData[i];
* 4) output:     lightControl.ScheduleStr {1,0,0....} and  return this string
*/

String LightLogicControl::getScheduleStrTad_print(bool fprint) {
  if (fprint) {
    Serial.println("BEFORE:payload_deviceConfig_sub.scheduleData[i]:");
    for (int i = 0; i < 42; i++) {
      Serial.print(payload_manager.payload_deviceConfig_sub.scheduleData[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");

    Serial.println("BEFORE:awsMqtt.scheduleData[i]");
    for (int i = 0; i < 42; i++) {
      Serial.print(awsMqtt.scheduleData[i], HEX);
      Serial.print(" ");
    }  //end    for(int i=0;i<42;i++)
    Serial.println("");

  }  //end    if(fprint)

  uint8_t scheduleData[42] = { 0 };
  int ret = eeprom.read(EEPROM_SCHEDULE_START_ADDRESS, scheduleData, sizeof(scheduleData));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(EEPROM_SCHEDULE_START_ADDRESS);
    return "";  // Return empty string on read failure
  }

  for (int i = 0; i < 42; i++) {
    payload_manager.payload_deviceConfig_sub.scheduleData[i] = scheduleData[i];
    awsMqtt.scheduleData[i] = scheduleData[i];
  }

  if (fprint) {
    Serial.println("AFTER:payload_deviceConfig_sub.scheduleData[i]:");
    for (int i = 0; i < 42; i++) {
      Serial.print(payload_manager.payload_deviceConfig_sub.scheduleData[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");

    Serial.println("AFTER:awsMqtt.scheduleData[i]:");
    for (int i = 0; i < 42; i++) {
      Serial.print(awsMqtt.scheduleData[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }  //end      if(fprintf)

  int dataByteLength = EEPROM_SCHEDULE_SIZE;
  String textData = "";
  textData.reserve(dataByteLength * 16);  //8bit +8comma

  uint8_t temp = 0;

  for (uint8_t x = 0; x < 42; x++) {
    for (uint8_t y = 0; y < 8; y++) {
      temp = scheduleData[x] & (1 << y);

      if (x == 41 && y == 7) {
        if (temp == 0) {
          textData = textData + '0';
        } else {
          textData = textData + '1';
        }
      } else {
        if (temp == 0) {
          textData = textData + '0' + ',';
        } else {
          textData = textData + '1' + ',';
        }

      }  //end    if(x==41 && y==7)

    }  //end  for(uint8_t y=0
  }    //end    for(uint8_t   x=0


  ScheduleStr = textData;

  if (fprint) {
    Serial.print("ScheduleStr:");
    Serial.println(ScheduleStr);
  }  //end    if(fprint)

  return ScheduleStr;

}  //end    String LightLogicControl::getScheduleStrTad_print

String LightLogicControl::getScheduleStrTad() {
  Serial.println("LightLogicControl::getScheduleStrTad:");
  for (int i = 0; i < 42; i++) {
    Serial.print(payload_manager.payload_deviceConfig_sub.scheduleData[i]);
    Serial.print(" ");
  }
  Serial.println("");

  uint8_t scheduleData[42] = { 0 };

  int ret = eeprom.read(EEPROM_SCHEDULE_START_ADDRESS, scheduleData, sizeof(scheduleData));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(EEPROM_SCHEDULE_START_ADDRESS);
    return "";  // Return empty string on read failure
  }

  Serial.println("eeprom.read:");
  for (int i = 0; i < 42; i++) {
    Serial.print(scheduleData[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");

  int dataByteLength = EEPROM_SCHEDULE_SIZE;
  String textData = "";
  textData.reserve(dataByteLength * 16);  //8bit +8comma

  uint8_t temp = 0 ;

  for (uint8_t x = 0; x < 42; x++) {
    for (uint8_t y = 0; y < 8; y++) {
      temp = scheduleData[x] & (1 << y);

      if (x == 41 && y == 7) {
        if (temp == 0) {
          textData = textData + '0';
        } else {
          textData = textData + '1';
        }
      } else {
        if (temp == 0) {
          textData = textData + '0' + ',';
        } else {
          textData = textData + '1' + ',';
        }

      }  //end    if(x==41 && y==7)

    }  //end  for(uint8_t y=0
  }    //end    for(uint8_t   x=0

  ScheduleStr = textData;
  Serial.print("ScheduleStr:");
  Serial.println(ScheduleStr);
  return ScheduleStr;

}  //end    String LightLogicControl::getScheduleStrTad

String LightLogicControl::getScheduleStr() {
  Serial.println("LightLogicControl::getScheduleStr:");
  for (int i = 0; i < 42; i++) {
    Serial.print(payload_manager.payload_deviceConfig_sub.scheduleData[i]);
    Serial.print(" ");
  }
  Serial.println("");

  uint8_t scheduleData[42] = { 0 };
  int ret = eeprom.read(EEPROM_SCHEDULE_START_ADDRESS, scheduleData, sizeof(scheduleData));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(EEPROM_SCHEDULE_START_ADDRESS);
    return "";  // Return empty string on read failure
  }

  Serial.println("eeprom.read:");
  for (int i = 0; i < 42; i++) {
    Serial.print(scheduleData[i], HEX);
    Serial.print(" ");
    //Serial.print(  awsMqtt.scheduleData[i]  );
  }
  Serial.println("");


  int dataByteLength = EEPROM_SCHEDULE_SIZE;
  String textData = "";
  textData.reserve(dataByteLength * 3);  // eg 0xc1 become 'C'+‘1'; this is two byte now

  for (int i = 0; i < EEPROM_SCHEDULE_SIZE; i++) {
    uint8_t _scheduleData = scheduleData[i];
    // convert julesData to padded hex string
    char hexHigh = (_scheduleData >> 4) + '0';
    if (hexHigh > '9') {
      hexHigh += 7;  // Convert to A-F
    }
    char hexLow = (_scheduleData & 0x0F) + '0';
    if (hexLow > '9') {
      hexLow += 7;  // Convert to A-F
    }
    textData += hexHigh;
    textData += hexLow;
    textData += " ";
  }

  ScheduleStr = textData;
  Serial.print("ScheduleStr:");
  Serial.println(ScheduleStr);
  return ScheduleStr;

}  //end

/// @brief setup the periodic light process timer
void LightLogicControl::startPeriodicLightProcess() {
  GTimer.begin(0, (200 * 1000), gTimer0handler);  // 200ms
}

/// @brief initialize the 2 radar sensors
void LightLogicControl::initRadar() {
  radar1Valid = radar1.begin(0);
  if (radar1Valid) {
    Serial.println("Radar1 initialized successfully");
  } else {
    Serial.println("Radar1 initialization failed");
  }
  // radar1.disableRadarStreaming();
  radar2Valid = radar2.begin(1);
  if (radar2Valid) {
    Serial.println("Radar2 initialized successfully");
  } else {
    Serial.println("Radar2 initialization failed");
  }
}

/// @brief Check the radar data from both radar sensors
/// @param printData Whether to print the radar data in serial monitor
void LightLogicControl::checkRadarData(bool printData) {
  if (radar1Valid) {
    radar1.activeSerialMux();
    // radar1.enableRadarStreaming();
    int radarNewdata = radar1.checkRadarData(200);  // no enable/disable streaming, takes about 80ms, enable/disable
                                                     // streaming takes about 180ms
    if (printData) {
      radar1.debugDumpBuffers();
    }
    if (radarNewdata) {
      if (printData) {
        Serial.println("Radar1 data: ");
        for (int i = 0; i < radar1.radarObjectCount; i++) {
          Serial.print("Object ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(radar1.radarObject[i].x);
          Serial.print(", ");
          Serial.print(radar1.radarObject[i].y);
          Serial.print(", ");
          Serial.print(radar1.radarObject[i].speed);
          Serial.print(", ");
          Serial.println(radar1.radarObject[i].resolution);
        }
        if (radar1.radarObjectCount < 0) {
          Serial.println("Radar1 data error, no data received");
        }
      }  //end    if (printData)
    } else {
      if (printData) {
        Serial.println("No new data from Radar1");
      }
    }
    // radar1.disableRadarStreaming();
  } else {
    if (printData) {
      Serial.println("Radar1 is not valid, skipping data check.");
    }
  }

  if (radar2Valid) {
    radar2.activeSerialMux();
    int radarNewdata = radar2.checkRadarData(200);
    if (radarNewdata) {
      if (printData) {
        Serial.println("Radar2 data: ");
        for (int i = 0; i < radar2.radarObjectCount; i++) {
          Serial.print("Object ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(radar2.radarObject[i].x);
          Serial.print(", ");
          Serial.print(radar2.radarObject[i].y);
          Serial.print(", ");
          Serial.print(radar2.radarObject[i].speed);
          Serial.print(", ");
          Serial.println(radar2.radarObject[i].resolution);
        }
        if (radar2.radarObjectCount < 0) {
          Serial.println("Radar2 data error, no data received");
        }
      }
    } else {
      if (printData) {
        Serial.println("No new data from Radar2");
      }
    }
    // radar2.disableRadarStreaming();
  } else {
    if (printData) {
      Serial.println("Radar2 is not valid, skipping data check.");
    }
  }
}

/// @brief Initialize the thermal sensor (MLX90640)
void LightLogicControl::initThermalSensor() {
  thermalSensorValid = thermalSensor.begin(
    MLX90640_I2CADDR_DEFAULT);  // MLX90640 I2C address,
                                // MLX90640_ExtractParameters takes a long time
                                // but it is OK
  if (thermalSensorValid) {
    Serial.println("Thermal sensor initialized successfully");
    Serial.print("Serial number: ");
    Serial.print(thermalSensor.serialNumber[0], HEX);
    Serial.print(thermalSensor.serialNumber[1], HEX);
    Serial.println(thermalSensor.serialNumber[2], HEX);
    thermalSensor.setMode(MLX90640_CHESS);
    // Set the resolution and refresh rate
    thermalSensor.setResolution(MLX90640_ADC_18BIT);
    thermalSensor.setRefreshRate(MLX90640_4_HZ);

    for (int i = 0; i < 2; i++) {
      float frame[32 * 24];
      thermalSensor.getFrame(frame);
      // discard first 2 frames
    }
  } else {
    Serial.println("Thermal sensor initialization failed");
  }
}

/// @brief Check the thermal sensor data and process it with MLX90640 Yolo Processor
/// @param printData Whether to print the thermal sensor data in serial monitor
void LightLogicControl::checkThermalSensorData(bool printData) {
  if (thermalSensorValid) {
    float frame[32 * 24];

    int ret = thermalSensor.getFrame(frame);

    if (ret == 0) {
      if (printData) {
        Serial.println("Thermal sensor data:");
        for (int i = 0; i < 32 * 24; i++) {
          Serial.print(frame[i], 2);
          Serial.print(" ");
        }
        Serial.println();
      }
      if (initMLX90640YoloProcessor() == 0) {
        feedFloatMLX90640YoloProcessorAndRun(frame);
        deinitMLX90640YoloProcessor();
        thermalDetectionCount = getDetectionsCount(&thermalDetections);
      } else {
        Serial.println("Failed to allocate MLX90640 Yolo Processor");
      }
    } else {
      if (printData) {
        Serial.println("Failed to get thermal sensor data");
      }
      thermalDetectionCount = -1;  // Reset detection count on error
    }
  } else {
    if (printData) {
      Serial.println("Thermal sensor is not valid, skipping data check.");
    }
  }
}

/// @brief Empty now
void LightLogicControl::update() {}

/// @brief Set the UI LED state
/// @param ledIndex The index of the LED to set
/// @param state The state to set the LED to (ON/OFF/Blink)
void LightLogicControl::setUiLedState(int ledIndex, int state) {
  int gTimer0LedStateCache = gTimer0LedState;
  gTimer0LedStateCache &= ~(3 << (ledIndex * 2));     // clear led state
  gTimer0LedStateCache |= (state << (ledIndex * 2));  // set led state
  gTimer0LedState = gTimer0LedStateCache;
}


float LightLogicControl::getlength(uint32_t startTime, uint32_t endTime) {
  bool fprint = false;
  float _length = 0;

  if (((endTime > startTime) && (startTime > 0) && (endTime > 0)) || ((endTime == startTime) && (startTime > 0))) {

    _length = (float)(endTime - startTime) / 60;
    if (_length < 0) {
      _length = 0;
    }

    if (fprint) {
      Serial.print("startTime:");
      Serial.println(startTime);
      Serial.print("endTime:");
      Serial.println(endTime);
      Serial.print("_length:");
      Serial.println(_length);
    }
    lampOnRecordCache.length = _length;
  }

  return _length;

}  //end    uint32_t    LightLogicControl::length

#define UNDEFINED 255
#define UV_LAMP_ON 1
#define UV_LAMP_OFF 0

void LightLogicControl::setLightState(bool state) {

  bool accumulatedExposureExceedThreshold = accumulatedExposure >= accumulatedExposureThreshold;

  uint32_t currentUixTime = this->currentRtcTime.unixtime();

  if (state && !accumulatedExposureExceedThreshold) {

    lightState = true;

    if (!fSetStartTime) {
      fSetStartTime = true;

      lampOnRecordCache.lampOnTimeStamps_start = currentUixTime;
      lampOnRecordCache.lampOnTimeStamps_end = currentUixTime;
      lampOnRecordCache.length = 0;
      lampOnRecordCache.reason = "trigger turn on";
    }  //end if


  } else {
    // If attempting to turn off OR if the accumulatedExposure exceeds threshold, always set lamp OFF

    lightState = false;

    if (fSetStartTime) {

      fSetStartTime = false;

      lampOnRecordCache.lampOnTimeStamps_end = currentUixTime;
      lampOnRecordCache.length = (getlength(lampOnRecordCache.lampOnTimeStamps_start, lampOnRecordCache.lampOnTimeStamps_end));
      lampOnRecordCache.reason = "turn off for any reason";

    }  //end      if(fSetStartTime)

  }  //end else

  if (accumulatedExposureExceedThreshold){
    Serial.println("Accumulated exposure exceed threshold, blinking white LED");
    setUiLedState(UI_LED_WHITE, UI_LED_BLINK);
  } else {
    Serial.print("Set white LED to: ");
    Serial.println(lightState ? "ON" : "OFF");
    setUiLedState(UI_LED_WHITE, lightState ? UI_LED_ON : UI_LED_OFF);
  }


  beacon_pcb.ballastPowerControl(lightState ? true : false);
  digitalWrite(PA12, lightState ? LOW : HIGH);  // ballast on when Low

}  //end  void LightLogicControl::setLightState

bool LightLogicControl::getLightState() {
  return lightState;
}

#define DETECT_DISTANCE_OFFSET 50  // measure a farther distance, so we need to minus this offset

/// @brief Use sensor data to calculate the minimal distance and jules level
/// @param printData Whether to print the sensor data in serial monitor
/// @return 1 if objects detected, 0 if no objects detected
int LightLogicControl::processSensorInfo(bool printData) {

  int r1_min = INT_MAX;
  int r2_min = INT_MAX;

  bool objectDetected = false;

  if (radar1Valid) {
    if (radar1.radarObjectCount > 0) {
      if (printData) {
        Serial.println("Radar1 detected objects:");
        for (int i = 0; i < radar1.radarObjectCount; i++) {
          Serial.print("Object ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(radar1.radarObject[i].x);
          Serial.print(", ");
          Serial.print(radar1.radarObject[i].y);
          Serial.print(", ");
          Serial.print(radar1.radarObject[i].speed);
          Serial.print(", ");
          Serial.println(radar1.radarObject[i].resolution);
        }
      }
    } else {
      if (printData) {
        Serial.println("Radar1 detected no objects.");
      }
    }
  }
  if (radar2Valid) {
    if (radar2.radarObjectCount > 0) {
      if (printData) {
        Serial.println("Radar2 detected objects:");
        for (int i = 0; i < radar2.radarObjectCount; i++) {
          Serial.print("Object ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(radar2.radarObject[i].x);
          Serial.print(", ");
          Serial.print(radar2.radarObject[i].y);
          Serial.print(", ");
          Serial.print(radar2.radarObject[i].speed);
          Serial.print(", ");
          Serial.println(radar2.radarObject[i].resolution);
        }
      }
    } else {
      if (printData) {
        Serial.println("Radar2 detected no objects.");
      }
    }
  }
  if (thermalSensorValid) {
    if (thermalDetectionCount > 0) {
      if (printData) {
        Serial.println("Thermal sensor detected objects:");
        for (int i = 0; i < thermalDetectionCount; i++) {
          Serial.print("Detection ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(thermalDetections[i].x);
          Serial.print(", ");
          Serial.print(thermalDetections[i].y);
          Serial.print(", ");
          Serial.print(thermalDetections[i].w);
          Serial.print(", ");
          Serial.println(thermalDetections[i].h);
        }
      }
    } else {
      if (printData) {
        Serial.println("Thermal sensor detected no objects.");
      }
    }
  }

  int minimalDistance = INT_MAX;

  if (radar1Valid && radar1.radarObjectCount > 0) {
    for (int i = 0; i < radar1.radarObjectCount; i++) {
      int r1_distance = radar1.radarObject[i].x * radar1.radarObject[i].x + radar1.radarObject[i].y * radar1.radarObject[i].y;
      r1_distance = sqrt(r1_distance);  // Calculate Euclidean distance
      radar1.radarObject[i].distance = r1_distance;

      Serial.print("radar1 object");
      Serial.print(i);
      Serial.print(" distance:");
      Serial.println(r1_distance);
      if (r1_distance < r1_min) {
        r1_min = r1_distance;
      }
    }
  }

  if (radar2Valid && radar2.radarObjectCount > 0) {
    for (int i = 0; i < radar2.radarObjectCount; i++) {
      int r2_distance = radar2.radarObject[i].x * radar2.radarObject[i].x + radar2.radarObject[i].y * radar2.radarObject[i].y;
      r2_distance = sqrt(r2_distance);  // Calculate Euclidean distance
      radar2.radarObject[i].distance = r2_distance;

      Serial.print("radar2 object");
      Serial.print(i);
      Serial.print(" distance:");
      Serial.println(r2_distance);
      if (r2_distance < r2_min) {
        r2_min = r2_distance;
      }
    }
  }

  // Take minimal distance from both radars
  minimalDistance = r1_min < r2_min ? r1_min : r2_min;

  if (thermalSensorValid && thermalDetectionCount > 0) {
    for (int i = 0; i < thermalDetectionCount; i++) {
      int distance = 0;
      float w = thermalDetections[i].w;
      float h = thermalDetections[i].h;
      float area = w * h;
      if (area > 50) {
        distance = 900;
      } else if (area > 20) {
        distance = 2400;
      } else {
        distance = 3000;  // No really detection
      }
      thermalDetections[i].distance = distance;

      Serial.print("thermal object");
      Serial.print(i);
      Serial.print(" distance:");
      Serial.println(distance);
      if (distance < minimalDistance) {
        minimalDistance = distance;
      }
    }
  }

  if (printData) {
    Serial.print("Temperature: ");
    String temperatureDataStr = String(temperatureData, 2);  // Format to 2 decimal places
    Serial.print(temperatureDataStr);
    Serial.println(" °C");
  }

  // If we had valid data to calculate minimalDistance, apply the distance offset
  if ((radar1Valid && radar1.radarObjectCount > 0) || (radar2Valid && radar2.radarObjectCount > 0) || (thermalSensorValid && thermalDetectionCount > 0)) {
    calculatedMinimalDistance = minimalDistance - DETECT_DISTANCE_OFFSET;
    objectDetected = true;
    if (calculatedMinimalDistance < 0) {
      calculatedMinimalDistance = 0;
    }
  } else if (!radar1Valid && !radar2Valid && !thermalSensorValid) {
    // No valid data, set value to average goes down to ensure lamp eventually turns off
    calculatedMinimalDistance = 0;  //
  } else {
    // All sensors are valid but no objects detected, set to a high value so that lowest jules value is accumulated
    // TODO this value is quite arbitrary.  Tune it?
    calculatedMinimalDistance = 5050;
  }

  // Rotate the new value of calculatedMinimalDistance into the array
  minimalDistanceHistory[historyIndex] = calculatedMinimalDistance;
  historyIndex = (historyIndex + 1) % 5;

  // Calculate the smart average of the 5 values in the array (excluding sentinel values)
  int64_t sum = 0;
  for (int i = 0; i < MINIMAL_DISTANCE_AVERAGE_COUNT; i++) {
      sum += minimalDistanceHistory[i];
  }

  averagedCalculatedMinimalDistance = (int)(sum / 5);

  minimalDistanceJulesLevel = beaconEnergyEngineer.jules16LevelForDistance(averagedCalculatedMinimalDistance);

  if (printData) {
    Serial.println("_____________________________________ Jules _______________________________________");
    Serial.print("Minimal distance: ");
    Serial.println(averagedCalculatedMinimalDistance);
    Serial.print("Minimal distance in Jules Level: ");
    Serial.print(minimalDistanceJulesLevel);
    Serial.print(", Jules: ");
    Serial.println(exposure16LevelToJules[minimalDistanceJulesLevel]);
  }

  Serial.print("object:");
  Serial.println(objectDetected ? "Yes" : "No");

  return objectDetected ? 1 : 0;  // Return 0 for no objects detected, 1 for objects detected
}

/// @brief convert distance to exposure level (4 levels)
/// @param distance The distance in mm
int LightLogicControl::julesLevelForDistance(int distance) {
  // TODO: confirm relationship between distance and jules
  int julesLevel = EXPOSURE_LEVEL_0;

  if (distance <= 1000) {
    julesLevel = EXPOSURE_LEVEL_3;

  } else if (distance <= 2500) {
    julesLevel = EXPOSURE_LEVEL_2;

  } else if (distance <= 4000) {
    julesLevel = EXPOSURE_LEVEL_1;
  }

  return julesLevel;
}


String LightLogicControl::getDiagnosticsLevelJson_254() {
  String str_averagedCalculatedMinimalDistance = String( averagedCalculatedMinimalDistance ); 
  String str_exposure16LevelToJules    = String( exposure16LevelToJules[minimalDistanceJulesLevel] );     

  int     accumulatedExposure          = eepromGetAccumulatedExposure_16Level(inProgress8hourSectionStartTime);
  String  str_accumulatedExposure      = String( accumulatedExposure );

  int rtcUnixtime = this->rtc.outputNowUnixtime();
  String rtcTimeStr = String(rtcUnixtime);
  //rtcTimeStr = "\"" + rtcTimeStr + "\"";

  String sendJson =
    "{\"MinimalDistance\":" + str_averagedCalculatedMinimalDistance + ",\"Jules\":" + str_exposure16LevelToJules + 
    ",\"accumulatedExposure\":" + str_accumulatedExposure   + ",\"timestamp\":" + rtcTimeStr + "}";

  return sendJson;

}  //end    String LightLogicControl::getDiagnosticsLevelJson_254


String LightLogicControl::getDiagnosticsLevelJson_255() {
  int radar1Count = radar1.radarObjectCount;
  int radar2Count = radar2.radarObjectCount;
  int thermalCount = thermalDetectionCount;

  String radar1Json = "";
  for (int i = 0; i < radar1Count; i++) {
    radar1Json +=
      "{\"x\":" + String(radar1.radarObject[i].x) + ",\"y\":" + String(radar1.radarObject[i].y) +
      //"\x1b[32m distance:\x1b[0m"+ String(radar1.radarObject[i].distance )+
      ",\"DISTANCE\":" + String(radar1.radarObject[i].distance) + ",\"speed\":" + String(radar1.radarObject[i].speed) + ",\"resolution\":" + String(radar1.radarObject[i].resolution) + "}";
    if (i < radar1Count - 1) {
      radar1Json += ",";
    }
  }  //end    for (int i = 0; i < radar1Count;

  if ((radar1Count >= 0) && radar1Valid) {
    radar1Json = "[" + radar1Json + "]";
  } else {
    radar1Json = "\"error\"";  // Handle error case for radar1
  }

  String radar2Json = "";
  for (int i = 0; i < radar2Count; i++) {
    radar2Json +=
      "{\"x\":" + String(radar2.radarObject[i].x) + ",\"y\":" + String(radar2.radarObject[i].y) +
      //"\x1b[32m distance:\x1b[0m"+  String(radar2.radarObject[i].distance )+
      ",\"DISTANCE\":" + String(radar2.radarObject[i].distance) + ",\"speed\":" + String(radar2.radarObject[i].speed) + ",\"resolution\":" + String(radar2.radarObject[i].resolution) + "}";
    if (i < radar2Count - 1) {
      radar2Json += ",";
    }
  }  //end    for (int i = 0; i < radar2Count

  if ((radar2Count >= 0) && radar2Valid) {
    radar2Json = "[" + radar2Json + "]";
  } else {
    radar2Json = "\"error\"";  // Handle error case for radar2
  }

  String thermalJson = "";
  for (int i = 0; i < thermalCount; i++) {
    thermalJson += "{\"x\":" + String(thermalDetections[i].x) + ",\"y\":" + String(thermalDetections[i].y) + ",\"w\":" + String(thermalDetections[i].w) + ",\"h\":" + String(thermalDetections[i].h) +
                   //"\x1b[32m DISTANCE:\x1b[0m"+
                   ",\"DISTANCE\":" + String(thermalDetections[i].distance) + "}";
    if (i < thermalCount - 1) {
      thermalJson += ",";
    }
  }
  if ((thermalCount >= 0) && thermalSensorValid) {
    thermalJson = "[" + thermalJson + "]";
  } else {
    thermalJson = "\"error\"";  // Handle error case for thermal sensor
  }

  int rtcUnixtime = this->rtc.outputNowUnixtime();
  String rtcTimeStr = String(rtcUnixtime);
  rtcTimeStr = "\"" + rtcTimeStr + "\"";


  String sendJson =
    "{\"radar1Events\":" + radar1Json + ",\"radar2Events\":" + radar2Json + ",\"thermalEvents\":" + thermalJson + ",\"timestamp\":" + rtcTimeStr + "}";

  return sendJson;

}  //end    String LightLogicControl::getDiagnosticsLevelJson_255()



String LightLogicControl::getDiagnosticsLevelJson_2() {
  int radar1Count = radar1.radarObjectCount;
  int radar2Count = radar2.radarObjectCount;
  int thermalCount = thermalDetectionCount;

  String radar1Json = "";
  for (int i = 0; i < radar1Count; i++) {
    radar1Json +=
      "{\"x\":" + String(radar1.radarObject[i].x) + ",\"y\":" + String(radar1.radarObject[i].y) + ",\"speed\":" + String(radar1.radarObject[i].speed) + ",\"resolution\":" + String(radar1.radarObject[i].resolution) + "}";
    if (i < radar1Count - 1) {
      radar1Json += ",";
    }
  }
  if ((radar1Count >= 0) && radar1Valid) {
    radar1Json = "[" + radar1Json + "]";
  } else {
    radar1Json = "\"error\"";  // Handle error case for radar1
  }
  String radar2Json = "";
  for (int i = 0; i < radar2Count; i++) {
    radar2Json +=
      "{\"x\":" + String(radar2.radarObject[i].x) + ",\"y\":" + String(radar2.radarObject[i].y) + ",\"speed\":" + String(radar2.radarObject[i].speed) + ",\"resolution\":" + String(radar2.radarObject[i].resolution) + "}";
    if (i < radar2Count - 1) {
      radar2Json += ",";
    }
  }
  if ((radar2Count >= 0) && radar2Valid) {
    radar2Json = "[" + radar2Json + "]";
  } else {
    radar2Json = "\"error\"";  // Handle error case for radar2
  }
  String thermalJson = "";
  for (int i = 0; i < thermalCount; i++) {
    thermalJson += "{\"x\":" + String(thermalDetections[i].x) + ",\"y\":" + String(thermalDetections[i].y) + ",\"w\":" + String(thermalDetections[i].w) + ",\"h\":" + String(thermalDetections[i].h) + "}";
    if (i < thermalCount - 1) {
      thermalJson += ",";
    }
  }
  if ((thermalCount >= 0) && thermalSensorValid) {
    thermalJson = "[" + thermalJson + "]";
  } else {
    thermalJson = "\"error\"";  // Handle error case for thermal sensor
  }

  int rtcUnixtime = this->rtc.outputNowUnixtime();
  String rtcTimeStr = String(rtcUnixtime);
  rtcTimeStr = "\"" + rtcTimeStr + "\"";

  String commandTraceStr = "to be defined";
  commandTraceStr = "\"" + commandTraceStr + "\"";

  String sendJson =
    "{\"radar1Events\":" + radar1Json + ",\"radar2Events\":" + radar2Json + ",\"thermalEvents\":" + thermalJson + ",\"timestamp\":" + rtcTimeStr + ",\"commandTrace\":" + commandTraceStr + "}";

  return sendJson;
}  //end      String

String LightLogicControl::getDiagnosticsLevelJson_1() {
  float _temperatureData;
  bool _lampEnable;
  int _personCount;
  int _accumulatedExposure;

  _temperatureData = temperatureData;
  _lampEnable = (uvLampMode != UV_MODE_OFF);
  _personCount = getpersonCount();
  _accumulatedExposure = accumulatedExposure;


  String _temperatureDataStr = String(_temperatureData, 2);  // Format to 2 decimal places
  if (isnan(_temperatureData)) {
    _temperatureDataStr = "null";  // Handle NaN case
  } else {
    _temperatureDataStr =
      "\"" + _temperatureDataStr + "\"";  // Wrap in quotes for JSON
  }

  String _lampEnableStr = String(_lampEnable ? "true" : "false");
  _lampEnableStr = "\"" + _lampEnableStr + "\"";

  String _personCountStr = String(_personCount);
  _personCountStr = "\"" + _personCountStr + "\"";

  String _accumulatedExposureStr = String(_accumulatedExposure);
  _accumulatedExposureStr = "\"" + _accumulatedExposureStr + "\"";

  String sendJson =
    "{\"temperature\":" + _temperatureDataStr + ",\"lampEnable\":" + _lampEnableStr + ",\"personCount\":" + _personCountStr + ",\"accumulatedExposure\":" + _accumulatedExposureStr + "}";

  return sendJson;

}  //end String


String LightLogicControl::getDiagnosticsLevelJson_0() {
  uint32_t uptimeSeconds = (millis() - program_StartUpTime) / 1000;

  uint8_t lastError = errorStatus;

  String uptimeSecondsStr = String(uptimeSeconds);
  uptimeSecondsStr = "\"" + uptimeSecondsStr + "\"";

  String lastErrorStr = String(lastError);
  lastErrorStr = "\"" + lastErrorStr + "\"";

  String sendJson =
    "{\"uptimeSeconds\":" + uptimeSecondsStr + ",\"lastError\":" + lastErrorStr + "}";

  return sendJson;

}  //end String

/// @brief Get the sensor data in JSON format for sending to the server
/// @return JSON string containing radar, thermal, temperature, RTC, EEPROM data
String LightLogicControl::getSensorDataJson() {
  int radar1Count = radar1.radarObjectCount;
  int radar2Count = radar2.radarObjectCount;
  int thermalCount = thermalDetectionCount;

  String radar1Json = "";
  for (int i = 0; i < radar1Count; i++) {
    radar1Json +=
      "{\"x\":" + String(radar1.radarObject[i].x) + ",\"y\":" + String(radar1.radarObject[i].y) + ",\"speed\":" + String(radar1.radarObject[i].speed) + ",\"resolution\":" + String(radar1.radarObject[i].resolution) + "}";
    if (i < radar1Count - 1) {
      radar1Json += ",";
    }
  }
  if ((radar1Count >= 0) && radar1Valid) {
    radar1Json = "[" + radar1Json + "]";
  } else {
    radar1Json = "\"error\"";  // Handle error case for radar1
  }
  String radar2Json = "";
  for (int i = 0; i < radar2Count; i++) {
    radar2Json +=
      "{\"x\":" + String(radar2.radarObject[i].x) + ",\"y\":" + String(radar2.radarObject[i].y) + ",\"speed\":" + String(radar2.radarObject[i].speed) + ",\"resolution\":" + String(radar2.radarObject[i].resolution) + "}";
    if (i < radar2Count - 1) {
      radar2Json += ",";
    }
  }
  if ((radar2Count >= 0) && radar2Valid) {
    radar2Json = "[" + radar2Json + "]";
  } else {
    radar2Json = "\"error\"";  // Handle error case for radar2
  }
  String thermalJson = "";
  for (int i = 0; i < thermalCount; i++) {
    thermalJson += "{\"x\":" + String(thermalDetections[i].x) + ",\"y\":" + String(thermalDetections[i].y) + ",\"w\":" + String(thermalDetections[i].w) + ",\"h\":" + String(thermalDetections[i].h) + "}";
    if (i < thermalCount - 1) {
      thermalJson += ",";
    }
  }
  if ((thermalCount >= 0) && thermalSensorValid) {
    thermalJson = "[" + thermalJson + "]";
  } else {
    thermalJson = "\"error\"";  // Handle error case for thermal sensor
  }

  int rtcUnixtime = this->rtc.outputNowUnixtime();

  String rtcTimeStr = String(rtcUnixtime);
  rtcTimeStr = "\"" + rtcTimeStr + "\"";

  //Serial.println("====================================================================================");
  Serial.print("rtcUnixtime");
  Serial.println(rtcUnixtime);

  String temperatureDataStr =
    String(temperatureData, 2);  // Format to 2 decimal places
  if (isnan(temperatureData)) {
    temperatureDataStr = "null";  // Handle NaN case
  } else {
    temperatureDataStr =
      "\"" + temperatureDataStr + "\"";  // Wrap in quotes for JSON
  }

  String accumulatedExposureStr = String(accumulatedExposure);
  accumulatedExposureStr =
    "\"" + accumulatedExposureStr + "\"";  // Wrap in quotes for JSON

  String eightHourSectionStartTimeStr = String(inProgress8hourSectionStartTime);
  eightHourSectionStartTimeStr = "\"" + eightHourSectionStartTimeStr + "\"";

  String eightHourSectionDataStr =
    eepromGetSectionData_16Level(inProgress8hourSectionStartTime);

  String compileTimeStr = String(compiledTimeStr);

  String lampOnTimeForLoggingInIntervalUnitStr = String(lightOnTimeForLoggingInIntervalUnit);

  String sendJson =
    "{\"rawdata\":{\"radar1\":" + radar1Json + ",\"radar2\":" + radar2Json + ",\"thermal\":" + thermalJson + ",\"temperature\":" + temperatureDataStr + ",\"rtcTime\":" + rtcTimeStr + ",\"accumulatedExposure\":" + accumulatedExposureStr + ",\"eightHourSectionStartTime\":" + eightHourSectionStartTimeStr + ",\"eightHourSectionData\":" + eightHourSectionDataStr + ",\"lampOnTime15Minute\":" + lampOnTimeForLoggingInIntervalUnitStr + "},\"clientToken\":\"" + String(NonvolatileDataManager::getClientId()) + "\",\"compileTime\":\"" + compileTimeStr + "\"}";

  return sendJson;
}

/// @brief Initialize the temperature sensor TMP102
void LightLogicControl::initTemperatureSensor() {
  temperatureSensorValid = temperatureSensor.begin();
  if (temperatureSensorValid) {
    Serial.println("Temperature sensor initialized successfully");
  } else {
    Serial.println("Temperature sensor initialization failed");
  }
}

/// @brief Get the temperature data from the temperature sensor
/// @return Temperature data in Celsius, or NaN if the sensor is not valid or data is not available
float LightLogicControl::getTemperatureData() {
  if (temperatureSensorValid) {
    temperatureData = temperatureSensor.checkTemperatureData();
    if (!isnan(temperatureData)) {
      return temperatureData;
    } else {
      Serial.println("Failed to read temperature data.");
      return NAN;
    }
  } else {
    Serial.println("Temperature sensor is not valid, skipping data check.");
    return NAN;
  }
}

/// @brief Initialize the RTC (Real Time Clock) DS3231
/// @note This function initializes the RTC and sets the current time if the RTC is valid.
void LightLogicControl::initRTC() {
  rtcValid = rtc.begin();
  if (rtcValid) {
    Serial.println("RTC initialized successfully");

    currentRtcTime = rtc.outputNowDateTime();
    //    currentRtcTime = rtc.now();
    Serial.print("Current RTC time: ");
    Serial.println(currentRtcTime.timestamp());
    Serial.print("UnixTime:");
    Serial.println(currentRtcTime.unixtime());

  } else {
    Serial.println("RTC initialization failed");
  }
}

/// @brief Get the current RTC time
/// @return Current RTC time as DateTime object
DateTime LightLogicControl::getCurrentRtcTime() {
  if (rtcValid) {
    currentRtcTime = rtc.now();
    return currentRtcTime;
  } else {
    Serial.println("RTC is not valid, returning default time.");
    return DateTime();  // Return default DateTime
  }
}

/// @brief Get the start time of the current 8-hour section
/// @return Start time of the current 8-hour section in seconds since epoch
uint32_t LightLogicControl::get8hourSectionStartTime() {
  if (rtcValid) {
    currentRtcTime = rtc.outputNowDateTime();

    uint32_t currentTime = currentRtcTime.unixtime();
    uint32_t sectionStartTime =
      currentTime - (currentTime % 28800);  // 28800 seconds = 8 hours
    inProgress8hourSectionStartTime = sectionStartTime;
    return sectionStartTime;
  } else {
    Serial.println("RTC is not valid, returning default time.");
    return 0;  // Return 0 if RTC is not valid
  }
}

/// @brief Initialize the EEPROM 24C08 for logging
void LightLogicControl::initEEPROM() {
  eepromValid = eeprom.begin();
  if (eepromValid) {
    Serial.println("EEPROM initialized successfully");
  } else {
    Serial.println("EEPROM initialization failed");
  }
}

uint8_t LightLogicControl::findWhichLevelSection(uint32_t sectionStartTime) {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section initialization.");
    return -1;  // Return -1 if EEPROM is not valid
  }

  uint32_t minimalStartTime = 0xFFFFFFFF;  // Initialize to a large value

  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRIES; i++) {
    uint16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + i * EEPROM_16LEVEL_LOG_ENTRY_SIZE;
    uint8_t startLogTime[4] = { 0 };  // Initialize data to zero
    int ret = eeprom.read(memAddress, startLogTime, sizeof(startLogTime));

    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(memAddress);
      return -1;  // Return -1 on read failure
    }

    // Check if the section start time matches
    uint32_t storedSectionStartTime;
    memcpy(&storedSectionStartTime, startLogTime, sizeof(storedSectionStartTime));

    if (storedSectionStartTime > sectionStartTime) {
      storedSectionStartTime =
        0;  // Reset if the stored time is greater than the section start time
    }

    if (storedSectionStartTime < minimalStartTime) {
      minimalStartTime = storedSectionStartTime;
    }

    if (storedSectionStartTime == sectionStartTime) {
      Serial.print("Section already initialized at  ");
      Serial.println(i);
      return i;  // Return the address if section is already initialized
    }

  }  //end for

  return -1;

}  //end    uint8_t   LightLogicControl::findWhichLevelSection


int LightLogicControl::eepromClear_16LevelSection(uint8_t index, uint32_t sectionStartTime) {
  uint16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + index * EEPROM_16LEVEL_LOG_ENTRY_SIZE;

  uint8_t sectionData[EEPROM_16LEVEL_LOG_ENTRY_SIZE] = { 0 };  // Initialize data to zero
  memcpy(sectionData, &sectionStartTime, sizeof(sectionStartTime));

  int ret = eeprom.write(memAddress, sectionData, sizeof(sectionData));
  if (ret < 0) {
    Serial.print("Failed to write EEPROM at address ");
    Serial.println(memAddress);
    return -1;  // Return -1 on write failure
  }

  return 1;

}  //end    int LightLogicControl::eepromClear_16LevelSection

/*
* Function
* 1) looking for which 8hr session in two section
* 2) if this sectionStartTime is not existing, replace the oldest session with this given sectionStartTime
* 3) all the energy level is 0 in this section
*
*/
int LightLogicControl::eepromInit_16LevelSection(uint32_t sectionStartTime) {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section initialization.");
    return -1;  // Return -1 if EEPROM is not valid
  }

  uint32_t minimalStartTime = 0xFFFFFFFF;  // Initialize to a large value
  int minimalStartTimeSectionIndex = 0;    // Initialize to -1 to indicate no valid address found

  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRIES; i++) {
    uint16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + i * EEPROM_16LEVEL_LOG_ENTRY_SIZE;
    uint8_t startLogTime[4] = { 0 };  // Initialize data to zero
    int ret = eeprom.read(memAddress, startLogTime, sizeof(startLogTime));

    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(memAddress);
      return -1;  // Return -1 on read failure
    }

    // Check if the section start time matches
    uint32_t storedSectionStartTime;
    memcpy(&storedSectionStartTime, startLogTime, sizeof(storedSectionStartTime));

    if (storedSectionStartTime > sectionStartTime) {
      storedSectionStartTime =
        0;  // Reset if the stored time is greater than the section start time
    }

    if (storedSectionStartTime < minimalStartTime) {
      minimalStartTime = storedSectionStartTime;
      minimalStartTimeSectionIndex = i;
    }

    if (storedSectionStartTime == sectionStartTime) {
      Serial.print("Section already initialized at  ");
      Serial.println(i);
      return i;  // Return the address if section is already initialized
    }

  }  //end for

  // init the section with the minimal start time
  Serial.print("Initializing section at index ");
  Serial.println(minimalStartTimeSectionIndex);
  uint16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + minimalStartTimeSectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE;

  uint8_t sectionData[EEPROM_16LEVEL_LOG_ENTRY_SIZE] = { 0 };  // Initialize data to zero
  memcpy(sectionData, &sectionStartTime, sizeof(sectionStartTime));

  int ret = eeprom.write(memAddress, sectionData, sizeof(sectionData));
  if (ret < 0) {
    Serial.print("Failed to write EEPROM at address ");
    Serial.println(memAddress);
    return -1;  // Return -1 on write failure
  }

  return minimalStartTimeSectionIndex;

}  //end      int LightLogicControl::eepromInit_16LevelSection


/*
* Function
* 1) write schedule into eeprom
* 2) optional vertify it
*/

int LightLogicControl::eepromWriteSchedule() {
  bool fprint = true;

  int readAddress = EEPROM_SCHEDULE_START_ADDRESS;
  uint8_t* schedule = payload_manager.payload_deviceConfig_sub.scheduleData;  //   uint8_t schedule[42] =    payload_manager.payload_deviceConfig_sub.scheduleData;

  if (fprint) {
    Serial.println("_________________  LightLogicControl::eepromWriteSchedule _______________________");
    Serial.println("uint8_t* schedule:");

    for (int i = 0; i < 42; i++) {
      if (schedule[i] <= 9) { Serial.print("0"); }
      Serial.print(schedule[i], HEX);
      Serial.print(" ");
    }  //end    for
    Serial.println("");

  }  //end    if(fprint)

  int ret = eeprom.write(readAddress, schedule, EEPROM_SCHEDULE_SIZE);
  if (ret < 0) {
    Serial.print("Failed to write EEPROM at address ");
    Serial.println(readAddress);
    return -1;  // Return -1 on write failure
  }


  if (true) {  //vertify
    for (int i = 0; i < EEPROM_SCHEDULE_SIZE; i++) {
      schedule[i] = 0x00;
    }

    ret = eeprom.read(readAddress, schedule, EEPROM_SCHEDULE_SIZE);
    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(readAddress);
      return -1;  // Return -1 on read failure
    }

    Serial.println("eeprom.read:uint8_t* schedule:");
    for (int i = 0; i < 42; i++) {
      if (schedule[i] <= 9) { Serial.print("0"); }
      Serial.print(schedule[i], HEX);
      Serial.print(" ");
    }  //end      for ( int i=0; i<42; i++ )
    Serial.println("");

  }  //end

  return 0;

}  //end      int LightLogicControl::eepromWriteSchedule

int LightLogicControl::eepromRead16LevelSection(uint32_t sectionStartTime) {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section write.");
    return -1;  // Return -1 if EEPROM is not valid
  }

  int sectionIndex = -1;
  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRIES; i++) {
    int16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + i * EEPROM_16LEVEL_LOG_ENTRY_SIZE;
    uint8_t startLogTime[4] = { 0 };  // Initialize data to zero
    int ret = eeprom.read(memAddress, startLogTime, sizeof(startLogTime));

    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(memAddress);
      return -1;  // Return -1 on read failure
    }             //end      if (ret < 0)

    uint32_t storedSectionStartTime;
    memcpy(&storedSectionStartTime, startLogTime,
           sizeof(storedSectionStartTime));

    if (storedSectionStartTime == sectionStartTime) {
      sectionIndex = i;
      break;  // Found the section
    }
  }  //end for

  if (sectionIndex < 0) {
    Serial.println("Section not found, cannot write data.");
    return -1;  // Return -1 if section is not found
  }


  Serial.println("___________________________ readData[EEPROM_16LEVEL_LOG_ENTRY_SIZE]  ________________________________________");
  // print debug info
  uint8_t readData[EEPROM_16LEVEL_LOG_ENTRY_SIZE] = { 0 };  // Initialize data to zero

  int ret = eeprom.read(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE,
                        readData, sizeof(readData));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE);
    return -1;  // Return -1 on read failure
  }

  Serial.print("EEPROM section data at index ");
  Serial.print(sectionIndex);
  Serial.println(": ");

  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRY_SIZE; i++) {
    Serial.print(readData[i], HEX);
    Serial.print(" ");

    if (((i % 10) == 0) && (i > 0)) { Serial.println(); }
    //  if( ( (i%10)==0 ) && (i>0)  || (i==3)){  Serial.println(); }
  }
  Serial.println();


  return 0;  // Success
}  //end

/*
* Function
* 1) Find out which section of two section
* 2) at specific minute of this section, update the energy level
*
*/
int LightLogicControl::eepromWrite16LevelSection(uint32_t sectionStartTime, int julesLevel, int dataPointIndex) {


  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section write.");
    return -1;  // Return -1 if EEPROM is not valid
  }

  if (false) {
    Serial.print("sectionStartTime:");
    Serial.println(sectionStartTime);
    Serial.print("julesLevel:");
    Serial.println(julesLevel);
    Serial.print("dataPointIndex:");
    Serial.println(dataPointIndex);
  }

  int sectionIndex = -1;
  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRIES; i++) {
    int16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + i * EEPROM_16LEVEL_LOG_ENTRY_SIZE;
    uint8_t startLogTime[4] = { 0 };  // Initialize data to zero
    int ret = eeprom.read(memAddress, startLogTime, sizeof(startLogTime));

    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(memAddress);
      return -1;  // Return -1 on read failure
    }             //end      if (ret < 0)

    uint32_t storedSectionStartTime;
    memcpy(&storedSectionStartTime, startLogTime,
           sizeof(storedSectionStartTime));

    if (storedSectionStartTime == sectionStartTime) {
      sectionIndex = i;
      break;  // Found the section
    }
  }  //end for

  //Serial.println("_______________________ eepromWrite16LevelSection STAGE 3  ________________________________________");


  if (sectionIndex < 0) {
    Serial.println("Section not found, cannot write data.");
    return -1;  // Return -1 if section is not found
  }


  //Serial.println("_______________________ eepromWrite16LevelSection STAGE 4  ________________________________________");

  // we use 4bit for 16-Jules levels
  int dataOffset =
    4 + dataPointIndex / 2;  // 4 bytes for section start time, then
                             // dataPointIndex byte for Jules level

  if (dataOffset >= EEPROM_16LEVEL_LOG_ENTRY_SIZE) {  //DEBUG: EEPROM_LOG_ENTRY_SIZE-4?
    Serial.println("Data point index out of bounds.");
    return -1;  // Return -1 if data point index is out of bounds
  }

  //Serial.println("_______________________ eepromWrite16LevelSection STAGE 5  ________________________________________");


  int readAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE + dataOffset;

  uint8_t julesData = 0;  // Initialize data to zero
  int ret = eeprom.read(readAddress, &julesData, sizeof(julesData));

  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(readAddress);
    return -1;  // Return -1 on read failure
  }

  // Set the Jules level
  julesData &= ~(0x0F << ((dataPointIndex % 2) * 4));  // Clear the bits for the current Jules level
  julesData |= (julesLevel & 0x0F) << ((dataPointIndex % 2) * 4);

  ret = eeprom.write(readAddress, &julesData, sizeof(julesData));
  if (ret < 0) {
    Serial.print("Failed to write EEPROM at address ");
    Serial.println(readAddress);
    return -1;  // Return -1 on write failure
  }

  if (true) {
    Serial.println("___________________________ readData[EEPROM_16LEVEL_LOG_ENTRY_SIZE]  ________________________________________");
    // print debug info
    uint8_t readData[EEPROM_16LEVEL_LOG_ENTRY_SIZE] = { 0 };  // Initialize data to zero

    ret = eeprom.read(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE,
                      readData, sizeof(readData));
    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE);
      return -1;  // Return -1 on read failure
    }

    Serial.print("EEPROM section data at index ");
    Serial.print(sectionIndex);
    Serial.println(": ");

    for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRY_SIZE; i++) {
      Serial.print(readData[i], HEX);
      Serial.print(" ");

      if (((i % 10) == 0) && (i > 0)) { Serial.println(); }
      //  if( ( (i%10)==0 ) && (i>0)  || (i==3)){  Serial.println(); }
    }
    Serial.println();
  }

  return 0;  // Success
}  //end    eepromWrite16LevelSection


/*
* Function
* 1) Find out which section in two section
*/
int LightLogicControl::eepromGetAccumulatedExposure_16Level(uint32_t sectionStartTime) {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section read.");
    return -1;  // Return -1 if EEPROM is not valid
  }

  if (sectionStartTime == 0) {
    return 0;
  }

  int sectionIndex = -1;
  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRIES; i++) {
    int16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + i * EEPROM_16LEVEL_LOG_ENTRY_SIZE;
    uint8_t startLogTime[4] = { 0 };  // Initialize data to zero
    int ret = eeprom.read(memAddress, startLogTime, sizeof(startLogTime));

    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(memAddress);
      return -1;  // Return -1 on read failure
    }

    uint32_t storedSectionStartTime;
    memcpy(&storedSectionStartTime, startLogTime, sizeof(storedSectionStartTime));

    if (storedSectionStartTime == sectionStartTime) {
      sectionIndex = i;
      break;  // Found the section
    }
  }

  if (sectionIndex < 0) {
    Serial.println("Section not found, cannot read data.");
    return -1;  // Return -1 if section is not found
  }

  uint8_t allDataInSection[EEPROM_16LEVEL_LOG_ENTRY_SIZE] = { 0 };  // Initialize data to zero
  int ret = eeprom.read(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE,
                        allDataInSection, sizeof(allDataInSection));

  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE);
    return -1;  // Return -1 on read failure
  }

  accumulatedExposure = 0;
  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRY_SIZE - 4; i++) {
    uint8_t julesData = allDataInSection[4 + i];
    for (int j = 0; j < 2; j++) {
      int julesLevel = (julesData >> (j * 4)) & 0x0F;  // Extract the Jules level for each 4 bits
      if (julesLevel > EXPOSURE_16LEVEL_0 && julesLevel <= EXPOSURE_16LEVEL_15) {
        accumulatedExposure += exposure16LevelToJules[julesLevel];
      }
    }
  }

  return accumulatedExposure;
}

/// @brief Get the section data for a specific 8-hour section from EEPROM (using 16-level data).
/// @param sectionStartTime The start time of the section to read.
/// @return The section data as a string for JSON, or an empty string on error.
String LightLogicControl::eepromGetSectionData_16Level(uint32_t sectionStartTime) {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section read.");
    return "null";  // Return null if EEPROM is not valid
  }

  int sectionIndex = -1;
  for (int i = 0; i < EEPROM_16LEVEL_LOG_ENTRIES; i++) {
    int16_t memAddress = EEPROM_16LEVEL_LOG_START_ADDRESS + i * EEPROM_16LEVEL_LOG_ENTRY_SIZE;
    uint8_t startLogTime[4] = { 0 };  // Initialize data to zero
    int ret = eeprom.read(memAddress, startLogTime, sizeof(startLogTime));
    if (ret < 0) {
      Serial.print("Failed to read EEPROM at address ");
      Serial.println(memAddress);
      return "null";  // Return null on read failure
    }
    // Check if the section start time matches
    uint32_t storedSectionStartTime;
    memcpy(&storedSectionStartTime, startLogTime, sizeof(storedSectionStartTime));
    if (storedSectionStartTime == sectionStartTime) {
      sectionIndex = i;
      break;  // Found the section
    }
  }

  if (sectionIndex < 0) {
    Serial.println("Section not found, cannot read data.");
    return "null";  // Return null if section is not found
  }

  uint8_t allDataInSection[EEPROM_16LEVEL_LOG_ENTRY_SIZE] = { 0 };  // Initialize data to zero
  int ret = eeprom.read(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE,
                        allDataInSection, sizeof(allDataInSection));
  if (ret < 0) {
    Serial.print("Failed to read EEPROM at address ");
    Serial.println(EEPROM_16LEVEL_LOG_START_ADDRESS + sectionIndex * EEPROM_16LEVEL_LOG_ENTRY_SIZE);
    return "null";  // Return null on read failure
  }

  String data = "";
  data.reserve(700); // Reserve memory for 120 entries (approx 120 * 5 chars + headers)
  data = "{\"sectionStartTime\":" + String(sectionStartTime) + ",\"data\":[";
  // The old format had 120 entries (1 per 4 minutes).
  // The new format has 240 bytes (1 per 2 minutes, with two 4-bit values per byte).
  // To match the old 120-entry format, we take every second byte.
  for (int i = 4; i < EEPROM_16LEVEL_LOG_ENTRY_SIZE; i += 2) {
    uint8_t julesData = allDataInSection[i];
    // convert julesData to padded hex string
    char hexHigh = (julesData >> 4) + '0';
    if (hexHigh > '9') {
      hexHigh += 7;  // Convert to A-F
    }
    char hexLow = (julesData & 0x0F) + '0';
    if (hexLow > '9') {
      hexLow += 7;  // Convert to A-F
    }
    data += "\"";
    data += hexHigh;
    data += hexLow;
    data += "\"";
    if (i < EEPROM_16LEVEL_LOG_ENTRY_SIZE - 2) {
      data += ",";
    }
  }
  data += "]}";
  return data;
}


int LightLogicControl::eepromGetNewestEntryLightOnData() {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping section index read.");
    return 0;  // Return 0 if EEPROM is not valid
  }
  uint8_t localCopy[EEPROM_LIGHT_LOG_DATA_SIZE * EEPROM_LIGHT_LOG_ENTRIES] = { 0 };  // Initialize data to zero
  eeprom.read(EEPROM_LIGHT_LOG_START_ADDRESS, localCopy, sizeof(localCopy));
  int newestEntryIndex = 0;
  for (int i = 0; i < EEPROM_LIGHT_LOG_ENTRIES; i++) {
    uint32_t lightOnTimeThisEntry = *(uint32_t*)(localCopy + i * EEPROM_LIGHT_LOG_DATA_SIZE);
    uint32_t lightOnTimeNextEntry = *(uint32_t*)(localCopy + ((i + 1) % EEPROM_LIGHT_LOG_ENTRIES) * EEPROM_LIGHT_LOG_DATA_SIZE);
    if ((lightOnTimeThisEntry + 1) == lightOnTimeNextEntry) {
      newestEntryIndex = ((i + 1) % EEPROM_LIGHT_LOG_ENTRIES);  // Found the newest entry
    } else {
      break;
    }
  }
  return newestEntryIndex;
}
/// @brief Read the light on data from EEPROM.

uint32_t LightLogicControl::eepromReadLightOnData() {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping light on data read.");
    return 0;  // Return 0 if EEPROM is not valid
  }

  int newestEntryIndex = eepromGetNewestEntryLightOnData();
  uint8_t lightOnData[4] = { 0 };  // Initialize data to zero
  eeprom.read(EEPROM_LIGHT_LOG_START_ADDRESS + newestEntryIndex * EEPROM_LIGHT_LOG_DATA_SIZE, lightOnData, sizeof(lightOnData));
  uint32_t lightOnTimeForLoggingInIntervalUnit;
  memcpy(&lightOnTimeForLoggingInIntervalUnit, lightOnData, sizeof(lightOnTimeForLoggingInIntervalUnit));
  return lightOnTimeForLoggingInIntervalUnit;  // Return the light on time as uint32_t
}

int LightLogicControl::eepromWriteLightOnData(uint32_t lightOnTimeForLoggingInIntervalUnit) {
  if (!eepromValid) {
    Serial.println("EEPROM is not valid, skipping light on data write.");
    return -1;  // Return -1 if EEPROM is not valid
  }

  int newestEntryIndex = eepromGetNewestEntryLightOnData();
  newestEntryIndex = (newestEntryIndex + 1) % EEPROM_LIGHT_LOG_ENTRIES;  // Move to the next entry
  Serial.print("Writing light on data to entry index: ");
  Serial.println(newestEntryIndex);

  uint8_t lightOnData[4];
  memcpy(lightOnData, &lightOnTimeForLoggingInIntervalUnit, sizeof(lightOnTimeForLoggingInIntervalUnit));
  int ret = eeprom.write(EEPROM_LIGHT_LOG_START_ADDRESS + newestEntryIndex * EEPROM_LIGHT_LOG_DATA_SIZE, lightOnData, sizeof(lightOnData));
  return ret;  // Success
}

/*
* Function
* 1) adjust  from utc rtc time into local time, reflecting in  "scheduleIndex" 
* 2)    find out scheduleInEffect at current time, 
* 2.1)  slot'0': turn off UV-lamp at '0'; slot'1' just routinely check accumlated enerey >161mJ     
* 3)  return    scheduleInEffect
*/
bool LightLogicControl::getscheduleInEffect(uint8_t* scheduleData, bool fprint) {

  DateTime currentUTCTime = rtc.outputNowDateTime();
  uint8_t dayOfTheWeek = currentUTCTime.dayOfTheWeek();  // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
  uint8_t hour = currentUTCTime.hour();
  uint8_t min = currentUTCTime.minute();

  //each slot is 30 minutes, so there are 48 slots in a day
  int scheduleIndex = dayOfTheWeek * 48 + (hour * 2) + (min / 30);

  //-------------------------- start of process shifting --------------------------------------
  //int offset;
  int timeZoneOffsetMinutes;
  int timeZoneOffsetHour;
  int shiftindex;
  timeZoneOffsetMinutes = payload_manager.payload_deviceConfig_sub.timeZoneOffsetMinutes;
  timeZoneOffsetHour = timeZoneOffsetMinutes / 60;  //eg    480/60 = 8hour

  shiftindex = scheduleIndexShift[abs(timeZoneOffsetHour)];
  //shiftindex =  scheduleIndexShift[timeZoneOffsetHour];

  if (fprint) {
    Serial.print("Before process,scheduleIndex:");
    Serial.println(scheduleIndex);  //eg 2,4,6...16(480min)..... 28
    Serial.print("timeZoneOffsetMinutes:");
    Serial.println(timeZoneOffsetMinutes);
  }

  if (timeZoneOffsetMinutes >= 0) {
    // case >=0
    Serial.println("positive");

    scheduleIndex = scheduleIndex + shiftindex;

    if (scheduleIndex < 336) {
      //dummy
    } else {
      scheduleIndex = scheduleIndex - 336;
    }


  } else {
    Serial.println("negative");
    scheduleIndex = scheduleIndex - shiftindex;

    if (scheduleIndex >= 0 && scheduleIndex < 336) {
      // dummy
    } else {
      scheduleIndex = 336 + scheduleIndex;
      //scheduleIndex = 336 - scheduleIndex;
    }


  }  //end else

  if (fprint) {
    Serial.print("After process,scheduleIndex:");
    Serial.println(scheduleIndex);
  }

  //-------------------------- end of process shifting --------------------------------------

  int byteIndex = scheduleIndex / 8;
  int bitIndex = scheduleIndex % 8;
  bool scheduleInEffect = false;

  if (byteIndex >= 0 && byteIndex < 42) {  // 42 bytes for 7 days * 48 slots / 8 bits
    scheduleInEffect = (scheduleData[byteIndex] & (1 << bitIndex)) != 0;
  }

  if (fprint) {
    Serial.print("Current UTC time:DayOfWk:");
    Serial.print(dayOfTheWeek);
    Serial.print("\t H:");
    Serial.print(hour);
    Serial.print("\t M:");
    Serial.print(min);
    Serial.print("\t scheduleInEffect:");
    Serial.print(scheduleInEffect);
    //        Serial.print(scheduleInEffect ? "EN" : "DIS");
    Serial.print("\t byteIndex:");
    Serial.print(byteIndex);
    Serial.print("\t bitIndex:");
    Serial.println(bitIndex);
    Serial.print("\t scheduleData[byteIndex]:");
    Serial.print(scheduleData[byteIndex], HEX);
    Serial.println();
  }  //end    if(fprint)

  if (scheduleInEffect) {
    if (fprint) {
      Serial.println("Schedule as ENABLED, do smart");
    }
  } else {
    if (fprint) {
      Serial.println("Schedule as DISABLED, turn off light");
    }
    setLightState(false);
    //return 0; // Light turned off
  }  //end else

  lastScheduleInEffect = scheduleInEffect; 

  return scheduleInEffect;

}  //end      int LightLogicControl::bool getscheduleInEffect

bool LightLogicControl::processSmartMode(bool scheduleEnabled, uint8_t* scheduleData) {
  bool fprint = true;

  if (fprint) {
    //     if(true){      // only display scheduleData[i]
    Serial.println("INT_UV_MODE_SMART________________");
    Serial.println("processSmartMode(bool scheduleEnabled, uint8_t* scheduleData): ");
    for (int i = 0; i < 42; i++) {

      if ((i % 6 == 0) && (i != 0))
      //  if(   (i%8==0)  && (i!=0)   )
      { Serial.println(""); }
      Serial.print(scheduleData[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }  //end if(true)

  if (scheduleEnabled) {
    bool scheduleInEffect;

    scheduleInEffect = getscheduleInEffect(scheduleData, true);
    if (!scheduleInEffect) { return 0; }

  }  //end    if (scheduleEnabled)

  if (true) {
    Serial.print("accumulatedExposureThreshold:");
    Serial.println(accumulatedExposureThreshold);
    Serial.print("accumulatedExposure:");
    Serial.println(accumulatedExposure);
  }

  if (accumulatedExposure >= accumulatedExposureThreshold) {
    Serial.println(
      "Accumulated exposure reached threshold, turning off light.");
    setLightState(false);

    lampOnRecord[LAMP_ON_RECORD_HIT8HOUR].lampOnTimeStamps_start = lampOnRecordCache.lampOnTimeStamps_start;
    lampOnRecord[LAMP_ON_RECORD_HIT8HOUR].lampOnTimeStamps_end = lampOnRecordCache.lampOnTimeStamps_end;
    lampOnRecord[LAMP_ON_RECORD_HIT8HOUR].length = lampOnRecordCache.length;
    lampOnRecord[LAMP_ON_RECORD_HIT8HOUR].reason = "Hit 8hr limit";

    setUiLedState(UI_LED_WHITE, UI_LED_BLINK);

    return 0;  // Light turned off
  } else {
    Serial.println("Accumulated exposure is below threshold, turning on light.");
    setLightState(true);
    return 1;  // Light turned on
  }

}  //end  void processSmartMode


/// @brief Process the light control logic based on the current mode and conditions.
/// @return 0 if the light is turned off, 1 if the light is turned on.
int LightLogicControl::processLightControl(bool scheduleEnabled, uint8_t* scheduleData) {


  switch (uvLampMode) {

    case INT_UV_MODE_SMART: 
      {
        processSmartMode(scheduleEnabled, scheduleData);
      }
      break;  //end         case INT_UV_MODE_SMART

    case INT_UV_MODE_MANUAL:
      {
        Serial.println("INT_UV_MODE_MANUAL________________");

        if (averagedCalculatedMinimalDistance < 300) {
          Serial.println("Minimal distance is less than 30cm, turning off the light");
          setLightState(false);
        } else {
          setLightState(true);
        }
        return 1;  // Light turned on
      }
    case INT_UV_MODE_UNLIMITED:
      {
        Serial.println("INT_UV_MODE_UNLIMITED_____________________");

        Serial.println("UV Lamp mode: Unlimited");
        setLightState(true);
      }
      return 1;  // Light turned on
    case INT_UV_MODE_OCCUPIED_ROOM:
      {
        Serial.println("INT_UV_MODE_OCCUPIED_ROOM_____________________");

        int radar1Count = radar1.radarObjectCount;
        int radar2Count = radar2.radarObjectCount;
        int thermalCount = thermalDetectionCount;

        Serial.print("radar1Count:");
        Serial.print(radar1Count);
        Serial.print("\t");
        Serial.print("radar2Count:");
        Serial.print(radar2Count);
        Serial.print("\t");
        Serial.print("thermalCount:");
        Serial.print(thermalCount);
        Serial.println("\t");

        int setPersonCountThreshold = payload_manager.payload_command_sub.setPersonCountThreshold;

        if (setPersonCountThreshold > 3) {
          setPersonCountThreshold = 3;
        }

        if ((radar1Count > 0 || radar2Count > 0 || thermalCount > 0) && ((radar1Count < setPersonCountThreshold) && (radar2Count < setPersonCountThreshold) && (thermalCount < setPersonCountThreshold)))
        {
          Serial.println("Occupied room detected, turning on light.");
          setLightState(true);
          return 1;  // Light turned on
        } else {
          Serial.println("No occupancy detected, turning off light.");
          setLightState(false);
          return 0;  // Light turned off
        }

      }
      break;
    case INT_UV_MODE_EMPTY_ROOM:
      {
        Serial.println("INT_UV_MODE_EMPTY_ROOM_____________________");
        int radar1Count = radar1.radarObjectCount;
        int radar2Count = radar2.radarObjectCount;
        int thermalCount = thermalDetectionCount;

        Serial.print("radar1Count:");
        Serial.print(radar1Count);
        Serial.print("\t");
        Serial.print("radar2Count:");
        Serial.print(radar2Count);
        Serial.print("\t");
        Serial.print("thermalCount:");
        Serial.print(thermalCount);
        Serial.println("\t");

        if (radar1Count > 0 || radar2Count > 0 || thermalCount > 0) {
          Serial.println("Occupied room detected, turning off light.");
          setLightState(false);
          return 0;  // Light turned off
        } else {
          Serial.println("No occupancy detected, turning on light.");
          setLightState(true);
          return 1;  // Light turned on
        }
      }
      break;
    case INT_UV_MODE_OFF:
      {
        Serial.println("INT_UV_MODE_OFF_____________________");
        Serial.println("UV Lamp mode: Off");
        setLightState(false);

        lampOnRecord[LAMP_ON_RECORD_MANUAL].lampOnTimeStamps_start = lampOnRecordCache.lampOnTimeStamps_start;
        lampOnRecord[LAMP_ON_RECORD_MANUAL].lampOnTimeStamps_end = lampOnRecordCache.lampOnTimeStamps_end;
        lampOnRecord[LAMP_ON_RECORD_MANUAL].length = lampOnRecordCache.length;
        lampOnRecord[LAMP_ON_RECORD_MANUAL].reason = "Manual mode shut off";

        return 0;  // Light turned off
      }
      break;

    default:
      {
        Serial.println("INT_UV_MODE_default_____________________");
        Serial.println("UV Lamp mode: Off");
        setLightState(false);
        return 0;  // Light turned off
      }
      break;
  }  //end switch

  return lightState;

}  //end    int LightLogicControl::processLightControl

const char* LightLogicControl::getCompiledTimeStr() {
  return compiledTimeStr;
}
