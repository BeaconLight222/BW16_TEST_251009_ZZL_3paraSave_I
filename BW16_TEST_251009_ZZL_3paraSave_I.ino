//History"
// ZZL_3paraSave_H   fix scheduleEnable false lossing after device-reset                                  working
// ZZL_3paraSave_G   double checked lampstatus emergency turn off to be false twice, and version 2019.01.09 working
// ZZL_3paraSave_F   (r1+r1)/2                                                                            working
// ZZL_3paraSave_E  implemnent diagnosic level 254                                                        working
// ZZL_3paraSave_D  2)flick, 3) 8hr reset acc  4)updated   4) resetaccumulatedexposureuint = true         working
//      acculated energy too fast                                                                         pending
// ZZL_3paraSave_C            product supplied by24V    make-up function (TACT,Maunal and Schedule mode)  working
// ZZL_3paraSave_B            bare pcb                  make-up function                                  working
// ZZL save uvLampMode, ie detectionMode in eeprom                    working
//     save                scheduleEnable in eeprom                   working
//     save                schedule table [42]byte in eeprom          working
// ZZK schedule HK and USA                                            working
// ZZJ schedule                                                                         pending
// ZZI in smart mode, emerergy turn-off, restored immediately after removal of hand     working
// ZZH 1min turn off 1s bug                                           Fixed
//     Schedule slot '1' turn on, turn off by hand is ALWAYS OFF until next minute, it is not immedately turn on after hand remove
// ZZF schedule 0,1,0,1                                               working
//     test schedule off with detectionMode =SMART and scheduleEnable=true and ,0
//     test schedule on  with detectionMode =SMART and scheduleEnable=true and ,1
//      release to BEACON  on Jan-12-2026
// ZZE released to Tad                                                working
// ZZD implement fprint in callback                                   working
// ZZC wifi reset check                                               working
// ZZB check 8hr over 161000                                          working
// ZZA updated Tad's telemetry publish example message                working
// Beacon_Aviton_20251222                                             copy from ZZ
// ZZ
// ZY  placing lightControl.thermalSensorValid/min eliminate reset after 60s      working
// ZX  16Level read and write in eeprom                               working
// ZW  pause at SECTION 3 , section not found                         pending
// ZV  schedule format follow Tad's format at stringTad               working
// ZU  lastErrorTimestamp  in telemetry/pub                           working
// ZT  red-blue led                                                   not working
// ZS  wifiReset in node.js                                           working
// ZR  telemetry/pub, uptimeSecond done, start of errorStatus         working
// ZQ  variable 161,479mJ sending from command/sub                    working
// ZP  excel payload is finished                                      working
// ZO  up to lastCommandStatus                                        working
// ZN  record Manual Off End-time updating    ,both fSetStartTime     working
//     theory is only update on and off one time by using fSetStartTime
//     at on-time, update both on and off time together,even though off-time is not useful
//     at off-time, update only off time
// ZM  light restore very slowly like 20sec                           pending
// ZL in setLightState, 'turn off any reason'                         working
// ZK too many setlightSet                                            pending
// ZJ 946,684,800s  at first turn on                                  pending
// ZI telemetry schedule                                              working
// ZH eepromWriteSchdule                                              working
// ZG      kept it as ref only                                        not working
// ZF: start saving schedule at eeprom location 0x200                 working
// ZE: telemetry/pub    schedule to be defined                        pending
// ZD: telemetry/pub initialize                                       working
// ZC: publish define   telemetryIntervalSeconds                      working
// ZB：diagnosticLevel 2 and 9                                        working
// Z: create swich case for diagnosticsLevel                          working
// Y: firmwareAutoUpdate                                              working
// X: telemetryInterval Seconds                                       working
// W: 60sec time out working                                          working
// V:                                                                 ignored
// U: after reset UV-lamp follow last lamp status                     working
// T: up to timeZoneOffsetMinutes completed                           working
// S： complete command/sub                                           working
// R: Node.js receive the correct rtcTime with 60sec increment /min   working
// Q: can enter 60sec block                                           working
// P: only print_now()  is working properly in last{}                 wprking
// N:  muteAlarms                                                     working
// M:beacon_pcb replace ioport                                        working
// L: muteAlarm cannot turn off Buzzer                                pending
// K:
// J: fix all 4 pints:distance, temperature and eliminate beep at power-on    working
// I: Node.js   up to 2 UMLIMITED                                     working
//  check -50mm offset; no object return 0; -2xxxxx min distance      working
// H: white LED turn on and off                                       working
// F: build beacon_pcb class                                          working
// E: erase ssid/password with the blinking red-blue LED              working
// D: Ballast power switch was implemented                            working
// C:   WhiteLED, ballaast and flag in phrase                         working
//       or >40C turn off fan                                         working
// B:   the beginning time of program is same as actual time          working
// BW16_TEST_25100_A:     beep sound drive in while loop              working
// copy from deqing 2025-10-09 version

/// @file BW16_TEST_20250529_Light_Logic_log.ino
/// @brief Main file compiled with 3.1.9

#include "ToolsArray.h"

#include "EnergyEngineer.h"
#include "uv_lamp_parameter.h"
#include "src/Simple_misc_sensor/beacon_pcb.h"
#include "src/ButtonHandling/ButtonHandler.h"
// compiled with 3.1.9
#include <WiFi.h>
#include <WiFiClient.h>

#include "esp32Provision.h"
#include "AwsMqtt.h"
#include "NonvolatileDataManager.h"
#include <Wire.h>
#include "WDT.h"
#include "LightLogicControl.h"

#include "payload.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_api.h"

#ifdef __cplusplus
}
#endif

uint32_t program_StartUpTime = 0;

unsigned long previousMillis = 0;  // Stores the last time the event occurred

unsigned long interval = 1000;

static bool fToggle;

EnergyEngineer beaconEnergyEngineer;


extern PAYLOAD_MANAGER payload_manager;
LightLogicControl lightControl;

extern volatile int gTimer0Counter;

uint32_t lastWifiConnectedTime = 0;
uint32_t lastWifiTryConnectTime = 0;
bool bleAdvertisingForWifiConfig = false;
bool wifiWasConnected = false;
uint32_t inProgress8hourSectionStartTimePrevious = 0;

#define NO_CONNECTION_RESTART_BLE_CONFIG_TIME 15000

ESP32Provision *esp32Provision = nullptr;

uint32_t lastEsp32ProvisionActivityMillis = 0;

BEACON_PCB beacon_pcb;

#define UNDEFINED 255
#define UV_LAMP_ON 1
#define UV_LAMP_OFF 0


//#define BEEPER_ALWAYS_ON
#define BALLAST_CONTROL_SIGNAL AMB_D12
#define LOGIC_HIGH 1
#define LOGIC_LOW 0
#define MODE_OUTPUT 0x00
#define MODE_INPUT 0x01


AwsMqtt awsMqtt;
WDT wdt;

//Start of local function declaration
void processAwsMqtt_print(bool fprint);

void record_lastErrorTimestamp_print(bool fprint);
void action_diagnosticlevel_print(bool wifiConnected, uint8_t diagnosticlevel, bool fprint);
uint8_t get_errorStatus();
void record_lastErrorTimestamp();
void action_diagnosticlevel(bool wifiConnected, uint8_t diagnosticlevel);
void resetWifi();
void print_second();
void print_now();
//end of local function declaration


uint8_t get_errorStatus() {
  uint8_t error = ERROR_UNDEFINED;

  if (payload_manager.payload_command_sub.factoryReset) {
    error = ERROR_SYSRESET_FACTORY;
  } else if (payload_manager.payload_command_sub.wifiReset) {
    error = ERROR_SYSRESET_WIFI;
  } else if (!(lightControl.radar1Valid || lightControl.radar2Valid)) {
    error = ERROR_2XRADAR_INVALID;
  } else if (!lightControl.temperatureSensorValid) {
    error = ERROR_TEMPERATURE_INVALID;
  } else if (!lightControl.eepromValid) {
    error = ERROR_EEPROM_INVALID;
  } else if (!lightControl.rtcValid) {
    error = ERROR_RTC_INVALID;
  } else if (!lightControl.thermalSensorValid) {
    error = ERROR_THERMAL_INVALID;
  } else if (!lightControl.radar2Valid) {
    error = ERROR_RADAR2_INVALID;
  } else if (!lightControl.radar1Valid) {
    error = ERROR_RADAR1_INVALID;
  } else {
    error = ERROR_NO;
  }

  lightControl.errorStatus = error;
  return lightControl.errorStatus;

}  //end      uint8_t   get_errorStatus()

void action_diagnosticlevel_print(bool wifiConnected, uint8_t diagnosticlevel, bool fprint) {
  uint8_t status = 0x00;

  lightControl.eeprom.read(EEPROM_STATUS_ADDRESS, &status, sizeof(status));

  if (fprint) {
    Serial.println("____________________ action_diagnosticlevel __________________________");
    Serial.print("diagnosticlevel:");
    Serial.println(diagnosticlevel);
    Serial.print("status:");
    Serial.println(status);
    Serial.print("wifiReset:");
    Serial.println(payload_manager.payload_command_sub.wifiReset);
  }

  if (wifiConnected) {
    switch (diagnosticlevel) {
      case 0:

        get_errorStatus();

        if (fprint) {
          Serial.println("__________________________  diagnosticlevel 0  _____________________________________");
        }

        //lightControl.eeprom.read(  EEPROM_STATUS_ADDRESS, &status, sizeof(status)   );
        //Serial.print("status:");Serial.println(    status      );
        //Serial.print("wifiReset:");Serial.println(payload_manager.payload_command_sub.wifiReset);

        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_0().c_str());
        break;

      case 1:
        if (fprint) {
          Serial.println("__________________________  diagnosticlevel 1  _____________________________________");
        }
        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_1().c_str());
        break;

      case 2:
        if (fprint) {
          Serial.println("__________________________  diagnosticlevel 2  _____________________________________");
        }
        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_2().c_str());
        break;


      case 9:
        break;

      case 254:
        if (fprint) {
          Serial.println("__________________________  diagnosticlevel 254 _____________________________________");
        }
        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_254().c_str());
        break;



      case 255:
        if (fprint) {
          Serial.println("__________________________  diagnosticlevel 255 _____________________________________");
        }
        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_255().c_str());
        break;

      default:
        //dummy
        break;

    }  //end    switch
  }    //end    if (wifiConnected)
}  //end    void action_diagnosticlevel_print




void action_diagnosticlevel(bool wifiConnected, uint8_t diagnosticlevel) {
  uint8_t status = 0x00;

  Serial.println("____________________ action_diagnosticlevel __________________________");
  Serial.print("diagnosticlevel:");
  Serial.println(diagnosticlevel);

  lightControl.eeprom.read(EEPROM_STATUS_ADDRESS, &status, sizeof(status));
  Serial.print("status:");
  Serial.println(status);
  Serial.print("wifiReset:");
  Serial.println(payload_manager.payload_command_sub.wifiReset);


  if (wifiConnected) {
    switch (diagnosticlevel) {
      case 0:

        get_errorStatus();

        Serial.println("__________________________  diagnosticlevel 0  _____________________________________");

        lightControl.eeprom.read(EEPROM_STATUS_ADDRESS, &status, sizeof(status));
        Serial.print("status:");
        Serial.println(status);
        Serial.print("wifiReset:");
        Serial.println(payload_manager.payload_command_sub.wifiReset);

        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_0().c_str());

        break;

      case 1:
        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_1().c_str());
        break;

      case 2:
        awsMqtt.logSensorToServer((char *)lightControl.getDiagnosticsLevelJson_2().c_str());
        break;

      case 9:
      default:

        break;
    }  //end switch

  }  //end if (wifiConnected)

}  //end    void action_diagnosticlevel


void record_lastErrorTimestamp_print(bool fprint) {
  DateTime dateTime = lightControl.rtc.outputNowDateTime();
  lightControl.currentRtcTime = dateTime;

  uint32_t unix_time = lightControl.currentRtcTime.unixtime();

  if (fprint) {
    Serial.print("Before eeprom saved, unix_time:");
    Serial.println(unix_time);
  }

  lightControl.setLongWordEeprom(EEPROM_LASTERROR_TIMESTAMP_4, unix_time);

  if (fprint) {
    unix_time = 0;
    unix_time = lightControl.getLongWordEeprom(EEPROM_LASTERROR_TIMESTAMP_4);
    Serial.print("After eeprom saved, unix_time:");
    Serial.println(unix_time);
  }

}  //end    void record_lastErrorTimestamp_print


void record_lastErrorTimestamp() {
  DateTime dateTime = lightControl.rtc.outputNowDateTime();
  lightControl.currentRtcTime = dateTime;

  uint32_t unix_time = lightControl.currentRtcTime.unixtime();
  Serial.print("Before unix_time:");
  Serial.println(unix_time);
  lightControl.setLongWordEeprom(EEPROM_LASTERROR_TIMESTAMP_4, unix_time);
  unix_time = 0;
  unix_time = lightControl.getLongWordEeprom(EEPROM_LASTERROR_TIMESTAMP_4);
  Serial.print("After unix_time:");
  Serial.println(unix_time);
}  //end

void resetWifi() {
  Serial.println("_____________________________  resetWifi  _________________________________________");

  record_lastErrorTimestamp_print(false);

  uint8_t status;
  lightControl.setBitEeprom(EEPROM_STATUS_ADDRESS, EEPROM_STATUS_BIT_POSITION_WIFI);
  lightControl.eeprom.read(EEPROM_STATUS_ADDRESS, &status, sizeof(status));
  Serial.print("resetWifi::status:");
  Serial.println(status, HEX);

  NonvolatileDataManager::eraseSsidPasswordFromFlash();
  lightControl.setUiLedState(UI_LED_BLUE, UI_LED_BLINK);
  lightControl.setUiLedState(UI_LED_RED, UI_LED_BLINK);
  delay(1000);

  sys_reset();
  while (1)
    ;

}  //end void




void processAwsMqtt_print(bool fprint) {
  if (payload_manager.list_newModeCommand.size() > 0) {
    char *ssid = getWifiSsidAfterFetch();
    bool wifiConnected = (ssid[0] != 0);

    String mode;

    if (fprint) {
      Serial.println("process aws mqtt:");
    }

    for (unsigned int n = 0; n < payload_manager.list_newModeCommand.size(); n++) {
      if (fprint) {
        Serial.print("newModeCommand:");
        Serial.print(n);
        Serial.print("\t");
        Serial.println(payload_manager.list_newModeCommand[n]);
      }
      mode = payload_manager.list_newModeCommand[n];

      if (mode == "UV_MODE_SMART") {
        lightControl.uvLampMode = INT_UV_MODE_SMART;
      }  //end if(mode=="UV_MODE_SMART")

      if (mode == "UV_MODE_MANUAL") {
        lightControl.uvLampMode = INT_UV_MODE_MANUAL;
      } else if (mode == "UV_MODE_OFF") {
        lightControl.uvLampMode = INT_UV_MODE_OFF;
        //lightControl.uvLampMode = UV_MODE_OFF;
      }

      if (mode == "UV_MODE_UNLIMITED") {
        lightControl.uvLampMode = INT_UV_MODE_UNLIMITED;
      }  //end    if(mode=="UV_MODE_UNLIMITED")

      if (mode == "UV_MODE_OCCUPIED_ROOM") {
        lightControl.uvLampMode = INT_UV_MODE_OCCUPIED_ROOM;
        // lightControl.uvLampMode = UV_MODE_OCCUPIED_ROOM;
      }

      if (mode == "UV_MODE_EMPTY_ROOM") {
        lightControl.uvLampMode = INT_UV_MODE_EMPTY_ROOM;
      }


      if (mode == "JOB_RESET_ACCUMULATED_EXPOSURE_LIMIT") {
        Serial.println("mode == JOB_RESET_ACCUMULATED_EXPOSURE_LIMIT_________________________________");
        if (payload_manager.payload_command_sub.resetAccumulatedExposureLimit) {

          lightControl.eepromRead16LevelSection(lightControl.inProgress8hourSectionStartTime);

          uint8_t index = lightControl.findWhichLevelSection(lightControl.inProgress8hourSectionStartTime);
          Serial.print("index:");
          Serial.println(index);


          lightControl.eepromClear_16LevelSection(0, lightControl.inProgress8hourSectionStartTime);
          lightControl.eepromClear_16LevelSection(1, lightControl.inProgress8hourSectionStartTime);
          //lightControl.eepromClear_16LevelSection( index, lightControl.inProgress8hourSectionStartTime  );

          lightControl.eepromGetAccumulatedExposure_16Level(lightControl.inProgress8hourSectionStartTime);
          lightControl.eepromRead16LevelSection(lightControl.inProgress8hourSectionStartTime);

        } else {
          //dummmy
        }

      }  //end


      if (mode == "RESET_WIFI") {
        if (fprint) {
          Serial.print("awsMqtt.wifiReset");
          Serial.println(awsMqtt.wifiReset);
        }

        sendLoggingAndTelemetryDataToServer();
        payload_manager.list_newModeCommand.clear();
        resetWifi();

      }  //end    if (mode == "RESET_WIFI")

      if (mode == "JOB_MUTE_ALARMS") {
        unsigned char data = payload_manager.payload_command_sub.muteAlarms ? 0x55 : 0xAA;
        lightControl.eeprom.write(EEPROM_MUTEALARMS_ADDRESS, &data, sizeof(data));
      }

      if (mode == "JOB_DEFAULT_DETECTION") {
        unsigned char data = payload_manager.payload_deviceConfig_sub.defaultDetectionMode;
        lightControl.eeprom.write(EEPROM_DEFAULT_DETECTION_MODE_ADDRESS, &data, sizeof(data));

        if (fprint) {
          Serial.print("EEPROM_DEFAULT_DETECTION_MODE_ADDRESS:");
          Serial.println(data);
        }
      }  //end  if( mode == "JOB_DEFAULT_DETECTION" )

      if (mode == "JOB_LAMP_STARTUP_STATE") {
        unsigned char data = UNDEFINED;
        String lampStartupState = payload_manager.payload_deviceConfig_sub.lampStartupState;

        if (lampStartupState == "lastKnown") {
          data = JOB_LAMP_STARTUP_LASTKNOWN;
        } else if (lampStartupState == "On") {
          data = JOB_LAMP_STARTUP_ON;
        } else if (lampStartupState == "Off") {
          data = JOB_LAMP_STARTUP_OFF;
        } else {
          //dummy
        }
        lightControl.eeprom.write(EEPROM_LAMP_STARTUP_STATE_ADDRESS, &data, sizeof(data));

        if (fprint) {
          data = UNDEFINED;
          lightControl.eeprom.read(EEPROM_LAMP_STARTUP_STATE_ADDRESS, &data, sizeof(data));
          Serial.print("EEPROM_LAMP_STARTUP:");
          Serial.println(data);
        }  //end  if(fprint)

      }  //end      if( mode == "JOB_LAMP_STARTUP_STATE" )


      if (mode == "JOB_SCHEDULE") {
        lightControl.eepromWriteSchedule();


        lightControl.getScheduleStrTad_print(false);
      }  //end  if( mode == "JOB_SCHEDULE" )



      if (mode == "JOB_THRES_ACCUMULATED_EXPOSURE") {
        lightControl.accumulatedExposureThreshold = payload_manager.payload_command_sub.thresAccumulatedExposure;
      }

      if (mode == "JOB_TIME_ZONE_OFFSET_MINUTES") {

        int timeZoneOffsetMinutes = payload_manager.payload_deviceConfig_sub.timeZoneOffsetMinutes;
        uint8_t byteData[4] = { 0 };
        memcpy(byteData, &timeZoneOffsetMinutes, sizeof(timeZoneOffsetMinutes));
        lightControl.eeprom.write(EEPROM_TIME_ZONE_OFFSET_MINUTES, byteData, sizeof(byteData));

        if (true) {  // vertify timeZoneOffsetMinutes saved in eeprom
          uint8_t checkByte[4] = { 0 };
          int checkingTime = 0;

          if (false) {
            Serial.print("Before checkingTime____________________________________:");
            Serial.println(checkingTime);
          }

          lightControl.eeprom.read(EEPROM_TIME_ZONE_OFFSET_MINUTES, checkByte, sizeof(checkByte));
          memcpy(&checkingTime, checkByte, sizeof(checkByte));

          if (false) {
            Serial.print("After checkingTime:");
            Serial.println(checkingTime);
          }

        }  //end if

      }  //end   if(  mode == "JOB_TIME_ZONE_OFFSET_MINUTES")


      if (mode == "JOB_SCHEDULE_ENABLE") {
        bool scheduleEnabled = awsMqtt.scheduleEnabled;

        if (scheduleEnabled) {
          lightControl.setBitEeprom(EEPROM_STATUS_ADDRESS, EEPROM_STATUS_BIT_POSITION_SCHEDULE_EN);
        } else {
          lightControl.clrBitEeprom(EEPROM_STATUS_ADDRESS, EEPROM_STATUS_BIT_POSITION_SCHEDULE_EN);
        }


        if (true) {  //vertify  scheduleEnabled in eeprom
          lightControl.getScheduleEnableEeprom();
        }  //end      if(true){ //vertify  scheduleEnabled in eeprom

      }  //end      if( mode == "JOB_SCHEDULE_ENABLE")

    }  //end  for(int n=0; n< payload_manager.list_newModeCommand.size()
    Serial.print("processAwsMqtt_print::lightControl.uvLampMode_____________:");
    Serial.println(lightControl.uvLampMode);
    lightControl.setDetectionModeEeprom();

    payload_manager.list_newModeCommand.clear();

    if (fprint) {
      Serial.print("Set UV lamp mode to: ");
      Serial.println(lightControl.uvLampMode);
    }

    lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);

    if (wifiConnected) {
      sendLoggingAndTelemetryDataToServer();
    }

  }  //end if ( payload_manager.list_newModeCommand.size()>0 )

}  //end  void processAwsMqtt_print

void print_second() {
  uint8_t u8_temp;
  u8_temp = lightControl.rtc.read_second();
  Serial.print("second:");
  Serial.println(u8_temp, HEX);
}

void print_now() {
  while (!lightControl.rtc.begin()) {
    Serial.println("PCF85363A init error !");
    delay(1000);
  }
  lightControl.currentRtcTime = lightControl.rtc.now();
  Serial.print("Current RTC time: ");
  Serial.print(lightControl.currentRtcTime.timestamp());
  Serial.print("  unix time: ");
  Serial.println(lightControl.currentRtcTime.unixtime());
}  //end    void print_now()


/// @brief setup is an Arduino function that is called once at the start of the program. \n
/// In this project, it checks bootloader, initializes the sensors \n
/// And then Wifi and BLE
void setup() {
  //send SCK clocks on PA25 to unfreeze the I2C
  pinMode(PA25, OUTPUT);
  pinMode(PA25, INPUT);
  pinMode(PB3, OUTPUT);
  pinMode(PA14, OUTPUT);
  pinMode(PA13, OUTPUT);

  GPIO_WriteBit(_PB_3, 0);   // Turn off the red LED
  GPIO_WriteBit(_PA_14, 0);  // Turn off the blue LED
  GPIO_WriteBit(_PA_13, 0);  // Turn off the white LED

  Wire.begin();
  Wire.setClock(400000);  // Set I2C clock speed to 400kHz

  Serial.begin(115200);
  Serial.println("—————————————————————————————— Beacon Device Reset ————————————————————————————————————————————");

  program_StartUpTime = millis();
  if (false) {
    Serial.print("program_StartUpTime:");
    Serial.println(program_StartUpTime);
  }

  payload_manager.begin();

  // Initialize button handling
  button_handler_init();

  if (beacon_pcb.begin()) {
    Serial.println("beacon_pcb: PASS");
  } else {
    Serial.println("beacon_pcb: FAIL");

    sys_reset();
    while (1)
      ;
  }

  beacon_pcb.allOutputLow();
  beacon_pcb.allOutputPin();

  {  // block of reading eeprom

    {  // vertify timeZoneOffsetMinutes saved in eeprom
      uint8_t checkByte[4] = { 0 };
      int timeZoneOffsetMinutes = 0;
      Serial.print("Before timeZoneOffsetMinutes____________________________________:");
      Serial.println(timeZoneOffsetMinutes);
      lightControl.eeprom.read(EEPROM_TIME_ZONE_OFFSET_MINUTES, checkByte, sizeof(checkByte));
      memcpy(&timeZoneOffsetMinutes, checkByte, sizeof(checkByte));
      Serial.print("After timeZoneOffsetMinutes:");
      Serial.println(timeZoneOffsetMinutes);
      payload_manager.payload_deviceConfig_sub.timeZoneOffsetMinutes = timeZoneOffsetMinutes;
    }



    if (false) {  // check status only
      uint8_t status = 0;
      lightControl.eeprom.read(EEPROM_STATUS_ADDRESS, &status, sizeof(status));
      Serial.print("EEPROM_STATUS_ADDRESS, &status:");
      Serial.println(status);
    }


    bool fResult = lightControl.getBitEeprom(EEPROM_STATUS_ADDRESS, EEPROM_STATUS_BIT_POSITION_WIFI);
    payload_manager.payload_command_sub.wifiReset = fResult;
    if (false) {
      Serial.print("fResult");
      Serial.println(fResult);
    }
    lightControl.clrBitEeprom(EEPROM_STATUS_ADDRESS, EEPROM_STATUS_BIT_POSITION_WIFI);


    unsigned char default_detection_mode = 255;
    lightControl.eeprom.read(EEPROM_DEFAULT_DETECTION_MODE_ADDRESS, &default_detection_mode, sizeof(default_detection_mode));
    if (false) {
      Serial.print("eeprom.read.default_detection_mode:");
      Serial.println(default_detection_mode);
    }

    unsigned char muteAlarms = 0;
    lightControl.eeprom.read(EEPROM_MUTEALARMS_ADDRESS, &muteAlarms, sizeof(muteAlarms));
    if (false) {
      Serial.print("eeprom.read.muteAlarms:");
      Serial.println(muteAlarms, HEX);
    }
  }

  int flashSize = OtaProcessor::checkFlashSize();
  if (flashSize != 4 * 1024 * 1024) {
    while (1) {
      Serial.println("Flash size is not 4MB");
      delay(1000);
    }
  }
  int img2Address = OtaProcessor::checkImg2AddressInBootloader();
  if (img2Address < 0x08200000) {
    int toggle = 0;
    while (1) {
      Serial.print("img2Address is now ");
      Serial.println(img2Address, HEX);
      Serial.println("It is likely you are using a bootloader for 2MB flash chips");
      Serial.println("Please use the bootloader for 4MB flash chips as we need to do OTA");
      Serial.println("Please modify OTA_Region through source code or binary modification");
      //dump the bootloader to packages/realtek/tools/ameba_d_tools/1.1.3/bsp/image/PMU_bins/NONE
      GPIO_WriteBit(_PB_3, toggle);   // Turn off the red LED
      toggle = 1 - toggle;
      delay(1000);
    }
  }

  int imgNumber = OtaProcessor::getCurrentImgNumber();

  if (false) {
    Serial.print("Current image number is ");
    Serial.println(imgNumber);
  }

  wdt.InitWatchdog(30000);
  wdt.StartWatchdog();

  NonvolatileDataManager::initialize();

  Serial.println("Firmware compiled at: ");
  Serial.println(lightControl.getCompiledTimeStr());
  Serial.println("device name: ");
  Serial.println(NonvolatileDataManager::getThingName());

  //  lightControl.setLightState(false);  // Turn on the light

  Serial.println("Starting radar...");
  lightControl.initRadar();  // Initialize the radar sensors
  Serial.println("Radar started");

  Serial.println("Starting ir sensor...");
  lightControl.initThermalSensor();  // Initialize the thermal sensor
  Serial.println("IR sensor started");

  Serial.println("Starting temperature sensor...");
  lightControl.initTemperatureSensor();  // Initialize the temperature sensor
  Serial.println("Temperature sensor started");

  Serial.println("Starting RTC...");
  lightControl.initRTC();  // Initialize the RTC
  Serial.println("RTC started");

  Serial.println("Starting EEPROM...");
  lightControl.initEEPROM();  // Initialize the EEPROM
  Serial.println("EEPROM started");


  lightControl.begin();                                                             // fetch eeprom data
  // lightControl.get8hourSectionStartTime();                                          // Initialize the 8-hour bucket timestamp
  // lightControl.eepromGetAccumulatedExposure_16Level(lightControl.inProgress8hourSectionStartTime); // Re-load exposure state from EEPROM
  // if (lightControl.accumulatedExposure >= lightControl.accumulatedExposureThreshold) {
  //   lightControl.setUiLedState(UI_LED_WHITE, UI_LED_BLINK);
  // }
  lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);  //processSmartMode,
  lightControl.processSensorInfo(true);                                             //fetch distance, find min distance
  getSensorData(false);                                                             // emergency shut down below 30cm

  {
    if (!lightControl.radar1Valid) { record_lastErrorTimestamp_print(false); }
    if (!lightControl.radar2Valid) { record_lastErrorTimestamp_print(false); }
    if (!lightControl.thermalSensorValid) { record_lastErrorTimestamp_print(false); }
    if (!lightControl.eepromValid) { record_lastErrorTimestamp_print(false); }
    if (!lightControl.temperatureSensorValid) { record_lastErrorTimestamp_print(false); }
    if (!lightControl.rtcValid) { record_lastErrorTimestamp_print(false); }
  }

  payload_manager.payload_telemetry_pub.lastErrorTimestamp = lightControl.getLongWordEeprom(EEPROM_LASTERROR_TIMESTAMP_4);

  if (false) {
    Serial.print("lastErrorTimestamp:");
    Serial.println(payload_manager.payload_telemetry_pub.lastErrorTimestamp);
  }


  lightControl.startPeriodicLightProcess();  // Start periodic light process

  lightControl.lightOnTimeForLoggingInIntervalUnit = lightControl.eepromReadLightOnData();
  Serial.print("Initial light on time in interval unit read from EEPROM: ");
  Serial.println(lightControl.lightOnTimeForLoggingInIntervalUnit);

  // if radar or thermal sensor is not valid, we retry 5 times
  if (!lightControl.radar1Valid || !lightControl.radar2Valid || !lightControl.thermalSensorValid) {
    for (int i = 0; i < 5; i++) {
      Serial.println("Retrying sensor initialization...");
      lightControl.initRadar();
      lightControl.initThermalSensor();
      if (lightControl.radar1Valid && lightControl.radar2Valid && lightControl.thermalSensorValid) {
        break;  // Exit loop if all sensors are valid
      }
      delay(1000);  // Wait before retrying
    }
  }

  if (lightControl.radar1Valid && lightControl.radar2Valid && lightControl.thermalSensorValid && lightControl.temperatureSensorValid && lightControl.rtcValid && lightControl.eepromValid) {
    // All sensors are valid, proceed with normal operation
  } else {
    bool atLeastOneSensorValid = lightControl.radar1Valid || lightControl.radar2Valid || lightControl.thermalSensorValid;
    if (!atLeastOneSensorValid) {
      lightControl.setUiLedState(UI_LED_RED, UI_LED_BLINK);  // Set red LED to blink if any sensor is not valid
    }

    int printCycle = atLeastOneSensorValid ? 1 : 5;

    for (int i = 0; i < printCycle; i++) {
      Serial.print("radar1Valid: ");
      Serial.print(lightControl.radar1Valid);
      Serial.print(" radar2Valid: ");
      Serial.print(lightControl.radar2Valid);
      Serial.print(" thermalSensorValid: ");
      Serial.print(lightControl.thermalSensorValid);
      Serial.print(" temperatureSensorValid: ");
      Serial.print(lightControl.temperatureSensorValid);
      Serial.print(" rtcValid: ");
      Serial.print(lightControl.rtcValid);
      Serial.print(" eepromValid: ");
      Serial.println(lightControl.eepromValid);
      delay(1000);
    }
    if (!atLeastOneSensorValid) {
      sys_reset();
      delay(100000);
    }
    //keep going even some sensors are not valid
  }

  char *ssidFromFlash, *passwordFromFlash;

  Serial.println("Reading stored WiFi credentials from flash...");

  NonvolatileDataManager::getSsidPasswordFromFlash(&ssidFromFlash, &passwordFromFlash);
  Serial.print("Stored SSID: ");
  Serial.println(ssidFromFlash);
  Serial.print("Stored Password: ");
  Serial.println(passwordFromFlash);

  if (ssidFromFlash[0] != 0) {
    Serial.println("Connecting to stored WiFi credentials...");
    lastWifiTryConnectTime = millis();
    lightControl.setUiLedState(UI_LED_BLUE, UI_LED_BLINK);  // Set blue LED to blink while connecting
    if (strlen(passwordFromFlash) == 0) {
      WiFi.begin(ssidFromFlash);  // Connect without password
    } else {
      WiFi.begin(ssidFromFlash, passwordFromFlash);  // Connect with password
    }
  } else {
    Serial.println("No stored WiFi credentials found, starting BLE advertising for WiFi config...");
    lastWifiConnectedTime = millis() - NO_CONNECTION_RESTART_BLE_CONFIG_TIME;  // Set to 10 seconds ago to trigger advertising immediately
  }

  button_handler_enable_interrupt();

  awsMqtt.begin();
}  //end  void setup()

void getSensorData(bool wifiStatus) {
  //calculate lamp on time for logging
  Serial.println("START getSensorData ————————————————————————————————————————————");
  {
    if (lightControl.lightOnTimeLastCheckMilliseconds == 0) {
      lightControl.lightOnTimeLastCheckMilliseconds = millis();
    } else {
      uint32_t elapsedTime = millis() - lightControl.lightOnTimeLastCheckMilliseconds;
      lightControl.lightOnTimeNotLoggedYetMilliseconds += elapsedTime;
      if (lightControl.lightOnTimeNotLoggedYetMilliseconds >= LED_ON_LOG_INTERVAL) {
        lightControl.lightOnTimeForLoggingInIntervalUnit += 1;
        lightControl.eepromWriteLightOnData(lightControl.lightOnTimeForLoggingInIntervalUnit);
        lightControl.lightOnTimeNotLoggedYetMilliseconds -= LED_ON_LOG_INTERVAL;
      }
      lightControl.lightOnTimeLastCheckMilliseconds = millis();
    }
  }

  float temperatureFromTMP;

  Serial.println("Fetching sensor data...");
  lightControl.checkRadarData(false);

  lightControl.rtc.outputNowDateTime();  //currentRtcTime  = this->now();

  temperatureFromTMP = lightControl.getTemperatureData();
  Serial.print("Sensor temperature:");
  Serial.println(temperatureFromTMP);

  print_now();
  print_second();


  if (awsMqtt.initOtaJobTimeMessageUnixTime != 0) {
    //update RTC time if the time difference between AWS message and RTC is more than 10 seconds
    //The AWS time is based on the time when the job inquiry was received
    uint32_t timeDiffFromAwsMessage = (uint32_t)(millis() - awsMqtt.initOtaJobTimeMessageMillis);
    uint32_t currentUnixTimeWithAwsReference = awsMqtt.initOtaJobTimeMessageUnixTime + (timeDiffFromAwsMessage / 1000);
    uint32_t currentUnixTimeRTC = lightControl.currentRtcTime.unixtime();
    int32_t timeDiffAwsRtc = (int32_t)(currentUnixTimeWithAwsReference - currentUnixTimeRTC);
    if (abs(timeDiffAwsRtc) > 10) {  // If the difference is more than 10 seconds
      Serial.print("Time difference between AWS message and RTC is ");
      Serial.print(timeDiffAwsRtc);
      Serial.println(" seconds, resetting RTC to AWS time.");
      DateTime awsDateTime(currentUnixTimeWithAwsReference);
      lightControl.rtc.adjust(awsDateTime);  // Adjust RTC to AWS time

      lightControl.rtc.outputNowDateTime();  //currentRtcTime  = this->now();
    }
    awsMqtt.initOtaJobTimeMessageUnixTime = 0;  // Reset after use
  }

  lightControl.get8hourSectionStartTime();

  bool fAnyObject = false;
  fAnyObject = lightControl.processSensorInfo(false);
  Serial.print("device detected at least one object:");
  Serial.println(fAnyObject);

  Serial.print("Sensor minimal distance: ");
  Serial.println(lightControl.calculatedMinimalDistance);

  //Emergency shutoff if user appear in 1FT, 30cm
  if ((lightControl.calculatedMinimalDistance < 300)) {
    if (((lightControl.uvLampMode == UV_MODE_SMART) || (lightControl.uvLampMode == UV_MODE_MANUAL))) {
      Serial.println("Minimal distance is less than 30cm, turning off the light");
      if (lightControl.getLightState()) {

        lightControl.turnOffandRecord();
        Serial.print("After turnOffandRecord lightControl.getLightState()__________________: ");
        Serial.println(lightControl.getLightState());

        // If the light is on, log the sensor data before turning it off
        String emergencyData = "{\"emergencyOff\": {\"minimalDistanceMm\": " + String(lightControl.calculatedMinimalDistance) + "}}";
        if (wifiStatus) {
          awsMqtt.logSensorToServer((char *)emergencyData.c_str());
          //sendLoggingAndTelemetryDataToServer();
        }
      }
    }  //end    if (((lightControl.uvLampMode == UV_MODE_SMART)
  } else {

    lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);

  }  //end    else
  Serial.println("———————————————————————————————————————————— End getSensorData");
}  //end      void getSensorData(bool wifiStatus)

void sendLoggingAndTelemetryDataToServer() {
  bool occupied = (lightControl.calculatedMinimalDistance < 2000);  //consider occupied if someone is closer than 2 meters
  bool lampStatus = lightControl.getLightState();
  bool lampEnable = (lightControl.uvLampMode != UV_MODE_OFF);

  int detectionMode = lightControl.uvLampMode;
  if (detectionMode == UV_MODE_OFF) { detectionMode = UV_MODE_MANUAL; }

  bool scheduleEnable = awsMqtt.scheduleEnabled;
  bool factoryReset = payload_manager.payload_command_sub.factoryReset;
  bool wifiReset = payload_manager.payload_command_sub.wifiReset;
  int telemetryIntervalSeconds = awsMqtt.telemetryIntervalSeconds;
  int timeZoneOffsetMinutes = payload_manager.payload_deviceConfig_sub.timeZoneOffsetMinutes;
  int uptimeSeconds = (millis() - program_StartUpTime) / 1000;

  uint32_t lastErrorTimestamp = payload_manager.payload_telemetry_pub.lastErrorTimestamp;

  // char *sensorHealth = "ok";

  Serial.print("awsMqtt.scheduleEnabled____________________:");
  Serial.println(awsMqtt.scheduleEnabled);
  Serial.print("scheduleEnable:");
  Serial.println(scheduleEnable);


  lampStatus = lightControl.getLightState();

  String telemetryData = "{ \"occupied\": " + String(occupied ? "true" : "false") + ", \"lampStatus\": " + String(lampStatus ? "true" : "false") + ", \"settings\": " + "{ " + "\"lampEnable\": " + String(lampEnable ? "true" : "false") + ", \"detectionMode\": " + String(detectionMode) + ", \"scheduleEnable\": " + String(scheduleEnable ? "true" : "false") + ", \"factoryReset\": " + String(factoryReset ? "true" : "false") + ", \"wifiReset\": " + String(wifiReset ? "true" : "false") + ", \"telemetryIntervalSeconds\": " + String(telemetryIntervalSeconds) +
                         //", \"rtcUnixTime\": "                 + String((int)lightControl.rtc.outputNowUnixtime()) +

                         ", \"schedule\": [ " + String(lightControl.ScheduleStr) + " ]" + ", \"timeZoneOffsetMinutes\": " + String(timeZoneOffsetMinutes) +


                         ", \"lampOnTimestamps\": " + "[ " + "{ " + "\"start\": " + String((int)lightControl.lampOnRecord[2].lampOnTimeStamps_start) + ", "
                                                                                                                                                       "\"end\": "
                         + String(lightControl.lampOnRecord[2].lampOnTimeStamps_end) + ", "
                                                                                       "\"lengthMinutes\": "
                         + String(lightControl.lampOnRecord[2].length, 0) + ", "
                                                                            "\"reason\": "
                         + "\"Hit 8 hr limit\"" + " }" + ", " +

                         "{ " + "\"start\": " + String((int)lightControl.lampOnRecord[0].lampOnTimeStamps_start) + ", " + "\"end\": " + String(lightControl.lampOnRecord[0].lampOnTimeStamps_end) + ", " + "\"lengthMinutes\": " + String(lightControl.lampOnRecord[0].length, 0) + ", " + "\"reason\": " + "\"Manual Mode shut off\"" + " }" + ", " +


                         "{ " + "\"start\": " + String((int)lightControl.lampOnRecord[1].lampOnTimeStamps_start) + ", " + "\"end\": " + String(lightControl.lampOnRecord[1].lampOnTimeStamps_end) + ", " + "\"lengthMinutes\": " + String(lightControl.lampOnRecord[1].length, 0) + ", " + "\"reason\": " + "\"Emergency shut off\"" + " }" +


                         " ] " + "}" +

                         ", \"firmwareVersion\": " + "\"2026.06.10\"" + ", \"rtcUnixTime\": " + String((int)lightControl.rtc.outputNowUnixtime()) + ", \"accumulatedEnergyMJ\": " + String(lightControl.accumulatedExposure) + ", \"personCount\": " + "\"" + String(lightControl.getpersonCount()) + "+\"" + ", \"temperatureCelsius\": " + String(lightControl.temperatureData, 0) + ", \"lastCommandStatus\": " + "\"success\"" + ", \"motionEvents\": " + String(lightControl.getmotionEvents()) + ", \"uptimeSeconds\": " + String(uptimeSeconds) + ", \"lastErrorTimestamp\": " + String(lastErrorTimestamp) + ", \"sensorHealth\": " + String(lightControl.getsensorHealth_print(false) ? "\"ok\"" : "\"fault\"") +
                         //", \"sensorHealth\": "         + String( lightControl.getsensorHealth() ? "\"ok\"":"\"fault\"") +
                         ", \"error\": " + String(get_errorStatus()) +
                         //", \"error\": "                + "0"                                                            +
                         " }";

  awsMqtt.sendTelemetryToServer((char *)telemetryData.c_str());
  delay(100);
}

/// @brief loop is an Arduino function that is called repeatedly \n
/// In this project, it checks the button activity, check sensor data, and handle WiFi connection. \n
/// Every minute it will calculate the accumulated exposure with minimal distance data and write the data to EEPROM. \n
/// Depending on the Mode, the light will be turned on or off. \n
//static bool f_loop;

void loop() {

  Serial.println("\r\n—————————————————————————————— Beacon Device Loop Start ————————————————————————————————————————————");

  {
    lightControl.lightState = lightControl.getLightState();
  }

  // Serial.println("START Checking Button Acitivity ————————————————————————————————————————————");
  int buttonState = checkButtonActivity();
  if ((buttonState & (1 << 0))) {
    Serial.println("Button short pressed__________________________________________________________________");

    if (lightControl.uvLampMode == UV_MODE_MANUAL) {
      //turn off the light if it is on
      Serial.println("Button switching UV lamp mode to OFF____________________________________________________");

      lightControl.uvLampMode = UV_MODE_OFF;

    } else {
      //turn on the light if it is off
      Serial.println("Button switching UV lamp mode to MANUAL__________________________________________________");

      lightControl.uvLampMode = INT_UV_MODE_MANUAL;

    }

    lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);
    sendLoggingAndTelemetryDataToServer();

    lightControl.setDetectionModeEeprom();

  } else if (buttonState & (1 << 1)) {

    Serial.println("Button long pressed_________________________________________________________________");
    resetWifi();

  }

  { // Check if fan needs to be enabled based on lamp state and temperature

    float temperatureFromTMP;
    temperatureFromTMP = lightControl.getTemperatureData();

    if (false) {
      Serial.println("____________________ beacon_pcb.fanControl __________________________");
      Serial.print("lightControl.lightState:");
      Serial.println(lightControl.lightState);
      Serial.print("temperatureFromTMP:");
      Serial.println(temperatureFromTMP);
    }

    beacon_pcb.fanControl(lightControl.lightState, temperatureFromTMP, false);

  }
  // Serial.println("———————————————————————————————————————————— END Checking Button Acitivity");

  // Serial.println("START Checking WiFi Connection ————————————————————————————————————————————");
  fetchWifiSetting();
  char *ssid = getWifiSsidAfterFetch();
  char *password = getWifiPasswordAfterFetch();
  // WiFi.begin with wrong SSID or password, or not begin, or disconnected, the ssid will be empty.
  bool wifiConnected = (ssid[0] != 0);
  if (wifiConnected && (!bleAdvertisingForWifiConfig)) {
    if (awsMqtt.otaProcessor.pageSwapData == NULL) {
      //do not compete with memory
      getSensorData(wifiConnected);
    } else {
      //just turn off the light when OTA is in progress
      lightControl.setLightState(false);
    }
  }
  // Serial.println("———————————————————————————————————————————— END Checking WiFi Connection");

  // Serial.println("START Handling WiFi Connection ————————————————————————————————————————————");
  if (wifiWasConnected != wifiConnected) {
    if (wifiConnected) {

      lastWifiTryConnectTime = 0;
      Serial.println("WiFi connected");
      Serial.print("ssid: ");
      Serial.println(ssid);
      Serial.print("password: ");
      Serial.println(password);
      // IPAddress ip = WiFi.localIP();
      // Serial.print("IP Address: ");
      // Serial.println(ip);
      if (bleAdvertisingForWifiConfig) {
        unsigned long waitSendOut = millis();
        while ((millis() - waitSendOut) < (10000) && (esp32Provision->wifiConnectedMessageSent < 2)) {
          //wait for 3 seconds to send out the response, especially for iOS
          esp32Provision->loop();
          delay(100);
        }
        if (esp32Provision->wifiConnectedMessageSent >= 2) {
          //give iOS more time to close connection
          delay(500);
        }
        Serial.println("BLE advertising for wifi config stopped");
        //BLE.configAdvert()->stopAdv();
        if (esp32Provision != nullptr) {
          esp32Provision->stop();
          delete esp32Provision;
          esp32Provision = nullptr;
        }
        bleAdvertisingForWifiConfig = false;
      }
      NonvolatileDataManager::saveSsidPasswordToFlash(ssid, password);
    } else {
      Serial.println("WiFi disconnected");
      if (lightControl.uvLampMode != UV_MODE_SMART) {
        // force UV lamp mode to smart if WiFi is disconnected
        lightControl.uvLampMode = UV_MODE_SMART;
        Serial.println("WiFi disconnected, setting UV lamp mode to SMART");
        lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);
      }
    }
  } else {
    if (wifiConnected) {

      lastWifiConnectedTime = millis();

      awsMqtt.loop();

      if (awsMqtt.fConnectingMqtt) {
        lightControl.setUiLedState(UI_LED_BLUE, UI_LED_BLINK);
      } else {
        lightControl.setUiLedState(UI_LED_BLUE, UI_LED_ON);
      }
    }
    if (!wifiConnected) {
      if ((((int32_t)(millis() - lastWifiConnectedTime)) > NO_CONNECTION_RESTART_BLE_CONFIG_TIME) && !bleAdvertisingForWifiConfig) {
        Serial.println("WiFi lost connection for 10 seconds, restarting BLE advertising for wifi config");
        lightControl.setUiLedState(UI_LED_BLUE, UI_LED_OFF);
        if (!bleAdvertisingForWifiConfig) {
          Serial.println("BLE advertising for wifi config started");

          esp32Provision = new ESP32Provision();
          esp32Provision->begin();

          lastEsp32ProvisionActivityMillis = millis();

          bleAdvertisingForWifiConfig = true;
        }
      }

      if (esp32Provision != nullptr) {
        esp32Provision->loop();
      }

      if (BLEPatched.connected()) {
        lastEsp32ProvisionActivityMillis = millis();
      }

      if (bleAdvertisingForWifiConfig && ((int)(millis() - lastEsp32ProvisionActivityMillis) > 30000)) {
        Serial.println("BLE advertising for wifi config stopped due to inactivity");
        if (esp32Provision != nullptr) {
          esp32Provision->stop();
          delete esp32Provision;
          esp32Provision = nullptr;
        }
        bleAdvertisingForWifiConfig = false;

        //try wifi one more time
        int wifiReturn = 0;
        char *ssidFromFlash = NULL, *passwordFromFlash = NULL;
        NonvolatileDataManager::getSsidPasswordFromFlash(&ssidFromFlash, &passwordFromFlash);
        if (ssidFromFlash[0] != 0) {
          Serial.print("Trying to connect to stored WiFi credentials: ");
          Serial.print(ssidFromFlash);
          Serial.print(" with password: ");
          Serial.println(passwordFromFlash);
          lightControl.setUiLedState(UI_LED_BLUE, UI_LED_BLINK);
          if (strlen(passwordFromFlash) == 0) {
            wifiReturn = WiFi.begin(ssidFromFlash);  // Connect without password
          } else {
            wifiReturn = WiFi.begin(ssidFromFlash, passwordFromFlash);  // Connect with password
          }
          Serial.print("WiFi.begin return: ");
          Serial.println(wifiReturn);
          lastWifiTryConnectTime = millis();
        }
      }


      if ((((int32_t)(millis() - lastWifiTryConnectTime)) > 15000)) {
        lightControl.setUiLedState(UI_LED_BLUE, UI_LED_OFF);
        if (awsMqtt.otaProcessor.pageSwapData == NULL) {
          //do not compete with memory
          delay(2000);
          getSensorData(wifiConnected);
        }
      }

    }  //end    if (!wifiConnected)


    {
      unsigned long currentMillis = millis();
      interval = (unsigned long)(awsMqtt.telemetryIntervalSeconds * 1000);
      if (interval == 0) { interval = 1000; }


      uint8_t diagnosticlevel = payload_manager.payload_deviceConfig_sub.diagnosticsLevel;

      if (false) {
        Serial.println("____________________ diagnosticlevel __________________________");
        Serial.print("diagnosticlevel:");
        Serial.println(payload_manager.payload_deviceConfig_sub.diagnosticsLevel);
        Serial.print("currentMillis");
        Serial.println(currentMillis);
        Serial.print("previousMillis");
        Serial.println(previousMillis);
        Serial.print("interval");
        Serial.println(interval);
      }

      //interval = interval>>1;   //  eg required 5sec  /2 = 2sec, one loop become 4sec

      if (currentMillis - previousMillis >= interval) {

        // Time to perform the event
        previousMillis = currentMillis;  // Update the last time the event occurred

        if (fToggle) {

          beacon_pcb.digitalWrite(PIN8, LOGIC_HIGH);
          action_diagnosticlevel_print(wifiConnected, diagnosticlevel, false);
          //action_diagnosticlevel( wifiConnected ,  diagnosticlevel);


        } else {
          beacon_pcb.digitalWrite(PIN8, LOGIC_LOW);
          action_diagnosticlevel_print(wifiConnected, diagnosticlevel, false);
        }  //end else

        fToggle = !fToggle;



      }  //end  if (currentMillis - previousMillis >= interval)
    }    //end

    {

      //do logic every minute
      if (lightControl.inProgress8hourSectionStartTime > 0) {
        //get8hourSectionStartTime() update the inProgress8hourSectionStartTime
        if (inProgress8hourSectionStartTimePrevious != lightControl.inProgress8hourSectionStartTime) {
          //new section started
          inProgress8hourSectionStartTimePrevious = lightControl.inProgress8hourSectionStartTime;
          lightControl.processedMinutesCount = -1;

          if (false) {
            Serial.println("==================================== Start new session ===========================================");
            Serial.print("Start new session: ");
            Serial.println(lightControl.inProgress8hourSectionStartTime);
          }

          lightControl.eepromInit_16LevelSection(lightControl.inProgress8hourSectionStartTime);
          //          lightControl.eepromInitSection(lightControl.inProgress8hourSectionStartTime);

          lightControl.accumulatedExposure = 0;
          lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);

          uint32_t timestamp;
          timestamp = lightControl.getLongWordEeprom(EEPROM_16LEVEL_LOG_START_ADDRESS);
          //lightControl.eeprom.read(0x200, timestamp, 4);
          Serial.print("timestamp at 0x200:");
          Serial.println(timestamp);

          timestamp = lightControl.getLongWordEeprom(EEPROM_16LEVEL_LOG_START_ADDRESS + EEPROM_16LEVEL_LOG_ENTRY_SIZE);
          Serial.print("timestamp at 0x200 + 246byte:");
          Serial.println(timestamp);
        }

        DateTime dateTime = lightControl.rtc.outputNowDateTime();
        lightControl.currentRtcTime = dateTime;
        Serial.print("lightControl.currentRtcTime.unixtime():");
        Serial.println(lightControl.currentRtcTime.unixtime());
        int timeDiff = (int)(lightControl.currentRtcTime.unixtime() - lightControl.inProgress8hourSectionStartTime);
        int timeDiffMinutes = timeDiff / 60;
        //int timeDiffMinutes   = timeDiff / minuteCounter;
        Serial.print("timeDiffMinutes:");
        Serial.println(timeDiffMinutes);
        Serial.print("processedMinutesCount");
        Serial.println(lightControl.processedMinutesCount);


        if (timeDiffMinutes > lightControl.processedMinutesCount) {
          Serial.println("________________________________________________________  60 sec ____________________________________________________");

          {
            lightControl.thermalSensorValid = lightControl.thermalSensor.begin(MLX90640_I2CADDR_DEFAULT);  // MLX90640 I2C address,

            if (lightControl.thermalSensorValid) {
              lightControl.checkThermalSensorData(false);
            }
          }

          lightControl.processedMinutesCount = timeDiffMinutes;
          Serial.print("Processed minutes count: ");
          Serial.println(lightControl.processedMinutesCount);

          //update the accumulated exposure
          lightControl.eepromGetAccumulatedExposure_16Level(lightControl.inProgress8hourSectionStartTime);

          if (lightControl.accumulatedExposure >= lightControl.accumulatedExposureThreshold) {
            lightControl.setUiLedState(UI_LED_WHITE, UI_LED_BLINK);
          }

          lightControl.processLightControl(awsMqtt.scheduleEnabled, awsMqtt.scheduleData);

          if (lightControl.getLightState()) {
            int accumulatedExposureThisMinute = lightControl.minimalDistanceJulesLevel;
            lightControl.eepromWrite16LevelSection(lightControl.inProgress8hourSectionStartTime, accumulatedExposureThisMinute, lightControl.processedMinutesCount);
          }

          int accumulatedExposure = lightControl.eepromGetAccumulatedExposure_16Level(lightControl.inProgress8hourSectionStartTime);

          if (false) {
            Serial.println("_______________________ Accumulated exposure for section  ________________________________________");
          }

          uint32_t timestamp;
          timestamp = lightControl.getLongWordEeprom(EEPROM_16LEVEL_LOG_START_ADDRESS);

          if (false) {
            Serial.print("timestamp at 0x200:");
            Serial.println(timestamp);
          }

          timestamp = lightControl.getLongWordEeprom(EEPROM_16LEVEL_LOG_START_ADDRESS + EEPROM_16LEVEL_LOG_ENTRY_SIZE);

          if (false) {
            Serial.print("timestamp at 0x200 + 246byte:");
            Serial.println(timestamp);
          }

          if (false) {
            Serial.print("Accumulated exposure for section ");
            Serial.print(lightControl.inProgress8hourSectionStartTime);
            Serial.print(" is ");
            Serial.print(accumulatedExposure);
            Serial.print(" with threshold ");
            Serial.println(lightControl.accumulatedExposureThreshold);
          }

          if (wifiConnected) {
            sendLoggingAndTelemetryDataToServer();
          }
        }  //end   if (timeDiffMinutes > lightControl.processedMinutesCount);  complete 60min
      }    //end        if (lightControl.inProgress8hourSectionStartTime > 0)
    }      //end  {}
  }        //end    else
  // Serial.println("———————————————————————————————————————————— END Handling WiFi Connection");

  processAwsMqtt_print(false);
  wifiWasConnected = wifiConnected;
  wdt.RefreshWatchdog();
  Serial.println("—————————————————————————————— Beacon Device Loop END ————————————————————————————————————————————\r\n");
}  //end void loop()
