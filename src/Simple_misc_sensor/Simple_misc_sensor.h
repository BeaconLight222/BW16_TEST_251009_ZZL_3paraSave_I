#ifndef SIMPLE_MISC_SENSOR_H
#define SIMPLE_MISC_SENSOR_H
#include <Arduino.h>
#include <Wire.h>


//NXP IO-Expander system address is 0x77 |||||||||||||||||||||||||||||||||||||
#define IO_PCA9539_ADDRESS 0x77

#define BALLAST_CONTROL_SIGNAL   AMB_D12
#define LOGIC_HIGH          1
#define LOGIC_LOW           0
#define MODE_OUTPUT   0x00
#define MODE_INPUT    0x01

#define MODE_OUTPUT   0x00
#define MODE_INPUT    0x01


enum {
    PIN0=0,PIN1, PIN2, PIN3, PIN4, PIN5, PIN6, PIN7,
    PIN8,PIN9,PIN10,PIN11,PIN12,PIN13,PIN14,PIN15,
};

// PCA9555 defines
#define NXP_INPUT_LOW       0
#define NXP_INPUT_HIGH      1
#define NXP_OUTPUT_LOW      2
#define NXP_OUTPUT_HIGH     3
#define NXP_INVERT_LOW      4
#define NXP_INVERT_HIGH     5
#define NXP_CONFIG_LOW      6
#define NXP_CONFIG_HIGH     7


#define NXP_INPUT      0
#define NXP_OUTPUT     2
#define NXP_INVERT     4
#define NXP_CONFIG     6


class IO_PCA9539 {

  
public:
    //void  output_PIN12(bool flag);
   // void ballastPower(bool flag);
    void fanControl(bool flag);
    void fanOn();
    void fanOff();
    void allOutputLow();
    void allOutputHigh();    
    void allOutputPin();
    bool begin();
    //PCA9539(uint8_t address);                            // constructor
    void pinMode(uint8_t pin, uint8_t IOMode );          // pinMode
    uint8_t digitalRead(uint8_t pin);                    // digitalRead
    bool digitalWrite(uint8_t pin, uint8_t value );      // digitalWrite
    //void digitalWrite(uint8_t pin, uint8_t value );      // digitalWrite
      bool write_then_read(const uint8_t *write_buffer, int write_len,
                       uint8_t *read_buffer, int read_len, bool stop);

private:
    //
    // low level methods
    //
    uint16_t I2CGetValue(uint8_t address, uint8_t reg);
    void I2CSetValue(uint8_t address, uint8_t reg, uint8_t value);

    union {
        struct {
            uint8_t _configurationRegister_low;          // low order byte
            uint8_t _configurationRegister_high;         // high order byte
        };
        uint16_t _configurationRegister;                 // 16 bits presentation
    };
    union {
        struct {
            uint8_t _valueRegister_low;                  // low order byte
            uint8_t _valueRegister_high;                 // high order byte
        };
        uint16_t _valueRegister;
    };
    uint8_t _address;                                    // address of port this class is supporting
    byte _error;                                         // error code from I2C

};


// TMP102 AD0 pin is connected to GND, so the I2C address is 0x48 ||||||||||||||
#define TMP102_I2C_ADDR 0x48

class Simple_TMP102 {
public:
  Simple_TMP102();
  boolean begin();
  float checkTemperatureData();
};

// from datasheet
#define PCF85363A_ADDRESS 0x51  // I2C address for PCF85363A

#define PCF85363A_TIME    0x01  // Time second register
#define PCF85363A_ALARM1   0x08  // Alarm 1 register
#define PCF85363A_ALARM2   0x0D  // Alarm 2 register
#define PCF85363A_CONTROL
#define PCF85363A_STATUSREG
#define PCF85363A_TEMPERATUREREG
#define PCF85363A_BATTERY_SWITCH  0x26
#define PCF85363A_PIN_IO          0x27
#define PCF85363A_FUNCTION        0x28
#define PCF85363A_STOP_ENABLE 0x2E  // b0: monitor RTC clcok stop=1; running=0

#define BIT_PCF85363A_PIN_IO_CLKPM  7
#define BIT_PCF85363A_PIN_IO_TSPM   3

// from adafruit RTClib
#define DS3231_ADDRESS 0x68   ///< I2C address for DS3231
#define DS3231_TIME 0x00      ///< Time register
#define DS3231_ALARM1 0x07    ///< Alarm 1 register
#define DS3231_ALARM2 0x0B    ///< Alarm 2 
#define DS3231_CONTROL 0x0E   ///< Control register
#define DS3231_STATUSREG 0x0F ///< Status register
#define DS3231_TEMPERATUREREG                                                  \
  0x11 ///< Temperature register (high byte - low byte is at 0x12), 10-bit
       ///< temperature value

/** Constants */
#define SECONDS_PER_DAY 86400L ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000                                              \
  946684800 ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

class TimeSpan;
class DateTime {
public:
  DateTime(uint32_t t = SECONDS_FROM_1970_TO_2000);
  DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0,
           uint8_t min = 0, uint8_t sec = 0);
  DateTime(const DateTime &copy);
  bool isValid() const;
  String timestamp();

  /*!
      @brief  Return the year.
      @return Year (range: 2000--2099).
  */
  uint16_t year() const { return 2000U + yOff; }
  /*!
      @brief  Return the month.
      @return Month number (1--12).
  */
  uint8_t month() const { return m; }
  /*!
      @brief  Return the day of the month.
      @return Day of the month (1--31).
  */
  uint8_t day() const { return d; }
  /*!
      @brief  Return the hour
      @return Hour (0--23).
  */
  uint8_t hour() const { return hh; }

  uint8_t twelveHour() const;
  /*!
      @brief  Return whether the time is PM.
      @return 0 if the time is AM, 1 if it's PM.
  */
  uint8_t isPM() const { return hh >= 12; }
  /*!
      @brief  Return the minute.
      @return Minute (0--59).
  */
  uint8_t minute() const { return mm; }
  /*!
      @brief  Return the second.
      @return Second (0--59).
  */
  uint8_t second() const { return ss; }

  uint8_t dayOfTheWeek() const;

  /* 32-bit times as seconds since 1970-01-01. */
  uint32_t unixtime(void) const;

  DateTime operator+(const TimeSpan &span) const;
  DateTime operator-(const TimeSpan &span) const;
  TimeSpan operator-(const DateTime &right) const;
  bool operator<(const DateTime &right) const;

  /*!
      @brief  Test if one DateTime is greater (later) than another.
      @warning if one or both DateTime objects are invalid, returned value is
        meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the left DateTime is later than the right one,
        false otherwise
  */
  bool operator>(const DateTime &right) const { return right < *this; }

  /*!
      @brief  Test if one DateTime is less (earlier) than or equal to another
      @warning if one or both DateTime objects are invalid, returned value is
        meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the left DateTime is earlier than or equal to the
        right one, false otherwise
  */
  bool operator<=(const DateTime &right) const { return !(*this > right); }

  /*!
      @brief  Test if one DateTime is greater (later) than or equal to another
      @warning if one or both DateTime objects are invalid, returned value is
        meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the left DateTime is later than or equal to the right
        one, false otherwise
  */
  bool operator>=(const DateTime &right) const { return !(*this < right); }
  bool operator==(const DateTime &right) const;

  /*!
      @brief  Test if two DateTime objects are not equal.
      @warning if one or both DateTime objects are invalid, returned value is
        meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the two objects are not equal, false if they are
  */
  bool operator!=(const DateTime &right) const { return !(*this == right); }

protected:
  uint8_t yOff; ///< Year offset from 2000
  uint8_t m;    ///< Month 1-12
  uint8_t d;    ///< Day 1-31
  uint8_t hh;   ///< Hours 0-23
  uint8_t mm;   ///< Minutes 0-59
  uint8_t ss;   ///< Seconds 0-59
};

class TimeSpan {
public:
  TimeSpan(int32_t seconds = 0);
  TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
  TimeSpan(const TimeSpan &copy);

  /*!
      @brief  Number of days in the TimeSpan
              e.g. 4
      @return int16_t days
  */
  int16_t days() const { return _seconds / 86400L; }
  /*!
      @brief  Number of hours in the TimeSpan
              This is not the total hours, it includes the days
              e.g. 4 days, 3 hours - NOT 99 hours
      @return int8_t hours
  */
  int8_t hours() const { return _seconds / 3600 % 24; }
  /*!
      @brief  Number of minutes in the TimeSpan
              This is not the total minutes, it includes days/hours
              e.g. 4 days, 3 hours, 27 minutes
      @return int8_t minutes
  */
  int8_t minutes() const { return _seconds / 60 % 60; }
  /*!
      @brief  Number of seconds in the TimeSpan
              This is not the total seconds, it includes the days/hours/minutes
              e.g. 4 days, 3 hours, 27 minutes, 7 seconds
      @return int8_t seconds
  */
  int8_t seconds() const { return _seconds % 60; }
  /*!
      @brief  Total number of seconds in the TimeSpan, e.g. 358027
      @return int32_t seconds
  */
  int32_t totalseconds() const { return _seconds; }

  TimeSpan operator+(const TimeSpan &right) const;
  TimeSpan operator-(const TimeSpan &right) const;

protected:
  int32_t _seconds; ///< Actual TimeSpan value is stored as seconds
};


class PCF85363A {
public:
  DateTime outputNowDateTime();

  uint32_t outputNowUnixtime();
  bool clrPIN_IO_CLKPM();
  uint8_t read_second(void);
  uint8_t s(void);
  bool clr_OS(void);
  bool battery_switch(byte data);
  bool function_pcf( byte data);

  bool stop_enable( byte stop);
  boolean begin();
  void adjust(const DateTime &dt);
  bool lostPower(void);
  void clearLostPowerFlag(void);
  DateTime now();

  float getTemperature(); // in Celsius degree
  bool write_then_read(const uint8_t *write_buffer, int write_len,
                       uint8_t *read_buffer, int read_len, bool stop);
  
  static uint8_t dowToPCF85363A(uint8_t d) { return d == 0 ? 7 : d; }
  static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
  static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

};


class Simple_DS3231 {
public:
  boolean begin();
  void adjust(const DateTime &dt);
  bool lostPower(void);
  void clearLostPowerFlag(void);
  DateTime now();

  float getTemperature(); // in Celsius degree
  bool write_then_read(const uint8_t *write_buffer, int write_len,
                       uint8_t *read_buffer, int read_len, bool stop);
  /*!
      @brief  Convert the day of the week to a representation suitable for
              storing in the DS3231: from 1 (Monday) to 7 (Sunday).
      @param  d Day of the week as represented by the library:
              from 0 (Sunday) to 6 (Saturday).
      @return the converted value
  */
  static uint8_t dowToDS3231(uint8_t d) { return d == 0 ? 7 : d; }
  static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
  static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }
};

#define EEPROM_START_ADDRESS 0x50

class Simple_EEPROM_AT24C08 {
public:
  boolean begin();
  int read(uint16_t memAddress, uint8_t *data, uint16_t length);
  int write(uint16_t memAddress, uint8_t *data, uint16_t length);
};

#endif
