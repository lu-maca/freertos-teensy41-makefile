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
}


static void task1(void *) {
  pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
  while (true) {
    digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
    vTaskDelay(pdMS_TO_TICKS(2000));

    digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

  vTaskDelete(nullptr);
}

static void task2(void *) {
  Serial.begin(0);
  if (CrashReport) {
    Serial.print(CrashReport);
    Serial.println();
    Serial.flush();
  }

  Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER
                      ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION
                      ") on " __DATE__ ". ***\r\n"));
  while (true) {
    Serial.println("TICK");
    vTaskDelay(pdMS_TO_TICKS(1'000));

    Serial.println("TOCK");
    vTaskDelay(pdMS_TO_TICKS(1'000));
  }

  vTaskDelete(nullptr);
}

static void csp_task(void * param) {
/* Create socket with no specific socket options, e.g. accepts CRC32, HMAC, XTEA, etc. if enabled during compilation */
	csp_socket_t *sock = csp_socket(CSP_SO_NONE);

	/* Bind socket to all ports, e.g. all incoming connections will be handled here */
	csp_bind(sock, CSP_ANY);

	/* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued */
	csp_listen(sock, 10);

	/* Wait for connections and then process packets on the connection */
	while (1) {

		/* Wait for a new connection, 10000 mS timeout */
		csp_conn_t *conn;
		if ((conn = csp_accept(sock, 10000)) == NULL) {
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
				/* Call the default CSP service handler, handle pings, buffer use, etc. */
				csp_service_handler(conn, packet);
				break;
			}
		}

		/* Close current connection */
		csp_close(conn);

	}

	return CSP_TASK_RETURN;
}


int main() {

  xTaskCreate(task1, "task1", 128, nullptr, 2, nullptr);
  xTaskCreate(task2, "task2", 128, nullptr, 2, nullptr);
  xTaskCreate(csp_task, "CSP", 1024, nullptr, 2, nullptr);

  Serial.println(PSTR("setup(): starting scheduler..."));
  Serial.flush();

  vTaskStartScheduler();

  // never reach here
  while (1) {
  }
}
