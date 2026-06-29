#include "../test_harness.h"

#include "Arduino.h"
#include "ToolsArray.h"

void register_tools_array_tests() {
  describe("ToolsArray", []() {
    it("detects when the least significant bit is set", []() {
      expect_true(ToolsArray::record_lsb(0x01));
      expect_false(ToolsArray::record_lsb(0x00));
      expect_false(ToolsArray::record_lsb(0xFE));
    });

    it("rotates a 42-byte schedule right by one bit", []() {
      uint8_t schedule[42] = {0};
      schedule[0] = 0x01;
      ToolsArray::rotate_rightx1(schedule, 42);
      expect_eq(static_cast<int>(schedule[0]), 0);
      expect_eq(static_cast<int>(schedule[1]), 0x80);
    });

    it("rotates multiple times through rotate_right()", []() {
      uint8_t schedule[42] = {0};
      schedule[0] = 0x03;
      ToolsArray::rotate_right(2, schedule, 42);
      expect_eq(static_cast<int>(schedule[1]), 0xC0);
    });
  });
}
