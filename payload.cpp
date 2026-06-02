#include "payload.h"

PAYLOAD_MANAGER     payload_manager;

void PAYLOAD_MANAGER::begin(){

payload_command_sub.wifiReset               = false;

payload_deviceConfig_sub.diagnosticsLevel = 0;
//payload_deviceConfig_sub.diagnosticsLevel = 1;
//payload_deviceConfig_sub.diagnosticsLevel = 9;

}//end void