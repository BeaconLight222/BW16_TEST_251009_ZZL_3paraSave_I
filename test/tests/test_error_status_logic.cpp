#include "../test_harness.h"

#include "src/logic/ErrorStatusLogic.h"
#include "uv_lamp_parameter.h"

void register_error_status_logic_tests() {
  describe("ErrorStatusLogic", []() {
    it("reports no error when all sensors are healthy", []() {
      ErrorStatusInputs inputs = {false, false, true, true, true, true, true, true};
      expect_eq(computeErrorStatus(inputs), static_cast<uint8_t>(ERROR_NO));
    });

    it("prioritizes factory reset over sensor faults", []() {
      ErrorStatusInputs inputs = {true, false, false, false, false, false, false, false};
      expect_eq(computeErrorStatus(inputs), static_cast<uint8_t>(ERROR_SYSRESET_FACTORY));
    });

    it("reports wifi reset before sensor faults", []() {
      ErrorStatusInputs inputs = {false, true, false, false, false, false, false, false};
      expect_eq(computeErrorStatus(inputs), static_cast<uint8_t>(ERROR_SYSRESET_WIFI));
    });

    it("reports dual-radar failure when both radars are invalid", []() {
      ErrorStatusInputs inputs = {false, false, false, false, true, true, true, true};
      expect_eq(computeErrorStatus(inputs), static_cast<uint8_t>(ERROR_2XRADAR_INVALID));
    });

    it("reports thermal sensor failure after other single-sensor checks", []() {
      ErrorStatusInputs inputs = {false, false, true, true, false, true, true, true};
      expect_eq(computeErrorStatus(inputs), static_cast<uint8_t>(ERROR_THERMAL_INVALID));
    });
  });
}
