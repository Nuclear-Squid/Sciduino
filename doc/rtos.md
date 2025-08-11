# RTOS

- bare metal: 
  - no OS at all, handle raw registers and write your own memory allocators

- lightweight / small-footprint:
  - [FreeRTOS]: lightweight, very popular, best-suited for single apps
  - [ChibiOS]: famously used by QMK (along with [lufa] for AVR controllers)
  - [NuttX]
  - [RT-Thread]

- full-featured:
  - [Zephyr]
  - [ThreadX]`*`
  - [VxWorks], proprietary
  - [MbedOS], abandoned (replaced by Zephyr in the Arduino framework)

- Yocto: a build framework for creating custom Linux distributions

| Name        | Governance           | In a word
| ----------- | -------------------- | ------------------
| [FreeRTOS]  | Amazon Web Services  | simplicity
| [NuttX]     | Apache Foundation    | POSIX-compliant
|
| [Zephyr]    | Linux Foundation     | scalability, highest project activity
| [ThreadX]`*`| Eclipse Foundation   | used by STM32Cube
| [VxWorks]   | Wind River Systems   | non-free, safety/security certifications

`*` formerly Microsoft Azure RTOS (bought by MS, then handed over to Eclipse)

[FreeRTOS]:  https://en.wikipedia.org/wiki/FreeRTOS
[lufa]:      https://github.com/qmk/lufa
[ChibiOS]:   https://en.wikipedia.org/wiki/ChibiOS/RT
[Zephyr]:    https://en.wikipedia.org/wiki/Zephyr_(operating_system)
[NuttX]:     https://en.wikipedia.org/wiki/NuttX
[ThreadX]:   https://threadx.io/
[MbedOS]:    https://en.wikipedia.org/wiki/Mbed#Mbed_OS
[VxWorks]:   https://en.wikipedia.org/wiki/VxWorks
[RT-Thread]: https://en.wikipedia.org/wiki/RT-Thread

Sources:
[Hackaday](https://hackaday.com/2021/03/18/getting-started-with-freertos-and-chibios/),
[IIoT World](https://www.iiot-world.com/industrial-iot/connected-industry/freertos-vs-threadx-vs-zephyr-the-fight-for-true-open-source-rtos/),
[Reddit](https://www.reddit.com/r/embedded/comments/1bn85wq/opinion_wanted_whats_the_best_rtos/),
[Medium](https://medium.com/@lanceharvieruntime/small-footprint-big-impact-freertos-nuttx-and-rt-thread-compared-28e332ff42d8),
[PromWad](https://promwad.com/news/choosing-rtos-freertos-zephyr-threadx-comparison),
[Wikipedia](https://en.wikipedia.org/wiki/Comparison_of_real-time_operating_systems),
[OSTROS](https://www.osrtos.com/).

## Alternatives

[Embedded Rust] sits between bare metal and lightweight RTOS:
- [embedded-hal] provides a HAL spec, which can be implemented by stm32, rp2040…
- [rtic] provides concurrency support

[Embedded Rust]: https://docs.rust-embedded.org/book/
[embedded-hal]:  https://github.com/rust-embedded/embedded-hal
[rtic]:          https://rtic.rs
