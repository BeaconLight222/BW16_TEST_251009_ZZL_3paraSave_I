#include "../test_harness.h"

#include "Arduino.h"
#include "payload.h"

void register_payload_manager_tests() {
  describe("PAYLOAD_MANAGER", []() {
    it("initializes command defaults on begin()", []() {
      PAYLOAD_MANAGER manager;
      manager.begin();
      expect_false(manager.payload_command_sub.wifiReset);
      expect_eq(static_cast<int>(manager.payload_deviceConfig_sub.diagnosticsLevel), 0);
    });

    it("starts with an empty pending command list", []() {
      PAYLOAD_MANAGER manager;
      manager.begin();
      expect_eq(static_cast<int>(manager.list_newModeCommand.size()), 0);
    });
  });
}
