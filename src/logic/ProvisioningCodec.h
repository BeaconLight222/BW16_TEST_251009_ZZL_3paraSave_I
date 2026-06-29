#pragma once

#include <cstdint>
#include <vector>

/// WiFi auth mode values used when encoding BLE provisioning scan results.
enum WifiAuthMode {
  Open = 0,
  WEP = 1,
  WPA_PSK = 2,
  WPA2_PSK = 3,
  WPA_WPA2_PSK = 4,
  WPA2_ENTERPRISE = 5,
  WPA3_PSK = 6,
  WPA2_WPA3_PSK = 7,
};

/// Protobuf/LEB128 helpers extracted from ESP32Provision for host testing.
namespace beacon {

std::vector<unsigned char> encodeLEB128(int64_t value);
int convertFromRealtekAuthModeToEsp32AuthMode(uint32_t realtekAuthMode);

}  // namespace beacon
