#pragma once

#include <memory>
#include <mutex>
#include "DmaSerial.h"
#include "drivers/device.h"

namespace drivers {

class Uart : public Device {
  DmaSerial& serial_;

public:
  Uart(DmaSerial& serial, const std::string& name)
      : Device(name), serial_{serial} {};

  void begin(uint32_t baud, uint16_t format = 0);
  
  [[nodiscard]] int available();

  [[nodiscard]] int read();

  [[nodiscard]] size_t readBytes(char *buffer, size_t length);

  [[nodiscard]] size_t write(const uint8_t *buffer, size_t size);

  [[nodiscard]] int unsafeAvailable();

  [[nodiscard]] int unsafeRead();

  [[nodiscard]] size_t unsafeReadBytes(char *buffer, size_t length);

  [[nodiscard]] size_t unsafeWrite(const uint8_t *buffer, size_t size);


};

void uart_init(/* add name of the interface to be initialized */);

} // namespace drivers