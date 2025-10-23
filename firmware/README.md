# sqıd Firmware Reference Implementation

This directory contains a C++ reference implementation of the sqıd serial protocol for single-value, array-, and matrix-shaped data. It was created with the VS Code extension [PlatformIO](https://platformio.org/) (PIO) using the Arduino framework and is currently configured for the [Espressif32](https://github.com/platformio/platform-espressif32) platform targeted for [HUZZAH32 ESP32 Feather](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather) boards. Platform and target board can easily be modified using PIO. The Arduino dependency is mostly due to the `Serial` implementation of RS232, which can also easily be replaced. Anyways, most crucial here is the implementation of sqid protocol, implemented as a library located in [lib/sqid/](./lib/sqid/), hence, it can easily be [installed for the Arduino IDE](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries/#manual-installation), or be used with other environments.

## Dependencies

- lz4 (tested with 1.8.3) [[link](https://lz4.github.io/lz4/)]
- minilzo (tested with 2.10) [[link](http://www.oberhumer.com/opensource/lzo/)]
- zstd (tested with 1.4.0) [[link](https://github.com/facebook/zstd)]

## License

Copyright (C) 2025 eyeco

sqid Visual Programming Environment is licensed under the GPL3 License. See LICENSE file in the package root for license information.
