# MCU Chips

## Architectures

### PIC

Developed by Microchip since 1976.

8-bit Harvard architecture.

Mostly obsolete nowadays. Not to be confused with PIC32M (MIPS), PIC32C (ARM) and PIC64 (ARM or RISC‑V), which reuse the PIC trademark for totally different architectures.

### AVR

Developed by Atmel since 1996, acquired by Microchip Technology in 2016.

8-bit RISC single-chip architecture.

- ATtiny series, 1.6-20 MHz, 0.5-32 KB flash
- ATmega series, 1.6-20 MHz, 4-256 KB flash
- Dx series (signal conditioning), 20-24 MHz, 16-128 KB flash

Mostly obsolete compared to ARM, but still suitable for simple use cases.

### ARM Cortex‑M

Licensed by ARM Limited.

32-bit RISC ARM processor cores.

| Name          | Achitecture | Notes
| ------------- | ----------- | -----
| [Cortex-M0]   | ARMv6-M     | small footprint, low price
| [Cortex-M0+]  | ARMv6-M     | superset of M0, low power
| [Cortex-M1]   | ARMv6-M     | designed to be loaded into FPGA chips
| [Cortex-M3]   | ARMv7-M     |
| [Cortex-M4]   | ARMv7E-M    | DSP extension (MAC, SIMD) + optional FPU
| [Cortex-M7]   | ARMv7E-M    | high-performance, branch speculation
| [Cortex-M23]  | ARMv8-M     | M0+ with TrustZone security features
| [Cortex-M33]  | ARMv8-M     | M4 with TrustZone security features
| [Cortex-M35P] | ARMv8-M     | M33 with a new instruction cache
| [Cortex-M52]  | ARMv8.1-M   | M33/M55 cross-over
| [Cortex-M55]  | ARMv8.1-M   |
| [Cortex-M85]  | ARMv8.1-M   |

[Cortex-M0]:   https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M0
[Cortex-M0+]:  https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M0+
[Cortex-M1]:   https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M1
[Cortex-M3]:   https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M3
[Cortex-M4]:   https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M4
[Cortex-M7]:   https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M7
[Cortex-M23]:  https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M23
[Cortex-M33]:  https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M33
[Cortex-M35P]: https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M35P
[Cortex-M52]:  https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M52
[Cortex-M55]:  https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M55
[Cortex-M85]:  https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M85

### RISC‑V

32-bit RISC architecture, open-source (CC/BSD), initiated at Berkeley. Handled by [RISC-V International](https://riscv.org/), a Swiss non-profit entity.

Still very new, but likely to become the next reference architecture for MCUs. Common RISC-V chips:

- Espressif ESP32-C3
- GigaDevice GD32 series
- Renesas R9A02G021
- Raspberry Pi RP2350 (two RISC‑V cores)
- SiFive FE310


## Best-Sellers

### STM32

Single- and dual-core, based on 32-bit Cortex‑M architectures.

Among the [huge product range](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html "huge product range"), two classes are easy to remember:

- **Cortex‑M0+** for low-cost versions: F0, L0, G0, C0 series
- **Cortex‑M7** for high-performance versions: F7, H7 series

Warning, the STM32 software ecosystem is not open-source: many key components are released under the [SLA0044 licence](https://www.st.com/resource/en/license/SLA0048_STM32CubeIDE.pdf "SLA0044 licence"), which is zero-cost but non-free. Besides, the IDE (Cube) is based on an old version of Eclipse, which might be an issue for modern IDE users.

The free, community-driven [STM32duino](https://github.com/stm32duino "STM32duino") tools can be used instead, adding STM32 support to the Arduino IDE and CLI.

### Other Cortex-M Controllers

Popular among makers:

- Atmel SAM:
  - [SAM D] series: Cortex-M0+ (e.g. [SAMD21])
  - SAM 3 series: Cortex-M3
  - SAM 4 series: Cortex-M4F
  - SAMx7 series: Cortex-M7
- [Renesas RA] series:
  - RA4M1: Cortex‑M4, segment LCD controller + capacitive sensor, commonly used for HMI designs
  - [e² studio](https://www.renesas.com/en/software-tool/e-studio): Eclipse-based IDE
  - [CS](https://www.renesas.com/en/software-tool/cs): user-friendly IDE

[SAM D]:  https://www.microchip.com/en-us/products/microcontrollers/32-bit-mcus/pic32-sam/sam-d
[SAMD21]:        https://ww1.microchip.com/downloads/en/DeviceDoc/SAM-D21DA1-Family-Data-Sheet-DS40001882G.pdf
[SAM3X]:         https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-11057-32-bit-Cortex-M3-Microcontroller-SAM3X-SAM3A_Datasheet.pdf
[Renesas RA]:    https://www.renesas.com/en/products/microcontrollers-microprocessors/ra-cortex-m-mcus
[Renesas RA4M1]: https://www.renesas.com/en/products/ra4m1

Popular IoT ontrollers with wireless support: Bluetooth [LE]/[Mesh]/[NLC], [Thread], [Zigbee], [Matter]…

- [Nordic nRF52] series: Cortex‑M4
  - [nRF52840] is the popular, high-performance variant
- [Silicon Labs MG24][]: Cortex‑M33

[Nordic nRF52]:      https://docs.nordicsemi.com/category/nrf-52-series
[Silicon Labs MG24]: https://www.silabs.com/wireless/zigbee/efr32mg24-series-2-socs

[nRF52840]: https://www.nordicsemi.com/Products/nRF52840
[MGM240S]:  https://www.silabs.com/documents/public/data-sheets/mgm240s-datasheet.pdf

[LE]:     https://en.wikipedia.org/wiki/Bluetooth_Low_Energy
[Mesh]:   https://en.wikipedia.org/wiki/Bluetooth_mesh_networking
[NLC]:    https://www.bluetooth.com/learn-about-bluetooth/use-cases/lighting-control/
[Thread]: https://www.threadgroup.org/
[Zigbee]: https://csa-iot.org/all-solutions/zigbee/
[Matter]: https://csa-iot.org/all-solutions/matter/

### Raspberry Pi

Dual-core and low-cost. Based on 32-bit Cortex‑M and RISC‑V architectures.

- [RP2040]: 133 MHz dual Cortex‑M0+
- [RP2350]: 150 MHz dual Cortex‑M33 and dual [Hazard3 RISC-V] <br>
  — software selectable, only two active cores at a time

[RP2040]: https://www.raspberrypi.com/products/rp2040/
[RP2350]: https://www.raspberrypi.com/products/rp2350/

Killer feature: the [PIO subsystem] enables software implementations of protocols such as SDIO, DPI, I²S, DVI-D. It can also be used to enable additional UART ports. There are 8 programmable I/O pins on the RP2040, 12 on the RP2350.

Possible performance bottlenecks:
- maximum of 16 MB off-chip Flash memory via QSPI;
- USB 1.1 controller (12 Mbits/s).

[Compared to STM32], RP2040 and RP2350 are “one size fits all” solutions that can be operated with various free software tools, whereas STM32 is more of a walled garden of hundreds of specialized chips, centered around STM32Cube.

[Hazard3 RISC-V]:    https://www.raspberrypi.com/news/risc-v-on-raspberry-pi-pico-2/
[PIO subsystem]:     https://blog.ploopy.co/rp2040-the-little-engine-that-could-96
[Compared to STM32]: https://toxigon.com/raspberry-pi-rp2040-hands-on-experiences-from-an-stm32-perspective

### Espressif ESP32

Low-cost and wireless (Wi‑Fi and Bluetooth):
- S-series are based on a high-performance [Xtensa LX] architecture, well suited for AI and audio applications;
- C-series are based on a low-consumption RISC-V architecture.

[ESP32-C3] is based on a single-core, 32-bit RISC-V architecture. It seems to be the most popular RISC‑V MCU these days.

[XTensa LX]: https://en.wikipedia.org/wiki/Tensilica
[ESP32-C3]:  https://www.espressif.com/en/products/socs/esp32-c3
