#include "log.h"

_LogLine::_LogLine(int level) {
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

_LogLine::~_LogLine() { std::clog << '\n'; }