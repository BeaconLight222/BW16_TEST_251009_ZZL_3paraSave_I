/// @file connectionUtils.ino
/// @brief Connection utilities for the BW16 project

extern "C" int wifi_set_autoreconnect(uint8_t mode);

#include "bte.h"
#include "wifi_conf.h"
#include "wifi_constants.h"
#include "wifi_drv.h"
#include "src/ButtonHandling/ButtonHandler.h"


#include "wifi_structures.h"
extern "C" int wifi_get_setting(const char *ifname, rtw_wifi_setting_t *pSetting);
rtw_wifi_setting_t wifi_setting_fetched = { RTW_MODE_NONE, { 0 }, 0, RTW_SECURITY_OPEN, { 0 }, 0 };

/// @brief wrapper for SDK wifi_get_setting
int fetchWifiSetting() {
  return wifi_get_setting(WLAN0_NAME, &wifi_setting_fetched);
}

char *getWifiSsidAfterFetch() {
  return (char *)wifi_setting_fetched.ssid;
}
char *getWifiPasswordAfterFetch() {
  return (char *)wifi_setting_fetched.password;
}

/// @brief Check the button on pin A15 activity and return a bitmask of the button states
/// @return A bitmask where: \n
/// - Bit 0: Button is short pressed \n
/// - Bit 1: Button is long pressed \n
int checkButtonActivity() {
  // Check for button activity using the button handler module
  ButtonPressType buttonPressType = button_handler_check_activity_and_reset();
  int returnValue = 0;
  if (buttonPressType == SHORT_PRESS) {
    returnValue |= (1 << 0);
  } else if (buttonPressType == LONG_PRESS) {
    returnValue |= (1 << 1);
  }
  return returnValue;
}