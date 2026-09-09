# teensy_libcsp_example
This project contains the porting of [libcsp](https://github.com/libcsp/libcsp) for a teensy 4.1, with a slightly modified version of FreeRTOS from [freertos-teensy](https://github.com/tsandmann/freertos-teensy).

The current version supports only CSP standard services by UART connection over teensy Serial1. It doesn't use the teensy4 core HardwareSerial implementation, but a modified DMA based serial 
implementation that has been taken from [DmaSerialTeensy4_0](https://github.com/alihares99/DmaSerialTeensy4_0).

Build options (compiler flags, defines and library config for libcsp) live in `project.yml`; `wscript` reads it via [waf](https://waf.io/).

```bash
python3 waf configure build   # build firmware
python3 waf upload            # flash it to the teensy over USB
```
