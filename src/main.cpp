
#include "arduino_freertos.h"
#include "csp/drivers/usart.h"
#include "csp/csp.h"
#include "drivers/uart/uart.h"

static void task1(void *) {
  pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
  while (true) {
    digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));

    digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  vTaskDelete(nullptr);
}

/// OBDH stuff
#ifndef CSP_ROUTER_TASK_PRIO
#define CSP_ROUTER_TASK_PRIO (configMAX_PRIORITIES - 3)
#endif

#ifndef CSP_ADDR 
#define CSP_ADDR 15
#endif

#ifndef CSP_ROUTER_STACK_DEPTH
#define CSP_ROUTER_STACK_DEPTH      2048
#endif

static void csp_route_task([[maybe_unused]] void *args) {
  for(;;)
	{
		csp_route_work();
	}
}

int obdh_init(int address) {
  // initialize drivers
  drivers::uart_init();

  csp_iface_t *iface;
  csp_usart_conf_t csp_conf = {
      .device = "/dev/serial1",
      .baudrate = 115200,
      .databits = 8,
      .stopbits = 1,
      .paritysetting = 0,
  };
  auto ret = csp_usart_open_and_add_kiss_interface(&csp_conf, "serial1", address, &iface);
  if (ret == CSP_ERR_NONE) {
    return ret;
  }
  iface->is_default = 1;

  static StaticTask_t csp_rtr_tcb;
	StackType_t *rtr_stk_ptr;

	rtr_stk_ptr = static_cast<StackType_t*>(malloc(CSP_ROUTER_STACK_DEPTH*sizeof(StackType_t)));
	if (!rtr_stk_ptr) {
		return CSP_ERR_NOMEM;
	}

  TaskHandle_t cr_task_ret = xTaskCreateStatic(csp_route_task, "csp_router", 2048,
	    NULL, CSP_ROUTER_TASK_PRIO, rtr_stk_ptr, &csp_rtr_tcb);

  if (cr_task_ret == NULL) {
		return CSP_ERR_NOMEM;
	}

  return CSP_ERR_NONE;
}

int main() {

  xTaskCreate(task1, "task1", 128, nullptr, 2, nullptr);
  // start obdh
  obdh_init(CSP_ADDR);

  vTaskStartScheduler();

  // never reach here
  while (1) {
  }
}
