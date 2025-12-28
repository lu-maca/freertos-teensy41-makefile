#include <csp/drivers/usart_teensy.h>
#include "arduino_freertos.h"


int teensy_usart_read(uint8_t * cbuf, const unsigned int length) {
    const int available = Serial1.available();
    if (available <= 400) {
        return Serial1.readBytes(cbuf, length);
    }
    return 0;
}

int teensy_usart_write(const uint8_t * data, const unsigned int data_length) {
    return Serial1.write(data, data_length);
}

void teensy_print(const char* s) {
    Serial.println(s);
}

void teensy_print_int(int s) {
    Serial.println(s);
}