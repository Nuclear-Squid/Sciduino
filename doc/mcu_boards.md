# MCU Dev Boards

Arduino Uno and Due are popular among hobbyists: easy to use and a lot of peripherals (“shields”) dedicated to their standard pinout, which is reused by a lot of manufacturers.

Most other MCU boards are designed to be plugged onto a breadboard (DIP). There’s no official standard, but some footprints/pinouts are here to stay — especially the Arduino Nano?

Here’s a list of popular MCU boards that follow a stable footprint/pinout.

[TOC]

## Arduino

### Uno

The [Uno family](https://store.arduino.cc/collections/uno) is the most popular: 32 pins (+6`*`) and countless shields.

| Name            | MCU              | Arch       | Clock  | I/O         |
|-----------------|------------------|------------|-------:|------------:|
| [Uno Wifi]      | ATmega 4809      | AVR        | 16 MHz | 5 V         |
| [Leonardo]      | ATmega 32u4      | AVR        | 16 MHz | 5 V         |
| [Uno R4 Minima] | RA4M1            | Cortex-M4  | 48 MHz | 5 V         |
| [Uno R4 Wifi]   | RA4M1 + ESP32‑S3 | Cortex-M4 + 2× Xtensa LX7 | 48 + 240 MHz | 5 V + 3.3 V |
| [Zero]          | SAMD21           | Cortex-M0+ | 48 MHz |       3.3 V |

[Uno Wifi]:      https://store.arduino.cc/products/arduino-uno-wifi-rev2
[Leonardo]:      https://store.arduino.cc/collections/uno/products/arduino-leonardo-with-headers
[Uno R4 Minima]: https://store.arduino.cc/collections/boards-modules/products/uno-r4-minima
[Uno R4 Wifi]:   https://store.arduino.cc/collections/boards-modules/products/uno-r4-wifi
[Zero]:          https://store.arduino.cc/collections/uno/products/arduino-zero

### Due

The [Due/Mega/Giga family](https://store.arduino.cc/collections/giga) is a superset of Uno, with 3 additional UARTs, 8 more ADC and a huge GPIO.

| Name           | MCU         | Arch         | Clock         | I/O   |
|----------------|-------------|--------------|--------------:|------:|
| [Mega 2560]    | ATmega 2560 | AVR          | 16 MHz        | 5 V   |
| [Due]          | SAM3X8E     | Cortex‑M3    | 84 MHz        | 3.3 V |
| [Giga R1 Wifi] | STM32H747XI | Cortex‑M7+M4 | 480 + 240 MHz | 3.3 V |

[Mega 2560]:    https://store.arduino.cc/products/arduino-mega-2560-rev3
[Due]:          https://store.arduino.cc/collections/boards-modules/products/arduino-due
[Giga R1 Wifi]: https://store.arduino.cc/products/giga-r1-wifi

### Nano

The [Nano family](https://store.arduino.cc/collections/nano-family) has a compact DIP format with 2×15 pins (+6`*`).

| Name          | MCU         | Arch          | Clock   | I/O   |
|---------------|-------------|---------------|--------:|------:|
| [Nano]        | ATmega 328  | AVR           |  16 MHz | 5 V   |
| [Nano Every]  | ATmega 4809 | AVR           |  20 MHz | 5 V   |
| [Nano 33 BLE] | nRF52840    | Cortex‑M4     |  64 MHz | 3.3 V |
| [Nano Matter] | MGM240S     | Cortex‑M33    |  78 MHz | 3.3 V |
| [Nano RP2040] | RP2040      | 2× Cortex‑M0+ | 125 MHz | 3.3 V |
| [Nano ESP32]  | ESP32-S3    | 2× Xtensa LX7 | 240 MHz | 3.3 V |

[Nano]:        https://store.arduino.cc/collections/nano-family/products/arduino-nano
[Nano Every]:  https://store.arduino.cc/products/nano-every-with-headers
[Nano 33 BLE]: https://store.arduino.cc/products/nano-33-ble-rev2-with-headers
[Nano Matter]: https://store.arduino.cc/collections/boards-modules/products/nano-matter-with-headers
[Nano RP2040]: https://store.arduino.cc/collections/nano-family/products/arduino-nano-rp2040-connect-with-headers
[Nano ESP32]:  https://store.arduino.cc/collections/boards-modules/products/nano-esp32-with-headers

`*` Most Arduino boards have a 6-pin SPI header (SPI+5V+GND+Reset) at the opposite side of the USB connector. On Uno and Nano it’s just a convenient connector for pins that are already on the sides of the board, but on the Due/Mega/Giga it’s a dedicated SPI pin set.

### MKR / Portenta

The [MKR family] targets the same goals as Adafruit Feather (see below), with a similar format — despite being released *after* the Feather spec. It’s dedicated to IoT, based on SAMD21, with boards for WiFi, GSM, Lora, SigFox, NarrowBand… A lot of MKR shields are available.

The [Portenta family] is targeted towards high-speed MCUs for professional use. These boards feature a high-density 80-pin connector, along with the MKR pinout.

| Name           | MCU            | Arch            |       Clock   | I/O   |
|----------------|----------------|-----------------|--------------:|------:|
| [MKR family]   | SAMD21G18A     | Cortex-M0+      |        48 MHz | 3.3 V |
| [Portenta C33] | R7FA6M5BH2CBG  | Cortex‑M33      |       200 MHz | 3.3 V |
| [Portenta H7]  | STM32H747XI    | Cortex‑M7+M4    | 480 + 240 MHz | 3.3 V |

The [Portenta X8] is a full SBC running Linux without MKR connectivity, and doesn’t fit in this list.

[MKR family]:      https://store.arduino.cc/collections/mkr-family
[Portenta family]: https://store.arduino.cc/collections/portenta-family
[Portenta C33]:    https://docs.arduino.cc/hardware/portenta-c33/
[Portenta H7]:     https://docs.arduino.cc/hardware/portenta-h7/
[Portenta X8]:     https://docs.arduino.cc/hardware/portenta-x8/

## STM32 Nucleo

Many manufacturers build Arduino-compatible products (same footprint, same pinout, same software stack). but STM32 has chosen to be only partially compatible.

- [Nucleo‑32]: Arduino Nano pinout
- [Nucleo‑64]: Arduino Uno rev3 pinout + ST Morpho extensions
- [Nucleo‑144]: Arduino Uno rev3 pinout + ST Morpho + ST Zio extensions

[Nucleo‑32]:  https://www.st.com/resource/en/data_brief/nucleo-f031k6.pdf
[Nucleo‑64]:  https://www.st.com/resource/en/data_brief/nucleo-c031c6.pdf
[Nucleo‑144]: https://www.st.com/resource/en/data_brief/nucleo-f207zg.pdf

Compatibility breakers:
- the Arduino 6-pin SPI header is missing;
- the USB connector is misplaced:
  - on Nucleo-32 boards, the USB port is on the opposite side;
  - on Nucleo-64 and -144 boards, the USB port is on an extended part of the PCB.

Arduino framework support through [STM32duino](https://github.com/stm32duino).

## Other Footprint Specs

### SparkFun Pro Micro

Started in 2013 (?) as SparkFun’s Arduino variant with a smaller footprint.

- 8-bit controllers: AVR [5V/16MHz] and [3V/8MHz]
- 32-bit controllers: [RP2040][PM1], [RP2350][PM2], [ESP32-C3][PM3]

Other manufacturers propose Pro Micro boards: [AdaFruit KB2040] (“Key Boar Driver”), [nice!nano] (nRF52840), [Elite-C] (ATmega 32u4)…

Pro Micro is the standard among keyboard makers but hasn’t gained general adoption otherwise.

[5V/16MHz]: https://www.sparkfun.com/pro-micro-5v-16mhz.html
[3V/8MHz]:  https://www.sparkfun.com/pro-micro-3-3v-8mhz.html
[PM1]:      https://www.sparkfun.com/sparkfun-pro-micro-rp2040.html
[PM2]:      https://www.sparkfun.com/sparkfun-pro-micro-rp2350.html
[PM3]:      https://www.sparkfun.com/sparkfun-pro-micro-esp32-c3.html

[AdaFruit KB2040]:   https://www.adafruit.com/product/5302
[nice!nano]:         https://nicekeyboards.com/nice-nano/
[Promicro nRF52840]: https://pandakb.com/products/parts/others/promicro-nrf52840/
[Elite-C]:           https://keeb.io/products/elite-c-low-profile-version-usb-c-pro-micro-replacement-atmega32u4

### Adafruit Feather

Released in 2015 (?), the Feather form factor is [an open specification](https://learn.adafruit.com/adafruit-feather/feather-specification) that Adafruit encourages to use.
SparkFun is the main other Feather manufacturer, under the name “Thing Plus”.

- [Adafruit Feather](https://www.adafruit.com/category/943)
- [SparkFun Thing Plus](https://www.sparkfun.com/development-boards.html?sf_ecosystem=3136)

It is desinged with battery-powered IoT devices in mind. It applies to MCU boards as well as peripherals, called FeatherWings (comparable to Arduino shields).

## Other Popular Boards

### Raspberry Pi Pico

| Name     | MCU    | Arch                    | PIO | Clock   | I/O   |
|----------|--------|-------------------------|----:|--------:|------:|
| [Pico 1] | RP2040 | 2× Cortex-M0+           |  8  | 133 MHz | 3.3 V |
| [Pico 2] | RP2350 | 2× Cortex-M33 2× RISC‑V | 12  | 150 MHz | 3.3 V |

Consistent footprint and pinout, very low price tag, and the PIO (programmable IO) are a game changer.

[Pico 1]: https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pico-1-family
[Pico 2]: https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pico-2-family

### Seeed Studio XIAO

| Name       | MCU       | Arch                    | Clock   | I/O         |
|------------|-----------|-------------------------|--------:|------------:|
| [SAMD21]   | SAMD21G18 | Cortex-M0+              |  48 MHz | 5 V + 3.3 V |
| [RA4M1]    | RA4M1     | Cortex-M4               |  48 MHz |       3.3 V |
| [nRF52840] | nRF52840  | Cortex-M4               |  64 MHz |       3.3 V |
| [MG24]     | MG24      | Cortex-M33              |  78 MHz |       3.3 V |
| [RP2040]   | RP2040    | 2× Cortex-M0+           | 133 MHz |       3.3 V |
| [RP2350]   | RP2350    | 2× Cortex-M33 2× RISC‑V | 150 MHz |       3.3 V |
| [ESP32-C3] | ESP32-C3  | 2× RISC‑V               | 160 MHz |       3.3 V |

Consistent footprint and pinout, very small footprint, and a nice range of MCUs.

[SAMD21]:   https://wiki.seeedstudio.com/Seeeduino-XIAO/
[RA4M1]:    https://wiki.seeedstudio.com/getting_started_xiao_ra4m1/
[nRF52840]: https://wiki.seeedstudio.com/XIAO_BLE/
[MG24]:     https://wiki.seeedstudio.com/xiao_mg24_getting_started/
[RP2040]:   https://wiki.seeedstudio.com/XIAO-RP2040/
[RP2350]:   https://wiki.seeedstudio.com/getting-started-xiao-rp2350/
[ESP32-C3]: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/

### Teensy

| Name         | MCU           | Arch      | Format   | Clock   | I/O   |
|--------------|---------------|-----------|---------:|--------:|------:|
| [Teensy 4.0] | NXP iMXRT1062 | Cortex-M7 | 1.4×0.7” | 600 MHz | 3.3 V |
| [Teensy 4.1] | NXP iMXRT1062 | Cortex-M7 | 2.4×0.7” | 600 MHz | 3.3 V |

Designed (and initially manufactured) by [PJRC], now manufactured by [SparkFun], these boards are popular for their performance/price ratio.

The footprint and pinout seem to remain consistent over time: the Teensy 4.0 and 4.1 are (mostly) compatible with the former 3.2 and 3.6 versions, respectively.

[Teensy 4.1]: https://www.sparkfun.com/teensy-4-1.html
[Teensy 4.0]: https://www.sparkfun.com/teensy-4-0.html
[PJRC]:       https://www.pjrc.com/store/teensy41.html
[SparkFun]:   https://www.sparkfun.com/development-boards/microcontrollers/teensy.html

### Espressif ESP32

These boards are *very* popular for IoT development, with built-in wireless features and good performances.

They are not listed here because unlike the other boards in this document, they don’t share a common footprint.

