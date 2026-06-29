cmake_minimum_required(VERSION 3.16)
project(beacon_unit_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(beacon_unit_tests
  main.cpp
  tests/test_energy_engineer.cpp
  tests/test_tools_array.cpp
  tests/test_error_status_logic.cpp
  tests/test_button_activity_logic.cpp
  tests/test_light_logic_helpers.cpp
  tests/test_provisioning_codec.cpp
  tests/test_payload_manager.cpp
  tests/test_main_ino_logic.cpp
  ${REPO_ROOT}/EnergyEngineer.cpp
  ${REPO_ROOT}/payload.cpp
  ${REPO_ROOT}/src/logic/ErrorStatusLogic.cpp
  ${REPO_ROOT}/src/logic/ButtonActivityLogic.cpp
  ${REPO_ROOT}/src/logic/LightLogicHelpers.cpp
  ${REPO_ROOT}/src/logic/ProvisioningCodec.cpp
)

target_include_directories(beacon_unit_tests PRIVATE
  ${REPO_ROOT}
  ${REPO_ROOT}/test/mocks
  ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_testing()
add_test(NAME beacon_unit_tests COMMAND beacon_unit_tests)
