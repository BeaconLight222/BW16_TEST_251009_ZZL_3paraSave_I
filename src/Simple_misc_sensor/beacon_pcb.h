#ifndef BEACON_PCB_H
#define BEACON_PCB_H
//
//#include "src/Simple_misc_sensor/Simple_misc_sensor.h"
#include "Simple_misc_sensor.h"
//#include "Simple_misc_sensor.h"

/// Board-level hardware interface for the Beacon UV lamp PCB.
/// Extends the I/O expander to control ballast power, fan cooling, and the buzzer.
class BEACON_PCB: public IO_PCA9539{
public:
        void ballastPowerControl(bool flag);        
        //void ballastPower(bool flag);

        void fanControl(bool f_lightState, float temperature, bool fprint);

        void testFan(int times, bool fprint);
        void beep_sound(unsigned char time);
};




#endif