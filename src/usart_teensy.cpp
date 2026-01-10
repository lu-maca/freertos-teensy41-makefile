#include "arduino_freertos.h"
#include <csp/arch/csp_semaphore.h>
#include <csp/drivers/usart_teensy.h>

static csp_mutex_t serial_mx;
static csp_mutex_t hw_serial_mx;

int teensy_usart_read(uint8_t *cbuf, const unsigned int length) {
  int out = 0;
  csp_mutex_lock(&hw_serial_mx, 5);
  int available = Serial1.available();
  csp_mutex_unlock(&hw_serial_mx);
  
  if ((unsigned int)available <= length) {
    csp_mutex_lock(&hw_serial_mx, 5);
    out =  Serial1.readBytes(cbuf, available);
    csp_mutex_unlock(&hw_serial_mx);
  }
  return out;
}

void teensy_usart_open(const int baudrate) {
  Serial1.begin(baudrate);
  csp_mutex_create(&hw_serial_mx);
}

int teensy_usart_write(const uint8_t *data, const unsigned int data_length) {
  csp_mutex_lock(&hw_serial_mx, 5);
  int written = Serial1.write(data, data_length);
  csp_mutex_unlock(&hw_serial_mx);
  return written;
}

void teensy_serial_setup(const unsigned int baud) {
  Serial.begin(baud);
  csp_mutex_create(&serial_mx);
}

void teensy_print(const char *s) {
  csp_mutex_lock(&serial_mx, 20);
  Serial.println(s);
  csp_mutex_unlock(&serial_mx);
}

void teensy_print_char(const char s) {
  csp_mutex_lock(&serial_mx, 20);
  Serial.println(s);
  csp_mutex_unlock(&serial_mx);
}

void teensy_print_int(int s) {
  csp_mutex_lock(&serial_mx, 20);
  Serial.println(s, arduino::DEC);
  csp_mutex_unlock(&serial_mx);
}