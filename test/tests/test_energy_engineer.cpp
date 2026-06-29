#include "../test_harness.h"

#include "EnergyEngineer.h"

void register_energy_engineer_tests() {
  describe("EnergyEngineer", []() {
    it("maps very close objects to the emergency-off exposure level", []() {
      EnergyEngineer engineer;
      expect_eq(engineer.jules16LevelForDistance(200), static_cast<int>(EXPOSURE_16LEVEL_0));
      expect_eq(engineer.jules16LevelForDistance(300), static_cast<int>(EXPOSURE_16LEVEL_0));
    });

    it("maps mid-range distances to the highest exposure level", []() {
      EnergyEngineer engineer;
      expect_eq(engineer.jules16LevelForDistance(301), static_cast<int>(EXPOSURE_16LEVEL_15));
      expect_eq(engineer.jules16LevelForDistance(440), static_cast<int>(EXPOSURE_16LEVEL_15));
    });

    it("steps down exposure level as distance increases", []() {
      EnergyEngineer engineer;
      expect_eq(engineer.jules16LevelForDistance(1000), static_cast<int>(EXPOSURE_16LEVEL_11));
      expect_eq(engineer.jules16LevelForDistance(2000), static_cast<int>(EXPOSURE_16LEVEL_6));
      expect_eq(engineer.jules16LevelForDistance(5000), static_cast<int>(EXPOSURE_16LEVEL_1));
    });

    it("returns the lowest level when no object is detected beyond range", []() {
      EnergyEngineer engineer;
      expect_eq(engineer.jules16LevelForDistance(6000), static_cast<int>(EXPOSURE_16LEVEL_0));
    });

    it("initializes without error via begin()", []() {
      EnergyEngineer engineer;
      engineer.begin();
      expect_true(true);
    });
  });
}
