#pragma once

#include <iostream>

#define INFO 0
#define WARN 1
#define ERROR 2

// silly, just to add newlines automatically after logging statements
struct _LogLine final {
  _LogLine(int level);
  ~_LogLine();

  template <typename T>
  _LogLine& operator<<(const T& t) {
    std::clog << t;
    return *this;
  }
};

#define LOG(LEVEL) _LogLine(LEVEL) << __FILE__ << ":" << __LINE__ << ": "