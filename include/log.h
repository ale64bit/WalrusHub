#pragma once

#include <iostream>

#define INFO 0
#define WARN 1
#define ERROR 2

// silly, just to add newlines automatically after logging statements
struct _LogLine {
  _LogLine(int level) {
    switch (level) {
      case INFO:
        std::clog << "\033[0;32m[INFO] ";
        break;
      case WARN:
        std::clog << "\033[0;33m[WARN] ";
        break;
      case ERROR:
        std::clog << "\033[0;31m[ERROR] ";
        break;
    }
  }

  ~_LogLine() { std::clog << '\n'; }

  template <typename T>
  _LogLine& operator<<(const T& t) {
    std::clog << t;
    return *this;
  }
};

#define LOG(LEVEL) _LogLine(LEVEL) << __FILE__ << ":" << __LINE__ << ": "