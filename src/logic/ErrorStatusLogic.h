#pragma once

#include <cstdint>

#include "uv_lamp_parameter.h"

/// Holds the sensor and command flags used to derive the device error status byte.
struct ErrorStatusInputs {
  bool factoryReset;
  bool wifiReset;
  bool radar1Valid;
  bool radar2Valid;
  bool thermalSensorValid;
  bool temperatureSensorValid;
  bool eepromValid;
  bool rtcValid;
};

/// Computes the telemetry error status byte from current hardware validity and reset flags.
/// Used by the main firmware loop before publishing diagnostics.
uint8_t computeErrorStatus(const ErrorStatusInputs& inputs);
