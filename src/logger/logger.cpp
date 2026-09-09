#include "logger/logger.h"
#include "arduino_freertos.h"
#include "logger/priv/mycdebug.h"

#include <cstdarg>

namespace logger
{

    void setup([[maybe_unused]] int level)
    {
        Serial.begin(115200);
    }

    void debug(const char* format, ...)
    {
        Serial.print("[debug   ] ");

        va_list args;
        va_start(args, format);
        vcdebug(format, args);
        va_end(args);

        Serial.println();
    }

    void info(const char* format, ...)
    {
        Serial.print("[info    ] ");

        va_list args;
        va_start(args, format);
        vcdebug(format, args);
        va_end(args);

        Serial.println();
    }

    void warning(const char* format, ...)
    {
        Serial.print("[warning ] ");

        va_list args;
        va_start(args, format);
        vcdebug(format, args);
        va_end(args);

        Serial.println();
    }

    void error(const char* format, ...)
    {
        Serial.print("[error   ] ");

        va_list args;
        va_start(args, format);
        vcdebug(format, args);
        va_end(args);

        Serial.println();
    }
}  // namespace logger
