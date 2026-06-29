#include "ErrorStatusLogic.h"

uint8_t computeErrorStatus(const ErrorStatusInputs& inputs) {
  if (inputs.factoryReset) {
    return ERROR_SYSRESET_FACTORY;
  }
  if (inputs.wifiReset) {
    return ERROR_SYSRESET_WIFI;
  }
  if (!(inputs.radar1Valid || inputs.radar2Valid)) {
    return ERROR_2XRADAR_INVALID;
  }
  if (!inputs.temperatureSensorValid) {
    return ERROR_TEMPERATURE_INVALID;
  }
  if (!inputs.eepromValid) {
    return ERROR_EEPROM_INVALID;
  }
  if (!inputs.rtcValid) {
    return ERROR_RTC_INVALID;
  }
  if (!inputs.thermalSensorValid) {
    return ERROR_THERMAL_INVALID;
  }
  if (!inputs.radar2Valid) {
    return ERROR_RADAR2_INVALID;
  }
  if (!inputs.radar1Valid) {
    return ERROR_RADAR1_INVALID;
  }
  return ERROR_NO;
}
