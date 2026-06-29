#include "../test_harness.h"

#include "src/logic/ButtonActivityLogic.h"

void register_button_activity_logic_tests() {
  describe("ButtonActivityLogic", []() {
    it("returns zero when no button press occurred", []() {
      expect_eq(buttonPressTypeToBitmask(NO_PRESS), 0);
    });

    it("sets bit 0 for a short press", []() {
      expect_eq(buttonPressTypeToBitmask(SHORT_PRESS), 1);
    });

    it("sets bit 1 for a long press", []() {
      expect_eq(buttonPressTypeToBitmask(LONG_PRESS), 2);
    });
  });
}
