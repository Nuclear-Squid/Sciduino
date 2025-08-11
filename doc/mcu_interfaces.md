# MCU Interfaces

## Peripherals

There are standard footprints and pinouts for the two main classes of peripherals:

- Arduino Uno “shields” (and Arduino Due to a lesser extent);
- Raspberry Pi “HATs” (Hardware Attached on Top).

These footprints are here to stay and a lot of shields/HATs are easily available. A solid standard. 

## DIP Development Boards

These MCU development boards are designed to be plugged onto a breadboard or a custom PCB design. As such, strict pinout compatibility is not as crucial as for HATs and shields, but footprints tend to become more and more common.

| Name                 | Size       | Pitch     | Pins  | Extra Connector
|----------------------|-----------:|----------:|:-----:|----------------
| [Arduino Nano]       | 18×44 mm   | 0.6×0.1”  | 15+15 | SPI (+6)
| [Arduino MKR]        | 25×61 mm   | 0.8×0.1”  | 14+14 | I²C (+5)
| [Adafruit Feather]   | 23×51 mm   | 0.8×0.1”  | 12+16 | Power
| [SparkFun Pro Micro] | 18×33 mm   | 0.6×0.1”  | 12+12 |
| [Raspberry Pi Pico]  | 21×51 mm   | 0.7×0.1”  | 20+20 | SWD (+3)
| [Seeed Studio XIAO]  | 18×20 mm   | 0.6×0.1”  |  7+7  |

[Arduino Nano]:       https://store.arduino.cc/collections/nano-family
[Arduino MKR]:        https://store.arduino.cc/collections/mkr-family
[Raspberry Pi Pico]:  https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html
[Adafruit Feather]:   https://learn.adafruit.com/adafruit-feather/feather-specification
[SparkFun Pro Micro]: https://github.com/sparkfun/Pro_Micro
[Seeed Studio XIAO]:  https://wiki.seeedstudio.com/SeeedStudio_XIAO_Series_Introduction/

The [Arduino Nano] footprint looks like a solid default: very common, implemented by Arduino and STM32 Nucleo, with a lot of different MCU options. The MKR footprint doesn’t seem to raise as much adoption yet.

The [Adafruit Feather] footprint could be an even better standard: many different MCU options and a *lot* of different projects are buit upon it. Even SparkFun favors Feather (under the “Things Plus” name) over its own Pro Micro.

Many other manufacturers have their own footprints; some of those, like the Pico and XIAO, are so popular that they should last long enough for custom designs.

## Connectors

### I²C / Generic

TL;DR: [Qwiic] is a sane default for I²C, all the following options being cross-compatible by changing the connector and/or adapting the signal level.

- 2010: [Seeed Grove][Grove]
- 2014: [Adafruit Stemma][Stemma], free alternative to Grove (only compatible for I²C)
- 2017: [SparkFun Qwiic][Qwiic], smaller I²C connector and 3.3V only
- 2018: [Adafruit Stemma QT][Stemma QT], compatible with Qwiic *and* 5V devices
- 2024: [Arduino Modulino][Modulino], Qwiic connector + fixed board size (25.4×41 mm) + software library

Having a standard connector for Analog/Digital/PWM devices can be interesting as well. The Stemma 3-pin cannot be confused with I²C, which is neat, but there’s no clear standard here.

| Model                    |Connector | Pin Pitch | Voltage     | I²C   | Analog, Digital, PWM
|--------------------------|-------------|-------:|------------:|-------|-------|
| Seeed Studio <br>[Grove] | Proprietary | 2.0 mm | 5 V ‑ 3.3 V | 4‑pin | 4‑pin |
| Adafruit <br>[Stemma]    | JST PH      | 2.0 mm | 5 V ‑ 3.3 V | 4‑pin | 3-pin |
| Adafruit <br>[Stemma QT] | JST SH      | 1.0 mm | 5 V ‑ 3.3 V | 4‑pin |  -    |
| SparkFun <br>[Qwiic]     | JST SH      | 1.0 mm |       3.3 V | 4‑pin |  -    |
| Arduino  <br>[Modulino]  | JST SH      | 1.0 mm |       3.3 V | 4‑pin |  -    |
| DF Robot <br>[Gravity]   | JST PH      | 2.0 mm | 5 V ‑ 3.3 V | 4-pin |  ?    |
| Pimoroni <br>[Breakout Garden]  | ?   | 2.54 mm | 5 V ‑ 3.3 V | 5‑pin |  -    |

[Grove]:     https://wiki.seeedstudio.com/Grove_System/
[Stemma]:    https://learn.adafruit.com/introducing-adafruit-stemma-qt/what-is-stemma
[Stemma QT]: https://learn.adafruit.com/introducing-adafruit-stemma-qt/what-is-stemma-qt
[Qwiic]:     https://www.sparkfun.com/qwiic
[Modulino]:  https://store.arduino.cc/pages/modulino
[Gravity]:   https://www.dfrobot.com/gravity
[Breakout Garden]: https://shop.pimoroni.com/collections/breakouts?tags=Breakout%20Garden

Two options are uncompatible with Qwiic:
- [DF Robot Gravity][Gravity] looks like Stemma but uses a different pin order, beware! To be avoided.
- [Pimoroni Breakout Garden][Breakout Garden] involves a 5-pin connector that reuses part of the Raspberry Pi GPIO connector (pins 1-9), adding an `INT` (interrupt) pin to the 4-pin I²C connector. This can be interesting to avoid polling the I²C bus over and over.

The I²C ecosystem is much more composable than the shield/HAT counterpart.

Sources:
[Tom’s Hardware](https://www.tomshardware.com/features/stemma-vs-qwiic-vs-grove-connectors),
[Adafruit](https://learn.adafruit.com/introducing-adafruit-stemma-qt/stemma-qt-comparison),
[DigiKey](https://www.digikey.com/en/maker/blogs/2022/popular-board-interconnect-systems-and-how-they-benefit-makers),
[Hackaday](https://hackaday.com/2022/05/04/the-connector-zoo-i2c-ecosystems/).

### Flash / Debug

| Architecture | Protocol    | Pins
| ------------ | ----------- | ----
| AVR          | ISP         | 4 wires: MISO, MOSI, SCK, reset (SPI-like)
| AVR          | PDI         | 2 wires: data, clock
| AVR          | DebugWire   | 1 wire
| PIC          | ICSP        |
| ARM Cortex‑M | SWD         | 2 wires: data, clock (Serial Wire Debug)
| TI MSP430    | Spy-Bi-Wire | 2 wires

[JTAG](https://en.wikipedia.org/wiki/JTAG) is an industry standard for verifying PCB designs and testing PCBs after manufacture. It may also be used for programming/debugging MCUs but every vendor has its own protocol: Cortex-M JTAG is not the same as AVR JTAG.
