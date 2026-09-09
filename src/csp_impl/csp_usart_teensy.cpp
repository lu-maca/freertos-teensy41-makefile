#include <array>
#include "arduino_freertos.h"
#include "csp/drivers/usart.h"  // here we implement this interface
#include "drivers/device_tree.h"
#include "drivers/uart/uart.h"
#include "logger/logger.h"

#ifndef CSP_RX_TASK_PRIO
#define CSP_RX_TASK_PRIO (configMAX_PRIORITIES - 4)
#endif

typedef struct
{
  csp_usart_callback_t rx_callback;
  void* user_data;
  csp_usart_fd_t fd;
  TaskHandle_t rx_task_handle;
} usart_context_t;

static std::shared_ptr<drivers::Uart> uart_csp;

static void usart_rx_thread(void* arg)
{
  constexpr size_t RCV_SIZE = 256;
  [[maybe_unused]] auto* ctx = static_cast<usart_context_t*>(arg);
  std::array<uint8_t, RCV_SIZE> received;
  int nbytes = 0;

  while (true)
  {
    // wait for event notification from dma
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    while ((nbytes = uart_csp->available()) > 0)
    {
      size_t to_read = std::min<size_t>(nbytes, RCV_SIZE);
      auto read = uart_csp->readBytes(reinterpret_cast<char*>(received.data()), to_read);
      ctx->rx_callback(ctx->user_data, received.data(), read, NULL);
    }
  }
}

int csp_usart_write([[maybe_unused]] csp_usart_fd_t fd, const void* data, size_t data_length)
{
  auto buf = static_cast<const uint8_t*>(data);
  return uart_csp->unsafeWrite(buf, data_length);
}

void csp_usart_lock(void* driver_data)
{
  std::mutex& mx = uart_csp->mutex();
  mx.lock();
}

void csp_usart_unlock(void* driver_data)
{
  std::mutex& mx = uart_csp->mutex();
  mx.unlock();
}

int csp_usart_open(const csp_usart_conf_t* conf, csp_usart_callback_t rx_callback, void* user_data, csp_usart_fd_t* fd)
{
  if (rx_callback == nullptr)
  {
    return CSP_ERR_INVAL;
  }
  // search for the device
  const std::string dev_name{conf->device};
  auto devices = drivers::DeviceTree::registered();
  // check if it's initialized AND is a uart
  // !TODO check how to call `and std::dynamic_pointer_cast<drivers::Uart>(devices[dev_name]))`
  if (not(devices.contains(dev_name)))
  {
    return CSP_ERR_INVAL;
  }
  uart_csp = std::static_pointer_cast<drivers::Uart>(devices[dev_name]);

  // initialize the context
  usart_context_t* ctx = static_cast<usart_context_t*>(calloc(1, sizeof(*ctx)));
  if (ctx == NULL)
  {
    return CSP_ERR_NOMEM;
  }
  ctx->rx_callback = rx_callback;
  ctx->user_data = user_data;
  ctx->fd = *fd;

  // initialize the rx thread
  xTaskCreate(usart_rx_thread, "csp_uart_rx", 8192, ctx, CSP_RX_TASK_PRIO, &ctx->rx_task_handle);

  *fd = static_cast<csp_usart_fd_t>(uart_csp->fd());

  uart_csp->setRxTaskHandle(ctx->rx_task_handle);
  // !TODO handle parity and stopbits
  uart_csp->begin(conf->baudrate);

  return CSP_ERR_NONE;
}
