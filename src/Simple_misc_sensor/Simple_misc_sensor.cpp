#include "Simple_misc_sensor.h"

#include "i2c_api.h"
extern i2c_t i2cwire0;
extern "C" void i2c_restart_enable(i2c_t *obj);

const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};


// Start of NXP IO Expander  Definition ////////////////////////////////////////////

bool IO_PCA9539::begin() {
  i2c_restart_enable(&i2cwire0);

  // Check if the device is connected
  byte x = 0;
  Wire.beginTransmission(IO_PCA9539_ADDRESS);
  Wire.write(x); // Request time register
  if (Wire.endTransmission() != 0) {
    return false; // Device not found
  }

  // Assume the device is a DS3231 if we can communicate with it
  return true;
}

/*
void IO_PCA9539::output_PIN12(bool flag){
  digitalWrite(PIN12, flag?LOGIC_HIGH:LOGIC_LOW);
}  
*/

/*
void IO_PCA9539::ballastPower(bool flag){
  digitalWrite(PIN12, flag?LOGIC_HIGH:LOGIC_LOW);
}  
*/





void IO_PCA9539::fanControl(bool flag){
  digitalWrite(PIN0, flag?LOGIC_HIGH:LOGIC_LOW);
}  

void IO_PCA9539::fanOn(){
  digitalWrite(PIN0, LOGIC_HIGH);
}

void IO_PCA9539::fanOff(){
  digitalWrite(PIN0, LOGIC_LOW);
}

void IO_PCA9539::allOutputLow(){
  digitalWrite(PIN15, LOGIC_LOW);
  digitalWrite(PIN14, LOGIC_LOW);
  digitalWrite(PIN13, LOGIC_LOW);
  digitalWrite(PIN12, LOGIC_LOW);
  digitalWrite(PIN11, LOGIC_LOW);
  digitalWrite(PIN10, LOGIC_LOW);
  digitalWrite(PIN9, LOGIC_LOW); 
  digitalWrite(PIN8, LOGIC_LOW); 
  digitalWrite(PIN7, LOGIC_LOW);
  digitalWrite(PIN6, LOGIC_LOW);
  digitalWrite(PIN5, LOGIC_LOW);
  digitalWrite(PIN4, LOGIC_LOW);
  digitalWrite(PIN3, LOGIC_LOW);
  digitalWrite(PIN2, LOGIC_LOW);
  digitalWrite(PIN1, LOGIC_LOW);
  digitalWrite(PIN0, LOGIC_LOW);
}//end


void IO_PCA9539::allOutputHigh(){
  digitalWrite(PIN15, LOGIC_HIGH);
  digitalWrite(PIN14, LOGIC_HIGH);
  //digitalWrite(PIN13, LOGIC_HIGH);
  digitalWrite(PIN12, LOGIC_HIGH);
  digitalWrite(PIN11, LOGIC_HIGH);
  digitalWrite(PIN10, LOGIC_HIGH);
  digitalWrite(PIN9, LOGIC_HIGH); 
  digitalWrite(PIN8, LOGIC_HIGH); 
  digitalWrite(PIN7, LOGIC_HIGH);
  digitalWrite(PIN6, LOGIC_HIGH);
  digitalWrite(PIN5, LOGIC_HIGH);
  digitalWrite(PIN4, LOGIC_HIGH);
  digitalWrite(PIN3, LOGIC_HIGH);
  digitalWrite(PIN2, LOGIC_HIGH);
  digitalWrite(PIN1, LOGIC_HIGH);
  digitalWrite(PIN0, LOGIC_HIGH);
}//end

void IO_PCA9539::allOutputPin(){
    pinMode(PIN0, MODE_OUTPUT);
    pinMode(PIN1, MODE_OUTPUT);
    pinMode(PIN2, MODE_OUTPUT);
    pinMode(PIN3, MODE_OUTPUT);
    pinMode(PIN4, MODE_OUTPUT);
    pinMode(PIN5, MODE_OUTPUT);
    pinMode(PIN6, MODE_OUTPUT);
    pinMode(PIN7, MODE_OUTPUT);
    pinMode(PIN8, MODE_OUTPUT);
    pinMode(PIN9, MODE_OUTPUT);
    pinMode(PIN10, MODE_OUTPUT);
    pinMode(PIN11, MODE_OUTPUT);
    pinMode(PIN12, MODE_OUTPUT);
    pinMode(PIN13, MODE_OUTPUT);
    pinMode(PIN14, MODE_OUTPUT);
    pinMode(PIN15, MODE_OUTPUT);
}

void IO_PCA9539::pinMode(uint8_t pin, uint8_t IOMode) {

    if (pin <= 15) {
        //
        // now set the correct bit in the configuration register
        //

        if (IOMode == MODE_OUTPUT) {    
        //if (IOMode == OUTPUT) {
            //
            // mask correct bit to 0 by inverting x so that only
            // the correct bit is LOW. The rest stays HIGH
            //
            _configurationRegister = _configurationRegister & ~(1 << pin);
        } else {
            //
            // or just the required bit to 1
            //
            _configurationRegister = _configurationRegister | (1 << pin);
        }

        //Serial.print("Debug: _config:");
        //Serial.println(_configurationRegister,HEX);
        // write configuration register to chip
        
//handle low
        uint8_t buffer[2] = {NXP_CONFIG_LOW,
                            _valueRegister_low,
                            };

        write_then_read(buffer, 2, nullptr, 0, true); 

//handle high
          uint8_t bufferB[2] = {NXP_CONFIG_HIGH,
                               _valueRegister_high,
                               };

          //uint8_t bufferB[2] = {NXP_CONFIG_LOW,
          //                  _valueRegister_low,
          //                  };

        write_then_read(bufferB, 2, nullptr, 0, true); 
      
        //I2CSetValue(_address, NXP_CONFIG    , _configurationRegister_low);
        //I2CSetValue(_address, NXP_CONFIG + 1, _configurationRegister_high);
    }// if (pin <= 15)

}

uint8_t IO_PCA9539::digitalRead(uint8_t pin) {
    uint16_t _inputData = 0;
    //
    // we wil only process pins <= 15
    //
    if (pin > 15 ) return 255;
    _inputData  = I2CGetValue(_address, NXP_INPUT);
    _inputData |= I2CGetValue(_address, NXP_INPUT + 1) << 8;
    //
    // now mask the bit required and see if it is a HIGH
    //
    if ((_inputData & (1 << pin)) > 0){
        //
        // the bit is HIGH otherwise we would return a LOW value
        //
        return HIGH;
    } else {
        return LOW;
    }
}


bool IO_PCA9539::digitalWrite(uint8_t pin, uint8_t value) {
bool flag;

 if (value > 0) {
        //
        // this is a High value so we will or it with the value register
        //
        _valueRegister = _valueRegister | (1 << pin);    // and OR bit in register
    } else {
        //
        // this is a LOW value so we have to AND it with 0 into the _valueRegister
        //
        _valueRegister = _valueRegister & ~(1 << pin);    // AND all bits
    }


  
  uint8_t buffer[2] = {NXP_OUTPUT_LOW,
                       _valueRegister_low,
                       };

  flag = write_then_read(buffer, 2, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time

  uint8_t bufferB[2] = {NXP_OUTPUT_HIGH,
                       _valueRegister_high,
                       };
  flag = write_then_read(bufferB, 2, nullptr, 0, true); // Write the time data


  
  return flag;                     
              

}




bool IO_PCA9539::write_then_read(const  uint8_t *write_buffer , int write_len,
                                        uint8_t *read_buffer  , int read_len,
                                        bool    stop) 
{
  Wire.beginTransmission(IO_PCA9539_ADDRESS);
  for (int i = 0; i < write_len; i++) {
    Wire.write(write_buffer[i]);
  }
  Wire.endTransmission(stop);

  if (read_len > 0) {
    Wire.requestFrom(IO_PCA9539_ADDRESS, read_len);
    for (int i = 0; i < read_len; i++) {
      if (Wire.available()) {
        read_buffer[i] = Wire.read();
      } else {
        return false; // Not enough data received
      }
    }//end    for 
  }
  return true;
}


/*
bool PCF85363A::write_then_read(const uint8_t *write_buffer, int write_len,
                                    uint8_t *read_buffer, int read_len,
                                    bool stop) {
  Wire.beginTransmission(PCF85363A_ADDRESS);
  for (int i = 0; i < write_len; i++) {
    Wire.write(write_buffer[i]);
  }
  Wire.endTransmission(stop);
  if (read_len > 0) {
    Wire.requestFrom(PCF85363A_ADDRESS, read_len);
    for (int i = 0; i < read_len; i++) {
      if (Wire.available()) {
        read_buffer[i] = Wire.read();
      } else {
        return false; // Not enough data received
      }
    }
  }
  return true;
}
*/

uint16_t IO_PCA9539::I2CGetValue(uint8_t address, uint8_t reg) {
    uint16_t _inputData;
    //
    // read the address input register
    //
    Wire.beginTransmission(address);          // setup read registers
    Wire.write(reg);
    _error = Wire.endTransmission();
    //
    // ask for 2 bytes to be returned
    //
    if (Wire.requestFrom((int)address, 1) != 1)
    {
        //
        // we are not receing the bytes we need
        //
        return 256;                            // error code is above normal data range
    };
    //
    // read both bytes
    //
    _inputData = Wire.read();
    return _inputData;
}




void IO_PCA9539::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value){
    Wire.beginTransmission(address);              // setup direction registers
    Wire.write(reg);                              // pointer to configuration register address 0
    Wire.write(value);                            // write config register low byte
   // _error = Wire.endTransmission(0);
    Wire.endTransmission();
    //_error = Wire.endTransmission();
}



// Start of Simple_TMP102   Definition /////////////////////////////////////////////
Simple_TMP102::Simple_TMP102() {}

boolean Simple_TMP102::begin() {

  i2c_restart_enable(&i2cwire0); // Enable I2C restart

  // Check if the device is connected
  byte x = 0;
  Wire.beginTransmission(TMP102_I2C_ADDR);
  Wire.write(x);
  if (Wire.endTransmission() != 0) {
    return false; // Device not found
  }

  // TMP102 only has 4 registers
  //  0x00: Temperature register
  //  0x01: Configuration register
  //  0x02: TLOW register
  //  0x03: THIGH register
  //  there is no signature register, so we cannot check if the device is a
  //  TMP102 Assume it is.

  return true;
}

float Simple_TMP102::checkTemperatureData() {
  Wire.beginTransmission(TMP102_I2C_ADDR);
  Wire.write(0x00); // Request temperature register
  Wire.endTransmission(false);

  Wire.requestFrom(TMP102_I2C_ADDR, 2); // Request 2 bytes for temperature
  if (Wire.available() < 2) {
    return NAN; // Not enough data received
  }

  int16_t rawTemp =
      (Wire.read() << 4) | (Wire.read() >> 4); // Read the two bytes
  float temperature = rawTemp * 0.0625;        // Convert to Celsius

  return temperature; // Return temperature in degrees
}


// Start of the definition of PCF85363A ///////////////////////////////////////////////////////////////
DateTime PCF85363A::outputNowDateTime(){
  DateTime currentRtcTime;

  // RTC clock runs
  while( !  this->begin()    ){
    Serial.println("PCF85363A init error !");
    //return (DateTime)null;
  };

  currentRtcTime  = this->now();
  return  currentRtcTime;
}//end    DateTime PCF85363A::outputNowDateTime


uint32_t PCF85363A::outputNowUnixtime(){
  DateTime currentRtcTime;
  uint32_t unixtime=0;
  
  while( !  this->begin()    ){
    Serial.println("PCF85363A init error !");
  };

  currentRtcTime  = this->now();      // directly fetch sec,min,hr from register(0x01h)
  unixtime        = currentRtcTime.unixtime();

  return unixtime;
}//end 


bool PCF85363A::clrPIN_IO_CLKPM(){
  uint8_t  buffer[1];
  uint8_t  result;

  buffer[0] = PCF85363A_PIN_IO;
  write_then_read(buffer, 1, buffer, 1, false);
  result =buffer[0];

  result = result & ~( 1<< BIT_PCF85363A_PIN_IO_CLKPM );

  // write here 
  bool    flag;
  uint8_t bufferA[2] = {PCF85363A_PIN_IO,
                       result,
                       };

  flag = write_then_read(bufferA, 2, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time
  return flag;  

}

 uint8_t  PCF85363A::read_second(void) {
  uint8_t  buffer[1];
  uint8_t  result;

  buffer[0] = PCF85363A_TIME;
  write_then_read(buffer, 1, buffer, 1, false);
  result =buffer[0];
  return result;
}  

bool PCF85363A::clr_OS(void) {
  uint8_t result;
  uint8_t buffer[1];

  buffer[0] = PCF85363A_TIME;
  write_then_read(buffer, 1, buffer, 1, false);
  result =buffer[0];

  /*
  return DateTime(bcd2bin(buffer[6]) + 2000U,   //year
                  bcd2bin(buffer[5] & 0x7F),    //month
                  //bcd2bin(buffer[4]),           //week of day 
                  bcd2bin(buffer[3] & 0x3F),    //day of month
                  bcd2bin(buffer[2]),           //hour         
                  bcd2bin(buffer[1]& 0x7F),     //min
                  bcd2bin(buffer[0] & 0x7F));   //sec
*/
  
  
  bool    flag;
  uint8_t data = result & 0x7f;   //BIT_OS

  uint8_t Sendbuffer[2] = { PCF85363A_TIME,
                            data,
                          };

  flag = write_then_read(Sendbuffer, 2, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time
  return flag; 
}

bool PCF85363A::battery_switch(byte data) {
  bool    flag;
  uint8_t buffer[2] = {PCF85363A_BATTERY_SWITCH,
                       data,
                       };

  flag = write_then_read(buffer, 2, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time
  return flag;                     
            
}

bool PCF85363A::function_pcf( byte data) {
  bool flag;
  uint8_t buffer[2] = {PCF85363A_FUNCTION,
                       data,
                       };

  flag = write_then_read(buffer, 2, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time
  return flag;                     
            
}


// stop: 1=stop the rtc   0=run the rtc
bool PCF85363A::stop_enable( byte stop) {
  bool flag;
  uint8_t buffer[2] = {PCF85363A_STOP_ENABLE,
                       stop,
                       };

  flag = write_then_read(buffer, 2, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time
  return flag;                     
            
}


boolean  PCF85363A::begin() {
  i2c_restart_enable(&i2cwire0);

//2Eh Stop-enable register
 if(!stop_enable(0x00)){  //0: rtc clock runs,    1:stop
  return false;
} 
  //28H  function register
 if(!function_pcf(0x00)){//0:100TH disabled 0:RTC   0:STOP(no IO-pin)  
  return false;
} 

    // Battery switch register
 if(!battery_switch(0x00)){
  return false;
} 

/*
  byte x = 0;
  Wire.beginTransmission(PCF85363A_ADDRESS);
  Wire.write(x); // Request time register
  if (Wire.endTransmission() != 0) {
    return false; // Device not found
  }
 */

  // the OS status bit is not accessible
  return true;
}

bool PCF85363A::write_then_read(const uint8_t *write_buffer, int write_len,
                                    uint8_t *read_buffer, int read_len,
                                    bool stop) {
  Wire.beginTransmission(PCF85363A_ADDRESS);
  for (int i = 0; i < write_len; i++) {
    Wire.write(write_buffer[i]);
  }
  Wire.endTransmission(stop);
  if (read_len > 0) {
    Wire.requestFrom(PCF85363A_ADDRESS, read_len);
    for (int i = 0; i < read_len; i++) {
      if (Wire.available()) {
        read_buffer[i] = Wire.read();
      } else {
        return false; // Not enough data received
      }
    }
  }
  return true;
}


float PCF85363A::getTemperature() {
  uint8_t buffer[2] = {DS3231_TEMPERATUREREG, 0};
  write_then_read(buffer, 1, buffer, 2, false);
  return (float)buffer[0] + (buffer[1] >> 6) * 0.25f;
}


bool PCF85363A::lostPower(void) {
  uint8_t statusReg = DS3231_STATUSREG;
  write_then_read(&statusReg, 1, &statusReg, 1, false);
  return (statusReg & 0x80) != 0; // Check the 32KHz output bit
}

void PCF85363A::clearLostPowerFlag(void) {
  uint8_t statusReg = DS3231_STATUSREG;
  write_then_read(&statusReg, 1, &statusReg, 1, false);
  statusReg &= ~0x80; // Clear the lost power flag
  uint8_t writeBuffer[2] = {DS3231_STATUSREG, statusReg};
  write_then_read(writeBuffer, 2, writeBuffer, 0,
                  true); // Write back the modified status register
}

void PCF85363A::adjust(const DateTime &dt) {
  uint8_t buffer[8] = {PCF85363A_TIME,
                       bin2bcd(dt.second()),
                       bin2bcd(dt.minute()),
                       bin2bcd(dt.hour()),
                       bin2bcd(dt.day()),                             // day of month
                       bin2bcd(dowToPCF85363A(dt.dayOfTheWeek())),    // Weekdays
                       bin2bcd(dt.month()),
                       bin2bcd(dt.year() - 2000U)};

  write_then_read(buffer, 8, nullptr, 0, true); // Write the time data
  //clearLostPowerFlag(); // Clear the lost power flag after adjusting time
}

DateTime PCF85363A::now() {
  uint8_t buffer[7];
  buffer[0] = PCF85363A_TIME;
  write_then_read(buffer, 1, buffer, 7, false);
  return DateTime(bcd2bin(buffer[6]) + 2000U,   //year
                  bcd2bin(buffer[5] & 0x7F),    //month
                  //bcd2bin(buffer[4]),           //week of day 
                  bcd2bin(buffer[3] & 0x3F),    //day of month
                  bcd2bin(buffer[2]),           //hour         
                  bcd2bin(buffer[1]& 0x7F),     //min
                  bcd2bin(buffer[0] & 0x7F));   //sec
}


// Start of the definition of DS3231 ///////////////////////////////////////////////////////////////
boolean Simple_DS3231::begin() {
  i2c_restart_enable(&i2cwire0); // Enable I2C restart

  // Check if the device is connected
  byte x = 0;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(x); // Request time register
  if (Wire.endTransmission() != 0) {
    return false; // Device not found
  }

  // DS3231 does not have a signature register to check,

  bool ds3231_lost_power = lostPower();
  if (ds3231_lost_power) {
    Serial.println("DS3231 lost power, Timer is not accurate.");
    // clearLostPowerFlag();  // Clear the lost power flag
  }

  // Assume the device is a DS3231 if we can communicate with it
  return true;
}




bool Simple_DS3231::write_then_read(const uint8_t *write_buffer, int write_len,
                                    uint8_t *read_buffer, int read_len,
                                    bool stop) {
  Wire.beginTransmission(DS3231_ADDRESS);
  for (int i = 0; i < write_len; i++) {
    Wire.write(write_buffer[i]);
  }
  Wire.endTransmission(stop);
  if (read_len > 0) {
    Wire.requestFrom(DS3231_ADDRESS, read_len);
    for (int i = 0; i < read_len; i++) {
      if (Wire.available()) {
        read_buffer[i] = Wire.read();
      } else {
        return false; // Not enough data received
      }
    }
  }
  return true;
}

float Simple_DS3231::getTemperature() {
  uint8_t buffer[2] = {DS3231_TEMPERATUREREG, 0};
  write_then_read(buffer, 1, buffer, 2, false);
  return (float)buffer[0] + (buffer[1] >> 6) * 0.25f;
}

bool Simple_DS3231::lostPower(void) {
  uint8_t statusReg = DS3231_STATUSREG;
  write_then_read(&statusReg, 1, &statusReg, 1, false);
  return (statusReg & 0x80) != 0; // Check the 32KHz output bit
}

void Simple_DS3231::clearLostPowerFlag(void) {
  uint8_t statusReg = DS3231_STATUSREG;
  write_then_read(&statusReg, 1, &statusReg, 1, false);
  statusReg &= ~0x80; // Clear the lost power flag
  uint8_t writeBuffer[2] = {DS3231_STATUSREG, statusReg};
  write_then_read(writeBuffer, 2, writeBuffer, 0,
                  true); // Write back the modified status register
}

void Simple_DS3231::adjust(const DateTime &dt) {
  uint8_t buffer[8] = {DS3231_TIME,
                       bin2bcd(dt.second()),
                       bin2bcd(dt.minute()),
                       bin2bcd(dt.hour()),
                       bin2bcd(dowToDS3231(dt.dayOfTheWeek())),
                       bin2bcd(dt.day()),
                       bin2bcd(dt.month()),
                       bin2bcd(dt.year() - 2000U)};

  write_then_read(buffer, 8, nullptr, 0, true); // Write the time data
  clearLostPowerFlag(); // Clear the lost power flag after adjusting time
}

DateTime Simple_DS3231::now() {
  uint8_t buffer[7];
  buffer[0] = DS3231_TIME;
  write_then_read(buffer, 1, buffer, 7, false);
  return DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5] & 0x7F),
                  bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                  bcd2bin(buffer[0] & 0x7F));
}


DateTime::DateTime(uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

  ss = t % 60;
  t /= 60;
  mm = t % 60;
  t /= 60;
  hh = t % 24;
  uint16_t days = t / 24;
  uint8_t leap;
  for (yOff = 0;; ++yOff) {
    leap = yOff % 4 == 0;
    if (days < 365U + leap)
      break;
    days -= 365 + leap;
  }
  for (m = 1; m < 12; ++m) {
    uint8_t daysPerMonth = daysInMonth[m - 1];
    if (leap && m == 2)
      ++daysPerMonth;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }
  d = days + 1;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
                   uint8_t min, uint8_t sec) {
  if (year >= 2000U)
    year -= 2000U;
  yOff = year;
  m = month;
  d = day;
  hh = hour;
  mm = min;
  ss = sec;
}

DateTime::DateTime(const DateTime &copy)
    : yOff(copy.yOff), m(copy.m), d(copy.d), hh(copy.hh), mm(copy.mm),
      ss(copy.ss) {}

bool DateTime::isValid() const {
  if (yOff >= 100)
    return false;
  DateTime other(unixtime());
  return yOff == other.yOff && m == other.m && d == other.d && hh == other.hh &&
         mm == other.mm && ss == other.ss;
}

String DateTime::timestamp() {
  String ts;
  ts.reserve(20); // Reserve space for the timestamp string
  ts += String(year());
  ts += '-';
  ts += String(month() < 10 ? "0" : "") + String(month());
  ts += '-';
  ts += String(day() < 10 ? "0" : "") + String(day());
  ts += 'T';
  ts += String(hour() < 10 ? "0" : "") + String(hour());
  ts += ':';
  ts += String(minute() < 10 ? "0" : "") + String(minute());
  ts += ':';
  ts += String(second() < 10 ? "0" : "") + String(second());
  return ts;
}

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000U)
    y -= 2000U;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += (daysInMonth[i - 1]);
  if (m > 2 && y % 4 == 0)
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}

uint8_t DateTime::dayOfTheWeek() const {
  uint16_t day = date2days(yOff, m, d);
  return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

static uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24UL + h) * 60 + m) * 60 + s;
}

uint32_t DateTime::unixtime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2ulong(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000

  return t;
}

boolean Simple_EEPROM_AT24C08::begin() {
  i2c_restart_enable(&i2cwire0); // Enable I2C restart

  // Check if the device is connected
  byte x = 0;
  Wire.beginTransmission(EEPROM_START_ADDRESS);
  Wire.write(x);
  if (Wire.endTransmission() != 0) {
    return false; // Device not found
  }

  return true; // Device is ready
}

#define WIRE_ACCESS_LEN 128
int Simple_EEPROM_AT24C08::read(uint16_t memAddress, uint8_t *data,
                                uint16_t length) {
  if (length == 0 || data == nullptr) {
    return 0; // Nothing to read
  }

  if (memAddress >= 1024) {
    return 0; // Invalid memory address
  }

  int writeIndex = 0;
  while (length > 0) {
    uint16_t toRead = length;
    if (toRead > (WIRE_ACCESS_LEN / 2)) {
      toRead = (WIRE_ACCESS_LEN / 2); // Limit to WIRE_ACCESS_LEN/2
    }
    uint8_t readAddress =
        EEPROM_START_ADDRESS + (memAddress >> 8); // High byte of address

    // Set the memory address to read from
    Wire.beginTransmission(readAddress);
    Wire.write((uint8_t)(memAddress &
                         0xFF)); // Low byte only as high byte is in the address
    Wire.endTransmission(false); // Send repeated start
    Wire.requestFrom((uint8_t)readAddress, (uint8_t)toRead);
    for (uint16_t i = 0; i < toRead && Wire.available(); i++) {
      data[writeIndex++] = Wire.read();
    }

    // Update remaining length and memory address
    length -= toRead;
    memAddress += toRead;
  }

  return writeIndex;
}

int Simple_EEPROM_AT24C08::write(uint16_t memAddress, uint8_t *data,
                                 uint16_t length) {
  if (length == 0 || data == nullptr) {
    return 0; // Nothing to write
  }

  if (memAddress >= 1024) {
    return 0; // Invalid memory address
  }

  int writtenLength = 0;
  while (length > 0) {
    // we can only write one page at a time
    uint16_t endAddress = memAddress + length;
    if (((endAddress - 1) & (~0x0F)) != (memAddress & (~0x0F))) {
      // if the end address is not in the same page as the start address
      endAddress = (memAddress | 0x0F) + 1;
    }
    int writeLength = endAddress - memAddress;
    // writeLength is at most 16 bytes, no need to check for WIRE_ACCESS_LEN
    uint8_t writeAddress =
        EEPROM_START_ADDRESS + (memAddress >> 8); // High byte of address
    Wire.beginTransmission(writeAddress);
    Wire.write((uint8_t)(memAddress &
                         0xFF)); // Low byte only as high byte is in the address
    for (int i = 0; i < writeLength; i++) {
      Wire.write(data[writtenLength++]); // Write data
    }
    if (Wire.endTransmission() != 0) {
      return writtenLength; // Return how many bytes were written before failure
    }

    // Use retry to do a delay
    for (int retry = 0; retry < 300; retry++) {
      Wire.beginTransmission(EEPROM_START_ADDRESS);
      byte x = 0;
      Wire.write(x);
      if (Wire.endTransmission() == 0) {
        break; // Exit retry loop on success
      }
    }

    memAddress += writeLength;
    length -= writeLength;
  }

  return writtenLength;
}
