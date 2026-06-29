#pragma once

#include <cstdint>

/// Pure light-control helpers shared by LightLogicControl and host unit tests.
namespace beacon {

/// Maps measured object distance (mm) to one of four exposure intensity levels.
int julesLevelForDistanceFromMm(int distance);

/// Returns lamp-on duration in minutes from Unix start/end timestamps.
float lampOnDurationMinutes(uint32_t startTime, uint32_t endTime);

/// Updates the packed UI LED state word used by the hardware timer callback.
void applyUiLedState(int* timerLedState, int ledIndex, int state);

/// Computes the schedule slot index after applying a timezone offset.
int computeScheduleIndex(uint8_t dayOfWeek, uint8_t hour, uint8_t minute, int timeZoneOffsetMinutes);

/// Reads whether the schedule bit is enabled for a given slot index.
bool readScheduleBit(const uint8_t* scheduleData, int scheduleIndex);

}  // namespace beacon
