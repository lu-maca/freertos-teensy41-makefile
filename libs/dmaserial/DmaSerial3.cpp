/**
 * Created by Hares.
 * You are free to use this file in any project as long as you keep my email address alihares99@gmail.com here.
 */

#include "DmaSerial.h"

#define IRQ_PRIORITY 64  // 0 = highest priority, 255 = lowest - matches old HardwareSerial3.cpp

// dmaSerial3 == LPUART2 (same physical port Serial3 used to own).
const DmaSerial::Base_t DmaSerial::serial3Base = {
	&IMXRT_LPUART2,
	DMAMUX_SOURCE_LPUART2_RX,
	DMAMUX_SOURCE_LPUART2_TX,
	CCM_CCGR0,
	CCM_CCGR0_LPUART2(CCM_CCGR_ON),
	{{15,2, &IOMUXC_LPUART2_RX_SELECT_INPUT, 1}, {0xff, 0xff, nullptr, 0}},
	{{14,2, &IOMUXC_LPUART2_TX_SELECT_INPUT, 1}, {0xff, 0xff, nullptr, 0}},
	IRQ_LPUART2,
	IRQ_PRIORITY,
};
