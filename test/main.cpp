#include "../test_harness.h"

void register_energy_engineer_tests();
void register_tools_array_tests();
void register_error_status_logic_tests();
void register_button_activity_logic_tests();
void register_light_logic_helpers_tests();
void register_provisioning_codec_tests();
void register_payload_manager_tests();
void register_main_ino_logic_tests();

int main() {
  register_energy_engineer_tests();
  register_tools_array_tests();
  register_error_status_logic_tests();
  register_button_activity_logic_tests();
  register_light_logic_helpers_tests();
  register_provisioning_codec_tests();
  register_payload_manager_tests();
  register_main_ino_logic_tests();
  return test_harness::run_all_tests();
}
