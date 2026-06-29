#include "ProvisioningCodec.h"

namespace beacon {

#ifndef RTW_SECURITY_OPEN
#define RTW_SECURITY_OPEN 0
#define RTW_SECURITY_WEP_PSK 1
#define RTW_SECURITY_WPA_TKIP_PSK 2
#define RTW_SECURITY_WPA_AES_PSK 3
#define RTW_SECURITY_WPA2_AES_PSK 4
#define RTW_SECURITY_WPA2_TKIP_PSK 5
#define RTW_SECURITY_WPA2_MIXED_PSK 6
#define RTW_SECURITY_WPA_WPA2_MIXED_PSK 7
#define RTW_SECURITY_WPA3_AES_PSK 8
#define RTW_SECURITY_WPA2_WPA3_MIXED 9
#endif

std::vector<unsigned char> encodeLEB128(int64_t value) {
  std::vector<unsigned char> result;
  uint64_t convertedValue = value;
  while (convertedValue > 0x7F) {
    result.push_back(static_cast<unsigned char>((convertedValue & 0x7F) | 0x80));
    convertedValue >>= 7;
  }
  result.push_back(static_cast<unsigned char>(convertedValue & 0x7F));
  return result;
}

int convertFromRealtekAuthModeToEsp32AuthMode(uint32_t realtekAuthMode) {
  switch (realtekAuthMode) {
    case RTW_SECURITY_OPEN:
      return static_cast<int>(WifiAuthMode::Open);
    case RTW_SECURITY_WEP_PSK:
      return static_cast<int>(WifiAuthMode::WEP);
    case RTW_SECURITY_WPA_TKIP_PSK:
    case RTW_SECURITY_WPA_AES_PSK:
      return static_cast<int>(WifiAuthMode::WPA_PSK);
    case RTW_SECURITY_WPA2_AES_PSK:
    case RTW_SECURITY_WPA2_TKIP_PSK:
    case RTW_SECURITY_WPA2_MIXED_PSK:
      return static_cast<int>(WifiAuthMode::WPA2_PSK);
    case RTW_SECURITY_WPA_WPA2_MIXED_PSK:
      return static_cast<int>(WifiAuthMode::WPA_WPA2_PSK);
    case RTW_SECURITY_WPA3_AES_PSK:
      return static_cast<int>(WifiAuthMode::WPA3_PSK);
    case RTW_SECURITY_WPA2_WPA3_MIXED:
      return static_cast<int>(WifiAuthMode::WPA2_WPA3_PSK);
    default:
      return static_cast<int>(WifiAuthMode::Open);
  }
}

}  // namespace beacon
