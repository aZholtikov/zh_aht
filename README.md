# ESP32 ESP-IDF component for AHT Series Humidity & Temperature Sensor

## Features

1. Support of AHT10/AHT15/AHT20/AHT21/AHT25/AHT30/AHT40.
2. Support multiple sensors on one device with [zh_pca9548a](https://github.com/aZholtikov/zh_pca9548a).
3. CRC8 data integrity verification.
4. Error statistics tracking.

## Using

In an existing project, run the following command to install the component:

```bash
cd ../your_project/components
git clone https://github.com/aZholtikov/zh_aht
```

In the application, add the component:

```c
#include "zh_aht.h"
```

## Example

```c
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

zh_aht_handle_t *aht_handle = NULL;

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
        printf("Humidity: %.2f%%\n", humidity);
        printf("Temperature: %.2f°C\n", temperature);
        const zh_aht_stats_t *stats = zh_aht_get_stats();
        printf("I2C errors: %ld\n", stats->i2c_driver_error);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```
