#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <csp/csp.h>
#include <csp/csp_cmp.h>
#include <csp/csp_debug.h>
#include <csp/drivers/usart.h>

static void* task_router(void* param)
{
    (void)param;
    while (1)
    {
        csp_route_work();
    }
    return NULL;
}

#define SERVER_ADDR 15
#define SERVER_PORT 10

#define CLIENT_ADDR 1
#define DEVICE_NAME "/dev/ttyUSB0"

void do_cmp_ident(int addr)
{
    // ident
    struct csp_cmp_ident_msg ident_msg;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int status = csp_cmp_ident(addr, 1000, &ident_msg);
    clock_gettime(CLOCK_MONOTONIC, &end);
	if (status == CSP_ERR_TIMEDOUT) {
    	printf("Timeout\n");
		return;
	}
    long long elapsed_ns = (long long)(end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
	double elapsed_us = elapsed_ns / 1000.0;
	
    printf("Replied in %f us.\n", elapsed_us);
}

int main(int argc, char* argv[])
{
    csp_usart_conf_t conf = {
        .device = DEVICE_NAME,
        .baudrate = 115200,
        .databits = 8,
        .stopbits = 1,
        .paritysetting = 0,
    };
    csp_iface_t* iface;
    int ret;

    /* init */
    csp_init();

    /* open */
    ret = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME, CLIENT_ADDR, &iface);
    if (ret != CSP_ERR_NONE)
    {
        csp_print("failed to open: %d\n", ret);
        return 1;
    }
    iface->is_default = 1;

    /* start the router task, without this no incoming packet is ever
     * processed */
    pthread_t router_thread;
    ret = pthread_create(&router_thread, NULL, task_router, NULL);
    if (ret != 0)
    {
        csp_print("failed to start router task: %d\n", ret);
        return 1;
    }

    do_cmp_ident(SERVER_ADDR);
    return 0;
}
