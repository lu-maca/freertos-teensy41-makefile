
#include "arduino_freertos.h"
#include "logger/logger.h"
#include "drivers/uart/uart.h"
#include "obdh/obdh.h"

static void task1(void*)
{
    pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    while (true)
    {
        digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));

        digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(nullptr);
}

static void obdhInitTask(void*)
{
    obdh_init(CSP_ADDR);

    vTaskDelete(nullptr);
}

int main()
{

    logger::setup(1);

    xTaskCreate(task1, "task1", 128, nullptr, 9, nullptr);
    xTaskCreate(obdhInitTask, "obdhInit", 512, nullptr, 9, nullptr);

    logger::info("Starting scheduler");

    vTaskStartScheduler();

    // never reach here
    while (1)
    {
    }
}
