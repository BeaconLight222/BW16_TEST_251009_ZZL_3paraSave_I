#include "../test_harness.h"

#include "src/logic/ButtonActivityLogic.h"
#include "src/logic/ErrorStatusLogic.h"
#include "uv_lamp_parameter.h"

static uint8_t getErrorStatusLikeMainSketch(const ErrorStatusInputs& inputs) {
  return computeErrorStatus(inputs);
}

static int checkButtonActivityLikeConnectionUtils(ButtonPressType pressType) {
  return buttonPressTypeToBitmask(pressType);
}

void register_main_ino_logic_tests() {
  describe("BW16_TEST_251009_ZZL_3paraSave_I.ino", []() {
    it("derives ERROR_NO when all tracked sensors are valid", []() {
      ErrorStatusInputs inputs = {false, false, true, true, true, true, true, true};
      expect_eq(getErrorStatusLikeMainSketch(inputs), static_cast<uint8_t>(ERROR_NO));
    });

    it("derives ERROR_RADAR1_INVALID when only radar1 fails after other checks", []() {
      ErrorStatusInputs inputs = {false, false, false, true, true, true, true, true};
      expect_eq(getErrorStatusLikeMainSketch(inputs), static_cast<uint8_t>(ERROR_RADAR1_INVALID));
    });

    it("derives ERROR_TEMPERATURE_INVALID when the temperature sensor is invalid", []() {
      ErrorStatusInputs inputs = {false, false, true, true, true, false, true, true};
      expect_eq(getErrorStatusLikeMainSketch(inputs), static_cast<uint8_t>(ERROR_TEMPERATURE_INVALID));
    });
  });

  describe("connectionUtils.ino", []() {
    it("returns a short-press bitmask from the button handler result", []() {
      expect_eq(checkButtonActivityLikeConnectionUtils(SHORT_PRESS), 1);
    });

    it("returns a long-press bitmask from the button handler result", []() {
      expect_eq(checkButtonActivityLikeConnectionUtils(LONG_PRESS), 2);
    });

    it("returns zero when the button handler reports no activity", []() {
      expect_eq(checkButtonActivityLikeConnectionUtils(NO_PRESS), 0);
    });
  });
}
