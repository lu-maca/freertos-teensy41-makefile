#include "obdh/obdh.h"
#include "HardwareSerial.h"
#include "drivers/uart/uart.h"
#include "logger/logger.h"

static void csp_route_task([[maybe_unused]] void* args)
{
    for (;;)
    {
        csp_route_work();
    }
}

/// @brief csp_packets_dispatcher_task() is a task that binds to all available
///        ports and dispatches packets to correct services
static void csp_packets_dispatcher_task([[maybe_unused]] void* args)
{
    csp_socket_t sock = {0};

    csp_bind(&sock, CSP_ANY);
    csp_listen(&sock, 10);

    while (1)
    {
        csp_conn_t* conn = csp_accept(&sock, CSP_MAX_DELAY);

        csp_packet_t* packet;
        while ((packet = csp_read(conn, CSP_MAX_DELAY)) != nullptr)
        {
            switch (csp_conn_dport(conn))
            {
                default:
                    csp_service_handler(packet);
                    break;
            }
        }
        /* Close current connection */
        csp_close(conn);
    }
}

int obdh_init(int address)
{
    // initialize drivers
    drivers::uart_init();
    csp_init();

    csp_iface_t* iface;
    csp_usart_conf_t csp_conf = {
        .device = "/dev/serial1",
        .baudrate = 1000000,
        .databits = 8,
        .stopbits = 1,
        .paritysetting = 0,
    };
    auto ret = csp_usart_open_and_add_kiss_interface(&csp_conf, CSP_IF_KISS_DEFAULT_NAME, address, &iface);
    if (ret != CSP_ERR_NONE)
    {
        logger::error("Can't open kiss interface, ret: %d", ret);
        return ret;
    }
    iface->is_default = 1;

    logger::info("post csp_usart_open_and_add_kiss_interface()");

    // set rtable
    csp_rtable_set(1, 5, iface, CSP_NO_VIA_ADDRESS);

    // csp router task
    static StaticTask_t csp_rtr_tcb;
    StackType_t* rtr_stk_ptr = static_cast<StackType_t*>(malloc(CSP_ROUTER_STACK_DEPTH * sizeof(StackType_t)));
    if (!rtr_stk_ptr)
    {
        logger::error("Can't initialize router_stack");
        return CSP_ERR_NOMEM;
    }

    TaskHandle_t task_ret = xTaskCreateStatic(csp_route_task, "csp_router", CSP_ROUTER_STACK_DEPTH, nullptr,
                                              CSP_ROUTER_TASK_PRIO, rtr_stk_ptr, &csp_rtr_tcb);

    if (!task_ret)
    {
        logger::error("Can't create csp_router task");
        return CSP_ERR_NOMEM;
    }

    // dispatcher task
    static StaticTask_t csp_dispatcher_tcb;
    StackType_t* dispatcher_stk_ptr =
        static_cast<StackType_t*>(malloc(CSP_PACKETS_DISPATCHER_STACK_DEPTH * sizeof(StackType_t)));

    if (!dispatcher_stk_ptr)
    {
        logger::error("Can't initialize dispatcher_task");
        return CSP_ERR_NOMEM;
    }

    task_ret =
        xTaskCreateStatic(csp_packets_dispatcher_task, "csp_packets_dispatcher", CSP_PACKETS_DISPATCHER_STACK_DEPTH,
                          nullptr, CSP_PACKETS_DISPATCHER_TASK_PRIO, dispatcher_stk_ptr, &csp_dispatcher_tcb);
    if (!task_ret)
    {
        logger::error("Can't create csp_packets_dispatcher task");
        return CSP_ERR_NOMEM;
    }

    return CSP_ERR_NONE;
}
