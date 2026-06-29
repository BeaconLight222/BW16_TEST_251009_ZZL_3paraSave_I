#include "../test_harness.h"

#include "src/logic/LightLogicHelpers.h"

void register_light_logic_helpers_tests() {
  describe("LightLogicHelpers", []() {
    it("assigns the highest exposure level for objects within one meter", []() {
      expect_eq(beacon::julesLevelForDistanceFromMm(500), 3);
      expect_eq(beacon::julesLevelForDistanceFromMm(1000), 3);
    });

    it("assigns medium exposure levels between one and four meters", []() {
      expect_eq(beacon::julesLevelForDistanceFromMm(1500), 2);
      expect_eq(beacon::julesLevelForDistanceFromMm(3000), 1);
    });

    it("assigns the lowest exposure level beyond four meters", []() {
      expect_eq(beacon::julesLevelForDistanceFromMm(5000), 0);
    });

    it("calculates lamp-on duration in minutes from unix timestamps", []() {
      expect_eq(beacon::lampOnDurationMinutes(1000, 1600), 10.0f);
      expect_eq(beacon::lampOnDurationMinutes(0, 100), 0.0f);
    });

    it("updates only the requested UI LED bits in the timer state word", []() {
      int timerState = 0;
      beacon::applyUiLedState(&timerState, 0, 2);
      expect_eq(timerState, 2);
      beacon::applyUiLedState(&timerState, 1, 1);
      expect_eq(timerState, 6);
    });

    it("computes schedule slot indices with timezone offsets", []() {
      const int utcIndex = beacon::computeScheduleIndex(1, 12, 0, 0);
      const int shiftedIndex = beacon::computeScheduleIndex(1, 12, 0, 480);
      expect_true(utcIndex >= 0 && utcIndex < 336);
      expect_true(shiftedIndex >= 0 && shiftedIndex < 336);
      expect_true(shiftedIndex != utcIndex);
    });

    it("reads schedule enable bits from the 42-byte schedule table", []() {
      uint8_t schedule[42] = {0};
      schedule[0] = 0x01;
      expect_true(beacon::readScheduleBit(schedule, 0));
      expect_false(beacon::readScheduleBit(schedule, 1));
    });
  });
}
