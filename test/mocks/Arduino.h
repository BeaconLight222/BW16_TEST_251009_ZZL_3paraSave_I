#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

using std::abs;
using std::memcpy;

#ifndef F
#define F(x) x
#endif

#ifndef HEX
#define HEX std::hex
#endif

#ifndef DEC
#define DEC std::dec
#endif

#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW 0x0
#endif

#ifndef INPUT
#define INPUT 0x0
#endif

#ifndef OUTPUT
#define OUTPUT 0x1
#endif

using byte = uint8_t;
using boolean = bool;

class MockSerial {
public:
  template <typename T>
  MockSerial& print(T value) {
    return *this;
  }

  template <typename T>
  MockSerial& print(T value, std::ios_base& (*)(std::ios_base&)) {
    return *this;
  }

  template <typename T>
  MockSerial& println(T value) {
    return *this;
  }

  MockSerial& println() {
    return *this;
  }
};

inline MockSerial Serial;

inline unsigned long millis() {
  return 0;
}

inline void delay(unsigned long) {}

inline void pinMode(uint8_t, uint8_t) {}

inline void digitalWrite(uint8_t, uint8_t) {}

inline uint8_t digitalRead(uint8_t) {
  return 0;
}

class String {
public:
  String() = default;
  String(const char* value) : data_(value ? value : "") {}
  String(int value) : data_(std::to_string(value)) {}
  String(unsigned int value) : data_(std::to_string(value)) {}
  String(long value) : data_(std::to_string(value)) {}
  String(unsigned long value) : data_(std::to_string(value)) {}
  String(float value) : data_(std::to_string(value)) {}
  String(double value) : data_(std::to_string(value)) {}

  String operator+(const String& other) const {
    return String((data_ + other.data_).c_str());
  }

  String& operator=(const char* value) {
    data_ = value ? value : "";
    return *this;
  }

  const char* c_str() const {
    return data_.c_str();
  }

  friend String operator+(const char* lhs, const String& rhs) {
    return String((std::string(lhs) + rhs.data_).c_str());
  }

private:
  std::string data_;
};
