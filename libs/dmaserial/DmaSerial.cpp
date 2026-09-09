/**
 * Created by Hares.
 * You are free to use this file in any project as long as you keep my email address alihares99@gmail.com here.
 */

#include "arduino_freertos.h"
#include "DmaSerial.h"
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef __IMXRT1062__  // teensy 4.0, 4.1

#define UART_CLOCK 24000000

#define CTRL_ENABLE 		(LPUART_CTRL_TE | LPUART_CTRL_RE)

// teensy 4.0 specific board information:

const DmaSerial::Base_t* DmaSerial::allSerialBases[7] = {
        &serial1Base,
        &serial2Base,
        &serial3Base,
        &serial4Base,
        &serial5Base,
        &serial6Base,
        &serial7Base,
};

DmaSerial dmaSerial1(1);
DmaSerial dmaSerial2(2);
DmaSerial dmaSerial3(3);
DmaSerial dmaSerial4(4);
DmaSerial dmaSerial5(5);
DmaSerial dmaSerial6(6);
DmaSerial dmaSerial7(7);
const DmaSerial* DmaSerial::dmaSerials[7] = {
        &dmaSerial1,
        &dmaSerial2,
        &dmaSerial3,
        &dmaSerial4,
        &dmaSerial5,
        &dmaSerial6,
        &dmaSerial7,
};
#else
#error "no supported board"
#endif

void DmaSerial::txCompleteCallback1() {dmaSerial1.txIsr();}
void DmaSerial::txCompleteCallback2() {dmaSerial2.txIsr();}
void DmaSerial::txCompleteCallback3() {dmaSerial3.txIsr();}
void DmaSerial::txCompleteCallback4() {dmaSerial4.txIsr();}
void DmaSerial::txCompleteCallback5() {dmaSerial5.txIsr();}
void DmaSerial::txCompleteCallback6() {dmaSerial6.txIsr();}
void DmaSerial::txCompleteCallback7() {dmaSerial7.txIsr();}
void (* const DmaSerial::allTxIsr[7])() = {
        txCompleteCallback1,
        txCompleteCallback2,
        txCompleteCallback3,
        txCompleteCallback4,
        txCompleteCallback5,
        txCompleteCallback6,
        txCompleteCallback7,
};

void DmaSerial::rxCompleteCallback1() {dmaSerial1.rxIsr();}
void DmaSerial::rxCompleteCallback2() {dmaSerial2.rxIsr();}
void DmaSerial::rxCompleteCallback3() {dmaSerial3.rxIsr();}
void DmaSerial::rxCompleteCallback4() {dmaSerial4.rxIsr();}
void DmaSerial::rxCompleteCallback5() {dmaSerial5.rxIsr();}
void DmaSerial::rxCompleteCallback6() {dmaSerial6.rxIsr();}
void DmaSerial::rxCompleteCallback7() {dmaSerial7.rxIsr();}
void (* const DmaSerial::allRxIsr[7])() = {
        rxCompleteCallback1,
        rxCompleteCallback2,
        rxCompleteCallback3,
        rxCompleteCallback4,
        rxCompleteCallback5,
        rxCompleteCallback6,
        rxCompleteCallback7,
};

// LPUARTx peripheral IRQ (formerly IRQHandler_SerialN -> HardwareSerialIMXRT::IRQHandler).
// This is now the ONLY code that runs on LPUART1..LPUART7's own vector; it
// exists solely to service the IDLE condition, which DMA cannot detect by itself.
void DmaSerial::uartCompleteCallback1() {dmaSerial1.uartIsr();}
void DmaSerial::uartCompleteCallback2() {dmaSerial2.uartIsr();}
void DmaSerial::uartCompleteCallback3() {dmaSerial3.uartIsr();}
void DmaSerial::uartCompleteCallback4() {dmaSerial4.uartIsr();}
void DmaSerial::uartCompleteCallback5() {dmaSerial5.uartIsr();}
void DmaSerial::uartCompleteCallback6() {dmaSerial6.uartIsr();}
void DmaSerial::uartCompleteCallback7() {dmaSerial7.uartIsr();}
void (* const DmaSerial::allUartIsr[7])() = {
        uartCompleteCallback1,
        uartCompleteCallback2,
        uartCompleteCallback3,
        uartCompleteCallback4,
        uartCompleteCallback5,
        uartCompleteCallback6,
        uartCompleteCallback7,
};


DmaSerial::DmaSerial(int serialNo)
    : serialNo(serialNo)
{
    serialBase = allSerialBases[serialNo - 1];
    rxPinIndex = 0; // default pin = first pin
    txPinIndex = 0; // default pin = first pin

    txBufferTail = 0;
    txBufferHead = 0;
    txBufferCount = 0;
    rxBufferTail = 0;
    // rxBufferHead = 0; // no need for this
}


void DmaSerial::begin(uint32_t baud, uint16_t format) {

    if (!txBuffer) {
        txBuffer = new uint8_t[DMA_TX_BUFFER_SIZE];
    }
    if (!rxBuffer) {
        rxBuffer = new uint8_t[DMA_RX_BUFFER_SIZE];
    }

    // configure DMA channels:
    if (!dmaChannelSend) {
        dmaChannelSend = new DMAChannel();
        dmaChannelSend->destination(*(uint8_t*)&serialBase->port->DATA);
        dmaChannelSend->triggerAtHardwareEvent(serialBase->dmaMuxSourceTx);
        dmaChannelSend->attachInterrupt(allTxIsr[serialNo - 1]);
        dmaChannelSend->interruptAtCompletion();
        dmaChannelSend->disableOnCompletion();
        // not enabled here
    }
    if (!dmaChannelReceive) {
        dmaChannelReceive = new DMAChannel();
        dmaChannelReceive->source(*(uint8_t*)&serialBase->port->DATA);
        dmaChannelReceive->destinationBuffer(rxBuffer, DMA_RX_BUFFER_SIZE);
        dmaChannelReceive->triggerAtHardwareEvent(serialBase->dmaMuxSourceRx);
        // NOTE: this used to (incorrectly) configure dmaChannelSend here,
        // which stomped the TX channel's ISR vector with the RX callback
        // and left the RX channel's own IRQ never attached/enabled.
        dmaChannelReceive->attachInterrupt(allRxIsr[serialNo - 1]);
        dmaChannelReceive->interruptAtHalf();       // RX design: half...
        dmaChannelReceive->interruptAtCompletion();  // ...and complete both notify the RX task
        // destinationBuffer() wired DLASTSGA for auto-wrap, and we never call
        // disableOnCompletion() for RX, so this channel free-runs continuously.
        dmaChannelReceive->enable();
    }

    txBufferTail = 0;
    txBufferHead = 0;
    txBufferCount = 0;
    rxBufferTail = 0;
    // rxBufferHead = 0; // no need for this

    // calculate baudrate:
    float base = (float)UART_CLOCK / (float)baud;
    float besterr = 1e20;
    int bestdiv = 1;
    int bestosr = 4;
    for (int osr=4; osr <= 32; osr++) {
        float div = base / (float)osr;
        int divint = lroundf(div);
        if (divint < 1) divint = 1;
        else if (divint > 8191) divint = 8191;
        float err = ((float)divint - div) / div;
        if (err < 0.0f) err = -err;
        if (err <= besterr) {
            besterr = err;
            bestdiv = divint;
            bestosr = osr;
        }
    }

    // turn on clock for UART:
    serialBase->ccm_register |= serialBase->ccm_value;

    // disable UART:
    serialBase->port->CTRL = 0;

    // config Rx Pin:
    *(portControlRegister(serialBase->rx_pins[rxPinIndex].pin)) = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_PUS(3) | IOMUXC_PAD_HYS;
    *(portConfigRegister(serialBase->rx_pins[rxPinIndex].pin)) = serialBase->rx_pins[rxPinIndex].mux_val;
    if (serialBase->rx_pins[rxPinIndex].select_input_register) {
        *(serialBase->rx_pins[rxPinIndex].select_input_register) =  serialBase->rx_pins[rxPinIndex].select_val;
    }

    // config Tx Pin:
    *(portControlRegister(serialBase->tx_pins[txPinIndex].pin)) =  IOMUXC_PAD_SRE | IOMUXC_PAD_DSE(3) | IOMUXC_PAD_SPEED(3);
    *(portConfigRegister(serialBase->tx_pins[txPinIndex].pin)) = serialBase->tx_pins[txPinIndex].mux_val;

    serialBase->port->BAUD = LPUART_BAUD_OSR(bestosr - 1) | LPUART_BAUD_SBR(bestdiv);
    serialBase->port->PINCFG = 0;

    // enabling DMA instead:
    serialBase->port->BAUD |= (LPUART_BAUD_TDMAE | LPUART_BAUD_RDMAE);

    // disabling FIFO:
    serialBase->port->FIFO &= ~(LPUART_FIFO_TXFE | LPUART_FIFO_RXFE);

    // Attach and enable the LPUARTx peripheral IRQ. This is the vector that
    // used to belong to IRQHandler_SerialN / HardwareSerialIMXRT::IRQHandler.
    // uartIsr() only services IDLE here - RIE/TIE are deliberately left OFF
    // below because RDMAE/TDMAE already move every byte via DMA; leaving
    // RIE/TIE set as well would fire this same IRQ on every single RX/TX
    // byte in addition to DMA, on top of whatever else is pending.
    attachInterruptVector(serialBase->irq, allUartIsr[serialNo - 1]);
    NVIC_SET_PRIORITY(serialBase->irq, serialBase->irq_priority);
    NVIC_ENABLE_IRQ(serialBase->irq);

    // lets configure up our CTRL register value
    // RIE/TIE intentionally omitted: RX/TX data movement is done entirely by
    // DMA (RDMAE/TDMAE above). ILIE is the only line-status interrupt we need,
    // serviced by uartIsr(). ORIE is worth adding if you want overrun errors
    // surfaced instead of silently dropped - not enabled here to match the
    // original HardwareSerial feature set as closely as possible.
    uint32_t ctrl = CTRL_ENABLE | LPUART_CTRL_ILIE;

    // Now process the bits in the Format value passed in
    // Bits 0-2 - Parity plus 9  bit.
    ctrl |= (format & (LPUART_CTRL_PT | LPUART_CTRL_PE) );	// configure parity - turn off PT, PE, M and configure PT, PE
    if (format & 0x04) ctrl |= LPUART_CTRL_M;		// 9 bits (might include parity)
    if ((format & 0x0F) == 0x04) ctrl |=  LPUART_CTRL_R9T8; // 8N2 is 9 bit with 9th bit always 1

    // Bit 5 TXINVERT
    if (format & 0x20) ctrl |= LPUART_CTRL_TXINV;		// tx invert

    // Bit 3 10 bit - Will assume that begin already cleared it.
    // process some other bits which change other registers.
    if (format & 0x08) 	serialBase->port->BAUD |= LPUART_BAUD_M10;

    // Bit 4 RXINVERT
    uint32_t c = serialBase->port->STAT & ~LPUART_STAT_RXINV;
    if (format & 0x10) c |= LPUART_STAT_RXINV;		// rx invert
    serialBase->port->STAT = c;

    // bit 8 can turn on 2 stop bit mote
    if ( format & 0x100) serialBase->port->BAUD |= LPUART_BAUD_SBNS;

    // write out computed CTRL and turn on UART transmit and receive
    serialBase->port->CTRL = ctrl;

}

/**
 * Number of bytes in the buffer
 * @return 0 to 2047
 */
int DmaSerial::available() {
    auto biter = dmaChannelReceive->TCD->BITER;
    auto citer = dmaChannelReceive->TCD->CITER;
    auto csr = dmaChannelReceive->TCD->CSR;
    if (csr & 0x80) { // done so Rx buffer is full
        if (rxBufferTail == 0) return 0;
        else return DMA_RX_BUFFER_SIZE - rxBufferTail;
    }
    else {
        // our version of buffer indexes are not update
        size_t head = biter - citer;
        if (head >= rxBufferTail) return head - rxBufferTail;
        else return head - rxBufferTail + DMA_RX_BUFFER_SIZE;
    }
}

int DmaSerial::read() {
    uint8_t c = rxBuffer[rxBufferTail++];
    if (rxBufferTail >= DMA_RX_BUFFER_SIZE)
        rxBufferTail -= DMA_RX_BUFFER_SIZE;
    return c;
}

int DmaSerial::peek() {
    return rxBuffer[rxBufferTail];
}

size_t DmaSerial::write(uint8_t c) {
    write(&c, 1);
    return 1;
}

size_t DmaSerial::write(char c) {
    return write((uint8_t *)&c, 1);
}

size_t DmaSerial::write(const uint8_t *p, size_t len) {

    size_t index = 0;
    while (index < len) {

        // wait until there is free space in the buffer:
        while (DMA_TX_BUFFER_SIZE - txBufferCount == 0); //

        // get a chunk of data to add to the buffer
        size_t chunkSize = std::min(len - index, DMA_TX_BUFFER_SIZE - txBufferCount);

        // copy the data to the buffer:
        size_t s1 = std::min(chunkSize, DMA_TX_BUFFER_SIZE - txBufferHead);
        size_t s2 = chunkSize - s1;
        memcpy(&txBuffer[txBufferHead], &p[index], s1);
        if (s2 > 0)
            memcpy(&txBuffer[0], &p[index + s1], s2);
        index += chunkSize;

        // move the head:
        txBufferCount += chunkSize;
        txBufferHead += chunkSize;
        if (txBufferHead >= DMA_TX_BUFFER_SIZE)
            txBufferHead -= DMA_TX_BUFFER_SIZE;

        // start transmitting from the tail:
        if (!transmitting) {
            transmitting = true;
            __disable_irq()
            size_t count = std::min(DMA_TX_BUFFER_SIZE - txBufferTail, chunkSize);
            count = std::min(count, size_t(DMA_MAX_BURST_DATA_TRANSFER)); // min(remaining in the buffer, len_truncate, max_burst)
            dmaChannelSend->sourceBuffer(&txBuffer[txBufferTail], count);
            dmaChannelSend->enable();
            __enable_irq();
        }
    }
    return len;

}

void DmaSerial::txIsr() {
    dmaChannelSend->clearInterrupt();

    // move the tail:
    {
        int count = dmaChannelSend->TCD->BITER;

        txBufferTail += count;
        if (txBufferTail >= DMA_TX_BUFFER_SIZE)
            txBufferTail -= DMA_TX_BUFFER_SIZE;
        txBufferCount -= count;
    }

    if (txBufferCount > 0) {
        transmitting = true;
        __disable_irq()
        size_t count = std::min(size_t(DMA_TX_BUFFER_SIZE) - txBufferTail, size_t(txBufferCount));
        count = std::min(count, size_t(DMA_MAX_BURST_DATA_TRANSFER)); // MIN(remaining in the buffer, txBufferCount, max_burst)
        dmaChannelSend->sourceBuffer(&txBuffer[txBufferTail], count);
        dmaChannelSend->enable();
        __enable_irq();
    }
    else {
        transmitting = false;
    }
}

void DmaSerial::rxIsr() {
    // Fires for BOTH DMA-half and DMA-complete (interruptAtHalf() +
    // interruptAtCompletion() both set in begin()). clearInterrupt() clears
    // the DMA channel's own IRQ-pending flag (DMA_CINT), not the ring buffer
    // position - the RX task recomputes how much is available on wake via
    // available()/read(), same as it will after uartIsr()'s IDLE wake.
    dmaChannelReceive->clearInterrupt();
    notifyRxTask();
}

void DmaSerial::uartIsr() {
    // This is the entire replacement for HardwareSerialIMXRT::IRQHandler()'s
    // IDLE branch (port->STAT & LPUART_STAT_IDLE, cleared by writing 1 back).
    // RDRF/TDRE/TC are not read or acted on here - DMA (RDMAE/TDMAE) owns
    // those, and RIE/TIE/TCIE are left disabled in begin() so this vector
    // should only ever be entered because of IDLE (or a line error, if you
    // enable ORIE/FEIE/PEIE/NEIE later - handle those here too if you do).
    if (serialBase->port->STAT & LPUART_STAT_IDLE) {
        // STAT is write-1-to-clear for IDLE; |= only ever sets bits that are
        // already 1 or is a no-op on the read-only status bits, so this can't
        // accidentally clear a flag we haven't looked at (same pattern as
        // the old HardwareSerialIMXRT::IRQHandler()).
        serialBase->port->STAT |= LPUART_STAT_IDLE;
        notifyRxTask();
    }
}

void DmaSerial::notifyRxTask() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(
        rxTaskHandle,
        &higher_priority_task_woken
    );
    portYIELD_FROM_ISR(higher_priority_task_woken);
}



