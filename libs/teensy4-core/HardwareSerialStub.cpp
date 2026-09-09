/*
 * HardwareSerialStub.cpp
 *
 * Compatibility shim required ONLY because HardwareSerial.cpp (and
 * HardwareSerial1..8.cpp) were removed from the build as part of handing
 * all LPUART ownership to DmaSerial.
 *
 * HardwareSerial.h declares HardwareSerialIMXRT::processSerialEventsList()
 * as a static inline function (so it can be called cheaply from yield()):
 *
 *   static inline void processSerialEventsList() {
 *       for (uint8_t i = 0; i < s_count_serials_with_serial_events; i++) {
 *           s_serials_with_serial_events[i]->doYieldCode();
 *       }
 *   }
 *
 * yield.cpp calls this unconditionally in its translation unit (gated at
 * runtime behind the YIELD_CHECK_HARDWARE_SERIAL bit, which is only ever
 * set by HardwareSerialIMXRT::addToSerialEventsList() - i.e. only if some
 * code actually calls Serial1..Serial7/8.begin()). Since DmaSerial never
 * calls begin() on any HardwareSerialIMXRT instance, that bit is never set
 * and this loop never runs - but the two static data members it references
 * are still ODR-used from yield.cpp's TU, so the linker needs a definition
 * of them to exist *somewhere* in the program. Previously that was
 * HardwareSerial.cpp. Now it's here.
 *
 * If HardwareSerial.cpp is ever restored, delete this file - it would
 * define these symbols twice.
 */

#include "HardwareSerial.h"

#if defined(ARDUINO_TEENSY41)
HardwareSerialIMXRT *HardwareSerialIMXRT::s_serials_with_serial_events[8];
#else
HardwareSerialIMXRT *HardwareSerialIMXRT::s_serials_with_serial_events[7];
#endif

uint8_t HardwareSerialIMXRT::s_count_serials_with_serial_events = 0;
