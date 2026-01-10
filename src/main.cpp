/*
 * This file is part of the FreeRTOS port to Teensy boards.
 * Copyright (c) 2020-2024 Timo Sandmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file    main.cpp
 * @brief   FreeRTOS example for Teensy boards
 * @author  Timo Sandmann
 * @date    17.05.2020
 */

#include "arduino_freertos.h"
extern "C" {
#include "csp.h"
#include "csp/arch/csp_thread.h"
#include "csp/drivers/usart.h"
#include "csp/interfaces/csp_if_kiss.h"
#include "csp/drivers/usart_teensy.h"
}

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

static void csp_task(void *param) {
  /* we want to use kiss, so we set up the csp_kiss interface */

  csp_usart_conf_t conf = {.device = "Serial1",
                           .baudrate = 500000, /* supported on all platforms */
                           .databits = 8,
                           .stopbits = 1,
                           .paritysetting = 0,
                           .checkparity = 0};

  csp_route_start_task(1024, 2);
  csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME,
                                        nullptr);

  // /* Setup routing table */
  csp_rtable_clear();
  csp_rtable_load("7/5 KISS");

  /* Create socket with no specific socket options, e.g. accepts CRC32, HMAC,
   * XTEA, etc. if enabled during compilation */
  csp_socket_t *sock = csp_socket(CSP_SO_NONE);

  /* Bind socket to all ports, e.g. all incoming connections will be handled
   * here */
  csp_bind(sock, CSP_ANY);

  /* Create a backlog of 10 connections, i.e. up to 10 new connections can be
   * queued */
  csp_listen(sock, 10);

  /* Wait for connections and then process packets on the connection */
  while (1) {

    /* Wait for a new connection, 10000 mS timeout */
    csp_conn_t *conn;
    if ((conn = csp_accept(sock, CSP_MAX_DELAY)) == NULL) {
      /* timeout */
      continue;
    }
    /* Read packets on connection, timout is 100 mS */
    csp_packet_t *packet;
    while ((packet = csp_read(conn, 50)) != NULL) {

      switch (csp_conn_dport(conn)) {
      case 10:
        /* Process packet here */
        csp_buffer_free(packet);
        break;

      default:
        /* Call the default CSP service handler, handle pings, buffer use,
        etc.
         */
        csp_service_handler(conn, packet);
        break;
      }
    }

    /* Close current connection */
    csp_close(conn);
  }
  vTaskDelete(nullptr);

  return CSP_TASK_RETURN;
}

int main() {
  teensy_serial_setup(115200);
  csp_conf_t csp_conf;
  csp_conf_get_defaults(&csp_conf);
  csp_conf.address = 1;
  csp_init(&csp_conf);

  xTaskCreate(task1, "task1", 128, nullptr, 2, nullptr);
  xTaskCreate(csp_task, "CSP", 1024, nullptr, 2, nullptr);

  vTaskStartScheduler();

  // never reach here
  while (1) {
  }
}
