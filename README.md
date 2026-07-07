# ESP32 ESP-IDF component for AHT family humidity & temperature sensor

## Wiki

[EN](WIKI_EN.md) | [RU](WIKI_RU.md)

## Tested on

1. [ESP32 ESP-IDF v6.0.0](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/index.html)

## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.

## Features

1. Support of AHT10/AHT15/AHT20/AHT21/AHT25/AHT30/AHT40.
2. Support some sensors on one device with [zh_pca9548a](https://github.com/aZholtikov/zh_pca9548a).

## Attention

For correct operation, please enable the following settings in the menuconfig:

```text
I2C_ISR_IRAM_SAFE
I2C_MASTER_ISR_HANDLER_IN_IRAM
```

## Using

In an existing project, run the following command to install the component:

```text
cd ../your_project/components
git clone https://github.com/aZholtikov/zh_aht.git
```

In the application, add the component:

```c
#include "zh_aht.h"
```

## Examples

See Wiki [EN](WIKI_EN.md#usage-examples) | [RU](WIKI_RU.md#примеры-использования)