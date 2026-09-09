#include "logger/priv/mycdebug.h"
#include <stdarg.h>
#include <stdio.h>
#include "arduino_freertos.h"

extern "C" void vcdebug(const char* format, va_list args)
{
    char buffer[128];

    vsnprintf(buffer, sizeof(buffer), format, args);

    Serial.print(buffer);
}

extern "C" void cdebug(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vcdebug(format, args);
    va_end(args);
}
