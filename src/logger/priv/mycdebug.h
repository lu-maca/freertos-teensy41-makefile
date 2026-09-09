#pragma once
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void cdebug(const char* format, ...);
    void vcdebug(const char* format, va_list args);

#ifdef __cplusplus
}
#endif
