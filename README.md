# ESP32 ESP-IDF component for AHT family humidity & temperature sensor

## Tested on

1. ESP32 ESP-IDF v5.2

## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.

## Features

1. Support of AHT10/AHT15/AHT20/AHT21/AHT25/AHT30.

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

## Example

Reading the sensor:

```c
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

zh_aht_handle_t aht_handle = {0};

void app_main(void)
{
    esp_log_level_set("zh_aht", ESP_LOG_ERROR);
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = GPIO_NUM_22,
        .sda_io_num = GPIO_NUM_21,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus_handle = NULL;
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
    zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT();
    config.i2c_handle = i2c_bus_handle;
    zh_aht_init(&config, &aht_handle);
    float humidity = 0.0;
    float temperature = 0.0;
    for (;;)
    {
        zh_aht_read(&aht_handle, &humidity, &temperature);
        printf("Humidity %0.2f\n", humidity);
        printf("Temperature %0.2f\n", temperature);
        const zh_aht_stats_t *stats = zh_aht_get_stats();
        printf("Number of i2c driver error: %ld.\n", stats->i2c_driver_error);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```
