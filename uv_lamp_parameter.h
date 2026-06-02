#ifndef   UV_LAMP_PARAMETER_H
#define   UV_LAMP_PARAMETER_H

#define   ERROR_NO                        0
#define   ERROR_MQTT_DISCONNECT           100
#define   ERROR_RADAR1_INVALID            101
#define   ERROR_RADAR2_INVALID            102
#define   ERROR_THERMAL_INVALID           103
#define   ERROR_RTC_INVALID               104
#define   ERROR_EEPROM_INVALID            105
#define   ERROR_TEMPERATURE_INVALID       106
#define   ERROR_2XRADAR_INVALID           107
#define   ERROR_3XSENSOR_INVALID          108

#define   ERROR_SYSRESET_WIFI             110
#define   ERROR_SYSRESET_FACTORY          111
 
#define   ERROR_UNDEFINED                 255


#define   INT_UV_MODE_SMART         (0x00) 
#define   INT_UV_MODE_MANUAL        (0x01)
#define   INT_UV_MODE_UNLIMITED     (0x02)
#define   INT_UV_MODE_OCCUPIED_ROOM (0x03)
#define   INT_UV_MODE_EMPTY_ROOM    (0x04)
#define   INT_UV_MODE_IDLE_OFF        (0XFC)
#define   INT_UV_MODE_IDLE_MANUAL     (0xFD)
//#define   INT_UV_MODE_IDLE          (0xFE)
#define   INT_UV_MODE_OFF             (0xFF) 

enum uv_lamp_mode {
  UV_MODE_SMART = 0x00,
  UV_MODE_MANUAL = 0x01,
  UV_MODE_UNLIMITED = 0x02,
  UV_MODE_OCCUPIED_ROOM = 0x03,
  UV_MODE_EMPTY_ROOM = 0x04,
  UV_MODE_OFF = 0xFF,
};

typedef     enum uv_lamp_mode  uv_lamp_mode_enum;
#endif
