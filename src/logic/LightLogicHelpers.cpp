#include "LightLogicHelpers.h"

#include <cstdlib>

namespace {

enum ExposureLevel {
  EXPOSURE_LEVEL_0 = 0,
  EXPOSURE_LEVEL_1 = 1,
  EXPOSURE_LEVEL_2 = 2,
  EXPOSURE_LEVEL_3 = 3,
};

const uint8_t scheduleIndexShift[15] = {
  0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28,
};

}  // namespace

namespace beacon {

int julesLevelForDistanceFromMm(int distance) {
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

float lampOnDurationMinutes(uint32_t startTime, uint32_t endTime) {
  float length = 0;

  if (((endTime > startTime) && (startTime > 0) && (endTime > 0)) ||
      ((endTime == startTime) && (startTime > 0))) {
    length = static_cast<float>(endTime - startTime) / 60.0f;
    if (length < 0) {
      length = 0;
    }
  }

  return length;
}

void applyUiLedState(int* timerLedState, int ledIndex, int state) {
  if (timerLedState == nullptr) {
    return;
  }

  int cache = *timerLedState;
  cache &= ~(3 << (ledIndex * 2));
  cache |= (state << (ledIndex * 2));
  *timerLedState = cache;
}

int computeScheduleIndex(uint8_t dayOfWeek, uint8_t hour, uint8_t minute, int timeZoneOffsetMinutes) {
  int scheduleIndex = dayOfWeek * 48 + (hour * 2) + (minute / 30);
  const int timeZoneOffsetHour = timeZoneOffsetMinutes / 60;
  const int shiftIndex = scheduleIndexShift[abs(timeZoneOffsetHour)];

  if (timeZoneOffsetMinutes >= 0) {
    scheduleIndex += shiftIndex;
    if (scheduleIndex >= 336) {
      scheduleIndex -= 336;
    }
  } else {
    scheduleIndex -= shiftIndex;
    if (scheduleIndex < 0 || scheduleIndex >= 336) {
      scheduleIndex = 336 + scheduleIndex;
    }
  }

  return scheduleIndex;
}

bool readScheduleBit(const uint8_t* scheduleData, int scheduleIndex) {
  if (scheduleData == nullptr || scheduleIndex < 0 || scheduleIndex >= 336) {
    return false;
  }

  const int byteIndex = scheduleIndex / 8;
  const int bitIndex = scheduleIndex % 8;

  if (byteIndex < 0 || byteIndex >= 42) {
    return false;
  }

  return (scheduleData[byteIndex] & (1 << bitIndex)) != 0;
}

}  // namespace beacon
