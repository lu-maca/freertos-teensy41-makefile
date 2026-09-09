/**
 * Created by Hares.
 * You are free to use this file in any project as long as you keep my email address alihares99@gmail.com here.
 */

#pragma once

#include "core_pins.h"
#include "imxrt.h"
#include "DMAChannel.h"
#include "Stream.h"
#include "arduino_freertos.h"
#include <cstddef>
// ------------------- change the following if you want ---------------------- //
#define DMA_TX_BUFFER_SIZE          128
#define DMA_RX_BUFFER_SIZE          4096

// ------------------- do not change the rest ------------------------//

#define DMA_MAX_BURST_DATA_TRANSFER 511         // This is the maximum data we are putting into DMA at once

class DmaSerial : public Stream {

private:

    static constexpr uint8_t cnt_tx_pins = 2;
    static constexpr uint8_t cnt_rx_pins = 2;
    struct pin_info_t {
        const uint8_t 		pin;		// The pin number
        const uint32_t 		mux_val;	// Value to set for mux;
        volatile uint32_t	*select_input_register; // Which register controls the selection
        const uint32_t		select_val;	// Value for that selection
    };

    // NOTE: irq/irq_priority are new - this is the piece HardwareSerialIMXRT
    // used to own (hardware_t::irq / hardware_t::irq_priority in HardwareSerial.h).
    // DmaSerial now attaches its own uartIsr() to this vector instead of
    // HardwareSerial's IRQHandler_SerialN, so LPUARTx IDLE/error events are
    // serviced here and nowhere else.
    struct Base_t {
        IMXRT_LPUART_t* port;
        uint8_t dmaMuxSourceRx;
        uint8_t dmaMuxSourceTx;
        volatile uint32_t &ccm_register;
        const uint32_t ccm_value;
        pin_info_t rx_pins[cnt_rx_pins];
        pin_info_t tx_pins[cnt_tx_pins];
        IRQ_NUMBER_t irq;
        uint8_t irq_priority;
    };

    const static Base_t serial1Base;
    const static Base_t serial2Base;
    const static Base_t serial3Base;
    const static Base_t serial4Base;
    const static Base_t serial5Base;
    const static Base_t serial6Base;
    const static Base_t serial7Base;
    const static Base_t* allSerialBases[7];
    const static DmaSerial* dmaSerials[7];

    static void txCompleteCallback1();
    static void txCompleteCallback2();
    static void txCompleteCallback3();
    static void txCompleteCallback4();
    static void txCompleteCallback5();
    static void txCompleteCallback6();
    static void txCompleteCallback7();
    static void (* const allTxIsr[7])();

    static void rxCompleteCallback1();
    static void rxCompleteCallback2();
    static void rxCompleteCallback3();
    static void rxCompleteCallback4();
    static void rxCompleteCallback5();
    static void rxCompleteCallback6();
    static void rxCompleteCallback7();
    static void (* const allRxIsr[7])();

    // LPUARTx peripheral IRQ (IDLE line detect). One dispatcher per instance,
    // same pattern as allTxIsr/allRxIsr above, so LPUART1..LPUART7's vector
    // table entries can point at a plain C function instead of a member fn.
    static void uartCompleteCallback1();
    static void uartCompleteCallback2();
    static void uartCompleteCallback3();
    static void uartCompleteCallback4();
    static void uartCompleteCallback5();
    static void uartCompleteCallback6();
    static void uartCompleteCallback7();
    static void (* const allUartIsr[7])();

    int serialNo;
    int rxPinIndex;
    int txPinIndex;

    uint8_t* txBuffer = nullptr;
    uint8_t* rxBuffer = nullptr;
    volatile size_t txBufferTail;
    volatile size_t txBufferHead;
    volatile size_t txBufferCount;
    volatile size_t rxBufferTail;

    volatile bool transmitting = false;

    const Base_t* serialBase = nullptr;
    DMAChannel* dmaChannelSend = nullptr;
    DMAChannel* dmaChannelReceive = nullptr;

    // Task woken from ISR context by DMA-half, DMA-complete AND UART-IDLE.
    // The RX task does not need to know which of the three fired.
    TaskHandle_t rxTaskHandle = nullptr;

    void txIsr();
    void rxIsr();
    void uartIsr();
    void notifyRxTask();

public:

    explicit DmaSerial(int serialNo);
    // Must be called before begin() so the very first IDLE/DMA event has
    // somewhere to deliver its notification.
    void setRxTaskHandle(TaskHandle_t handle) { rxTaskHandle = handle; }
    int peek() override;
    void begin(uint32_t baud, uint16_t format = 0);
    int available() override;
    int read() override;
    using Print::write;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(char c);
    size_t write(unsigned long n)   { return write((uint8_t)n); }
    size_t write(long n)            { return write((uint8_t)n); }
    size_t write(unsigned int n)    { return write((uint8_t)n); }
    size_t write(int n)             { return write((uint8_t)n); }
};

#if defined(__IMXRT1062__)

extern DmaSerial dmaSerial1;
extern DmaSerial dmaSerial2;
extern DmaSerial dmaSerial3;
extern DmaSerial dmaSerial4;
extern DmaSerial dmaSerial5;
extern DmaSerial dmaSerial6;
extern DmaSerial dmaSerial7;

#endif
