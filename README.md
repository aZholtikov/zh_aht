# ESP32 ESP-IDF and ESP8266 RTOS SDK component for AHT10/AHT15/AHT20/AHT21/AHT25/AHT30 humidity & temperature sensor

## Tested on

1. ESP8266 RTOS_SDK v3.4
2. ESP32 ESP-IDF v5.2

## Features

1. Autodetect sensor type.

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

void app_main(void)
{
    esp_log_level_set("zh_aht", ESP_LOG_NONE);
#ifdef CONFIG_IDF_TARGET_ESP8266
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_4, // In accordance with used chip.
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = GPIO_NUM_5, // In accordance with used chip.
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
    };
    i2c_driver_install(I2C_PORT, i2c_config.mode);
    i2c_param_config(I2C_PORT, &i2c_config);
#else
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = GPIO_NUM_22, // In accordance with used chip.
        .sda_io_num = GPIO_NUM_21, // In accordance with used chip.
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
#endif
    zh_aht_init_config_t aht_init_config = ZH_AHT_INIT_CONFIG_DEFAULT();
#ifdef CONFIG_IDF_TARGET_ESP8266
    aht_init_config.i2c_port = I2C_PORT;
#else
    aht_init_config.i2c_handle = i2c_bus_handle;
#endif
    zh_aht_init(&aht_init_config);
    float humidity = 0.0;
    float temperature = 0.0;
    for (;;)
    {
        zh_aht_read(&humidity, &temperature);
        printf("Humidity %0.2f\n", humidity);
        printf("Temperature %0.2f\n", temperature);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```

Any [feedback](mailto:github@azholtikov.ru) will be gladly accepted.
