#include "logger/logger.h"
#include "logger/priv/mycdebug.h"

#include <cstdarg>

namespace logger
{

static level log_level;

void init(const level lev)
{
  log_level = lev;
  Serial.begin(115200);
}

void debug(const char* format, ...)
{
  if (log_level == level::debug)
  {
    Serial.print("[debug   ] ");

    va_list args;
    va_start(args, format);
    vcdebug(format, args);
    va_end(args);

    Serial.println();
  }
}

void info(const char* format, ...)
{
  if (log_level <= level::info)
  {
    Serial.print("[info    ] ");

    va_list args;
    va_start(args, format);
    vcdebug(format, args);
    va_end(args);

    Serial.println();
  }
}

void warning(const char* format, ...)
{
  if (log_level <= level::warning)
  {
    Serial.print("[warning ] ");

    va_list args;
    va_start(args, format);
    vcdebug(format, args);
    va_end(args);

    Serial.println();
  }
}

void error(const char* format, ...)
{
  if (log_level <= level::error)
  {
    Serial.print("[error   ] ");

    va_list args;
    va_start(args, format);
    vcdebug(format, args);
    va_end(args);

    Serial.println();
  }
}
}  // namespace logger
