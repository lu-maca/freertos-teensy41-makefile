#pragma once
#include "arduino_freertos.h"

namespace logger
{

enum class level
{
  debug = 0,
  info,
  warning,
  error,
  disable
};

void init(const level lev);

void debug(const char* format, ...);

void info(const char* format, ...);

void warning(const char* format, ...);

void error(const char* format, ...);
}  // namespace logger
