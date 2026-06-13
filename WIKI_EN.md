# zh_aht - AHT Humidity & Temperature Sensor Component for ESP-IDF

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [API Reference](#api-reference)
- [Usage Examples](#usage-examples)
- [Technical Specifications](#technical-specifications)
- [Error Codes](#error-codes)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

`zh_aht` is a lightweight ESP-IDF component for AHT10/AHT15/AHT20/AHT21/AHT25/AHT30 humidity and temperature sensors. It provides a simple API to read humidity values from 0 to 100% and temperature values from -40 to +85°C. The component supports multiple sensors on a single I2C bus using the [zh_pca9548a](https://github.com/aZholtikov/zh_pca9548a) I2C multiplexer.

The component is designed specifically for ESP32 microcontrollers and uses ESP-IDF v5.0+ I2C driver API.

---

## Features

1. **Humidity and Temperature Measurement**: Reads humidity from 0 to 100% and temperature from -40 to +85°C
2. **I2C Interface**: Uses standard I2C protocol (400 kHz max frequency)
3. **Configurable I2C Address**: Supports both 0x38 and 0x39 addresses
4. **Multiple Model Support**: AHT10, AHT15, AHT20, AHT21, AHT25, AHT30
5. **Error Statistics**: Built-in error counter for I2C driver errors
6. **Low Power**: Works with ESP-IDF power management
7. **Thread-Safe**: Uses ESP-IDF I2C driver (thread-safe)
8. **Minimal Overhead**: Low memory and CPU overhead
9. **Multiple Sensor Support**: Compatible with I2C multiplexer (zh_pca9548a)

---

## Installation

1. Navigate to your project's components directory:

```bash
cd ../your_project/components
```

2. Clone the repository:

```bash
git clone https://github.com/aZholtikov/zh_aht.git
```

3. In your application, include the header:

```c
#include "zh_aht.h"
```

4. The component will be automatically built with your project.

### Required menuconfig Settings

For correct operation, enable the following settings in menuconfig:

```text
I2C_ISR_IRAM_SAFE
I2C_MASTER_ISR_HANDLER_IN_IRAM
```

### Optional: Using with I2C Multiplexer (zh_pca9548a)

To use multiple AHT sensors on the same I2C bus, also install the [zh_pca9548a](https://github.com/aZholtikov/zh_pca9548a) component.

---

## API Reference

### zh_aht_init_config_t Structure

```c
typedef struct
{
    i2c_master_bus_handle_t i2c_handle; // Unique I2C bus handle
    uint8_t i2c_address;                // Sensor I2C address (0x38 or 0x39)
    uint32_t i2c_frequency;             // Sensor I2C frequency (max 400000 Hz)
} zh_aht_init_config_t;
```

Use `ZH_AHT_INIT_CONFIG_DEFAULT()` macro to initialize with default values:

- `i2c_frequency`: 400000 Hz
- `i2c_address`: 0x38

---

### zh_aht_handle_t Structure

```c
typedef struct
{
    bool is_initialized;                // Sensor initialization flag
    i2c_master_dev_handle_t dev_handle; // Unique I2C device handle
} zh_aht_handle_t;
```

---

### zh_aht_stats_t Structure

```c
typedef struct
{
    uint32_t i2c_driver_error; // Number of I2C driver errors
} zh_aht_stats_t;
```

---

### zh_aht_init()

Initializes the AHT sensor.

**Parameters:**

- `config` - Pointer to AHT initialization configuration structure
- `handle` - Pointer to unique AHT handle

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_ARG` - Invalid argument (NULL config or handle)
- `ESP_FAIL` - Initialization failed (configuration check or I2C device addition)

**Example:**

```c
zh_aht_handle_t aht_handle = {0};
zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT();
config.i2c_handle = i2c_bus_handle;
zh_aht_init(&config, &aht_handle);
```

---

### zh_aht_deinit()

Deinitializes the AHT sensor and removes it from the I2C bus.

**Parameters:**

- `handle` - Pointer to unique AHT handle

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_ARG` - Invalid argument (NULL handle)
- `ESP_ERR_INVALID_STATE` - Sensor not initialized
- `ESP_FAIL` - I2C device removal failed

---

### zh_aht_read()

Reads humidity and temperature values from the sensor.

**Parameters:**

- `handle` - Pointer to unique AHT handle
- `humidity` - Pointer to store humidity value (in %)
- `temperature` - Pointer to store temperature value (in °C)

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_ARG` - Invalid argument (NULL handle, humidity or temperature)
- `ESP_ERR_NOT_FOUND` - Sensor not initialized
- `ESP_ERR_TIMEOUT` - Timeout waiting for data ready
- `ESP_ERR_INVALID_CRC` - Invalid CRC checksum
- `ESP_FAIL` - I2C communication error

**Note:** The function performs measurement and waits ~80ms for data ready.

---

### zh_aht_reset()

Resets the AHT sensor.

**Parameters:**

- `handle` - Pointer to unique AHT handle

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_ARG` - Invalid argument (NULL handle)
- `ESP_ERR_NOT_FOUND` - Sensor not initialized
- `ESP_FAIL` - I2C communication error

---

### zh_aht_get_stats()

Gets error statistics since last reset.

**Returns:**

- Pointer to the statistics structure

**Example:**

```c
const zh_aht_stats_t *stats = zh_aht_get_stats();
printf("I2C errors: %ld\n", stats->i2c_driver_error);
```

---

### zh_aht_reset_stats()

Resets error statistics counter.

**Example:**

```c
zh_aht_reset_stats();
```

---

## Usage Examples

### Basic Example: Single Sensor

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
        printf("Humidity: %.2f%%\n", humidity);
        printf("Temperature: %.2f°C\n", temperature);
        
        const zh_aht_stats_t *stats = zh_aht_get_stats();
        printf("I2C errors: %ld\n", stats->i2c_driver_error);
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```

---

### Multiple Sensors: Using I2C Multiplexer (zh_pca9548a)

```c
#include "zh_pca9548a.h"
#include "zh_aht.h"

#define I2C_PORT (I2C_NUM_MAX - 1)

zh_pca9548a_handle_t pca9548a_handle = {0};
zh_aht_handle_t aht_handle_chan_0 = {0};
zh_aht_handle_t aht_handle_chan_1 = {0};
zh_aht_handle_t aht_handle_chan_2 = {0};

void app_main(void)
{
    esp_log_level_set("zh_pca9548a", ESP_LOG_ERROR);
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
    
    // Initialize I2C multiplexer
    zh_pca9548a_init_config_t pca_config = ZH_PCA9548A_INIT_CONFIG_DEFAULT();
    pca_config.i2c_handle = i2c_bus_handle;
    pca_config.i2c_address = 0x70;
    zh_pca9548a_init(&pca_config, &pca9548a_handle);
    
    // Initialize AHT sensors on different channels
    zh_aht_init_config_t aht_config = ZH_AHT_INIT_CONFIG_DEFAULT();
    aht_config.i2c_handle = i2c_bus_handle;
    
    zh_pca9548a_set(&pca9548a_handle, ZH_PCA9548A_CHAN_NUM_0);
    zh_aht_init(&aht_config, &aht_handle_chan_0);
    
    zh_pca9548a_set(&pca9548a_handle, ZH_PCA9548A_CHAN_NUM_1);
    zh_aht_init(&aht_config, &aht_handle_chan_1);
    
    zh_pca9548a_set(&pca9548a_handle, ZH_PCA9548A_CHAN_NUM_2);
    zh_aht_init(&aht_config, &aht_handle_chan_2);
    
    float humidity = 0.0;
    float temperature = 0.0;
    for (;;)
    {
        // Read sensor on channel 0
        zh_pca9548a_set(&pca9548a_handle, ZH_PCA9548A_CHAN_NUM_0);
        zh_aht_read(&aht_handle_chan_0, &humidity, &temperature);
        printf("Sensor 1. Humidity: %.2f%%\n", humidity);
        printf("Sensor 1. Temperature: %.2f°C\n", temperature);
        
        // Read sensor on channel 1
        zh_pca9548a_set(&pca9548a_handle, ZH_PCA9548A_CHAN_NUM_1);
        zh_aht_read(&aht_handle_chan_1, &humidity, &temperature);
        printf("Sensor 2. Humidity: %.2f%%\n", humidity);
        printf("Sensor 2. Temperature: %.2f°C\n", temperature);
        
        // Read sensor on channel 2
        zh_pca9548a_set(&pca9548a_handle, ZH_PCA9548A_CHAN_NUM_2);
        zh_aht_read(&aht_handle_chan_2, &humidity, &temperature);
        printf("Sensor 3. Humidity: %.2f%%\n", humidity);
        printf("Sensor 3. Temperature: %.2f°C\n", temperature);
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```

---

## Technical Specifications

| Parameter | Value |
|-----------|-------|
| **Humidity Range** | 0 - 100% |
| **Temperature Range** | -40 - +85°C |
| **Humidity Resolution** | 0.01% |
| **Temperature Resolution** | 0.01°C |
| **I2C Address** | 0x38, 0x39 |
| **I2C Frequency** | Up to 400 kHz |
| **Measurement Time** | ~80 ms |
| **Supported Models** | AHT10, AHT15, AHT20, AHT21, AHT25, AHT30 |
| **ESP-IDF Version** | >= 5.0 |
| **Platform** | ESP32 series |
| **Language** | C (C99) |

---

## Error Codes

| Error Code | Description |
|------------|-------------|
| `ESP_OK` | Operation successful |
| `ESP_ERR_INVALID_ARG` | Invalid argument (NULL pointer or invalid configuration) |
| `ESP_ERR_INVALID_STATE` | Sensor not initialized |
| `ESP_ERR_NOT_FOUND` | Sensor not initialized or not responding |
| `ESP_ERR_TIMEOUT` | Timeout waiting for data ready |
| `ESP_ERR_INVALID_CRC` | Invalid CRC checksum |
| `ESP_FAIL` | General failure (I2C communication error) |

---

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

Please ensure your code follows the existing style and includes appropriate documentation.

---

## License

This project is licensed under the Apache License, Version 2.0 - see the [LICENSE](LICENSE) file for details.

### Apache License, Version 2.0

Copyright (c) 2026 Alexey Zholtikov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

---

## Additional Notes

- **I2C Pull-up Resistors**: Ensure proper pull-up resistors are connected to SDA and SCL lines
- **Measurement Time**: The sensor requires ~80ms for each measurement
- **I2C_ISR_IRAM_SAFE**: For correct operation, enable `I2C_ISR_IRAM_SAFE` and `I2C_MASTER_ISR_HANDLER_IN_IRAM` in menuconfig
- **Thread Safety**: The component uses ESP-IDF I2C driver which is thread-safe

---

*Generated for zh_aht v3.4.0*
