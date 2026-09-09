/**
 * Created by Hares.
 * You are free to use this file in any project as long as you keep my email address alihares99@gmail.com here.
 */

#include "DmaSerial.h"

#define IRQ_PRIORITY 64  // 0 = highest priority, 255 = lowest - matches old HardwareSerial1.cpp

// dmaSerial1 == LPUART6 (same physical port Serial1 used to own).
const DmaSerial::Base_t DmaSerial::serial1Base = {
	&IMXRT_LPUART6,
	DMAMUX_SOURCE_LPUART6_RX,
	DMAMUX_SOURCE_LPUART6_TX,
	CCM_CCGR3,
	CCM_CCGR3_LPUART6(CCM_CCGR_ON),
	#if defined(ARDUINO_TEENSY41)
	{{0,2, &IOMUXC_LPUART6_RX_SELECT_INPUT, 1}, {52, 2, &IOMUXC_LPUART6_RX_SELECT_INPUT, 0}},
	{{1,2, &IOMUXC_LPUART6_TX_SELECT_INPUT, 1}, {53, 2, &IOMUXC_LPUART6_TX_SELECT_INPUT, 0}},
	#else
	{{0,2, &IOMUXC_LPUART6_RX_SELECT_INPUT, 1}, {0xff, 0xff, nullptr, 0}},
	{{1,2, &IOMUXC_LPUART6_TX_SELECT_INPUT, 1}, {0xff, 0xff, nullptr, 0}},
	#endif
	IRQ_LPUART6,
	IRQ_PRIORITY,
};
