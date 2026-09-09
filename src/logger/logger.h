#pragma once

namespace logger
{

    void setup([[maybe_unused]] int level);

    void debug(const char* format, ...);

    void info(const char* format, ...);

    void warning(const char* format, ...);

    void error(const char* format, ...);
}  // namespace logger
