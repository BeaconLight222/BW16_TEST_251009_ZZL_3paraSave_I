#include "../test_harness.h"

#include "src/logic/ProvisioningCodec.h"

void register_provisioning_codec_tests() {
  describe("ProvisioningCodec", []() {
    it("encodes small integers as a single LEB128 byte", []() {
      const auto encoded = beacon::encodeLEB128(42);
      expect_eq(static_cast<int>(encoded.size()), 1);
      expect_eq(static_cast<int>(encoded[0]), 42);
    });

    it("encodes larger integers using continuation bytes", []() {
      const auto encoded = beacon::encodeLEB128(300);
      expect_eq(static_cast<int>(encoded.size()), 2);
      expect_eq(static_cast<int>(encoded[0]), 0xAC);
      expect_eq(static_cast<int>(encoded[1]), 0x02);
    });

    it("maps Realtek open networks to ESP32 open auth mode", []() {
      expect_eq(beacon::convertFromRealtekAuthModeToEsp32AuthMode(0), static_cast<int>(Open));
    });

    it("maps Realtek WPA2 networks to ESP32 WPA2 auth mode", []() {
      expect_eq(beacon::convertFromRealtekAuthModeToEsp32AuthMode(4), static_cast<int>(WPA2_PSK));
    });

    it("defaults unknown auth modes to open", []() {
      expect_eq(beacon::convertFromRealtekAuthModeToEsp32AuthMode(999), static_cast<int>(Open));
    });
  });
}
