#include "uart.h"
#include "drivers/device_tree.h"

namespace drivers
{

void uart_init()
{
  auto uart1 = std::make_shared<Uart>(dmaSerial1, "serial1");
  DeviceTree::add("/dev/serial1", uart1);
}

void Uart::begin(uint32_t baud, uint16_t format)
{
  serial_.begin(baud, format);
}

int Uart::unsafeAvailable()
{
  return serial_.available();
}

int Uart::unsafeRead()
{
  return serial_.read();
}

size_t Uart::unsafeReadBytes(char* buffer, size_t length)
{
  return serial_.readBytes(buffer, length);
}

size_t Uart::unsafeWrite(const uint8_t* buffer, size_t size)
{
  return serial_.write(buffer, size);
}

int Uart::available()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return unsafeAvailable();
}

int Uart::read()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return unsafeRead();
}

size_t Uart::readBytes(char* buffer, size_t length)
{
  std::lock_guard<std::mutex> lock(mutex_);
  return unsafeReadBytes(buffer, length);
}

size_t Uart::write(const uint8_t* buffer, size_t size)
{
  std::lock_guard<std::mutex> lock(mutex_);
  return unsafeWrite(buffer, size);
}

void Uart::setRxTaskHandle(TaskHandle_t handle)
{
  serial_.setRxTaskHandle(handle);
}

}  // namespace drivers