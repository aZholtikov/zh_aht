# ESP32 ESP-IDF component for AHT series humidity & temperature sensor (I2C)

## Features

1. Support for the whole AHT family including AHT10, AHT15, AHT20, AHT21, AHT25, AHT30 and AHT40.
2. Measurement of relative humidity in range 0 to 100 percent and temperature in range -50 to 150 degrees Celsius.
3. Data integrity verification with CRC8 algorithm on every measurement frame.
4. Automatic sensor detection and initialization during setup.
5. Software reset of the sensor over I2C.
6. Tracking of I2C driver error statistics with a resettable counter.
7. I2C clock frequency up to 400000 Hz on addresses 0x38, 0x39 and 0x44.
8. Multiple sensors on one bus through external multiplexer zh_pca9548a.

## Attention

1. The component is not thread safe, one sensor handle must be served by a single task.
2. I2C bus is created by the user and the component does not manage it.
3. Statistics counters are global and shared between all sensor instances.
4. Read and reset functions contain blocking delays and cannot be called from a context without task switching.

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

## Examples

### Basic Example: Read Humidity and Temperature

```c
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

i2c_master_bus_handle_t i2c_bus_handle = NULL;
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
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
    zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT();
    config.i2c_handle = i2c_bus_handle;
    esp_err_t ret = zh_aht_init(&config, &aht_handle);
    if (ret != ESP_OK)
    {
        printf("AHT initialization error: %s\n", esp_err_to_name(ret));
        return;
    }
    float humidity = 0.0;
    float temperature = 0.0;
    ret = zh_aht_read(&aht_handle, &humidity, &temperature);
    if (ret == ESP_OK)
    {
        printf("Humidity: %.2f%%, Temperature: %.2f°C\n", humidity, temperature);
    }
    else
    {
        printf("Read error: %s\n", esp_err_to_name(ret));
    }
    zh_aht_deinit(&aht_handle);
}
```

---

### Read In A Loop Example

```c
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

i2c_master_bus_handle_t i2c_bus_handle = NULL;
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
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
    zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT();
    config.i2c_handle = i2c_bus_handle;
    esp_err_t ret = zh_aht_init(&config, &aht_handle);
    if (ret != ESP_OK)
    {
        printf("AHT initialization error: %s\n", esp_err_to_name(ret));
        return;
    }
    float humidity = 0.0;
    float temperature = 0.0;
    for (int i = 0; i < 10; i++)
    {
        ret = zh_aht_read(&aht_handle, &humidity, &temperature);
        if (ret == ESP_OK)
        {
            printf("Humidity: %.2f%%, Temperature: %.2f°C\n", humidity, temperature);
        }
        else
        {
            printf("Read error: %s\n", esp_err_to_name(ret));
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    zh_aht_deinit(&aht_handle);
}
```

---

### Reset Sensor Example

```c
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

i2c_master_bus_handle_t i2c_bus_handle = NULL;
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
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
    zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT();
    config.i2c_handle = i2c_bus_handle;
    esp_err_t ret = zh_aht_init(&config, &aht_handle);
    if (ret != ESP_OK)
    {
        printf("AHT initialization error: %s\n", esp_err_to_name(ret));
        return;
    }
    ret = zh_aht_reset(&aht_handle);
    if (ret == ESP_OK)
    {
        printf("Sensor reset ok\n");
    }
    else
    {
        printf("Reset error: %s\n", esp_err_to_name(ret));
    }
    float humidity = 0.0;
    float temperature = 0.0;
    ret = zh_aht_read(&aht_handle, &humidity, &temperature);
    if (ret == ESP_OK)
    {
        printf("Humidity: %.2f%%, Temperature: %.2f°C\n", humidity, temperature);
    }
    zh_aht_deinit(&aht_handle);
}
```

---

### Error Statistics Example

```c
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

i2c_master_bus_handle_t i2c_bus_handle = NULL;
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
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
    zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT();
    config.i2c_handle = i2c_bus_handle;
    esp_err_t ret = zh_aht_init(&config, &aht_handle);
    if (ret != ESP_OK)
    {
        printf("AHT initialization error: %s\n", esp_err_to_name(ret));
        return;
    }
    float humidity = 0.0;
    float temperature = 0.0;
    for (int i = 0; i < 10; i++)
    {
        zh_aht_read(&aht_handle, &humidity, &temperature);
    }
    const zh_aht_stats_t *stats = zh_aht_get_stats();
    printf("I2C errors: %lu\n", (unsigned long)stats->i2c_driver_error);
    zh_aht_reset_stats();
    stats = zh_aht_get_stats();
    printf("I2C errors after reset: %lu\n", (unsigned long)stats->i2c_driver_error);
    zh_aht_deinit(&aht_handle);
}
```
