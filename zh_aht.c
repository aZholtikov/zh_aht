#include "zh_aht.h"

#define I2C_MAX_DATA_SIZE 7                    // Sensor maximum data size (in bytes).
#define MEASUREMENT_TIME 80                    // Sensor measurement time (in milliseconds).
#define RESET_TIME 20                          // Sensor reset time (in milliseconds).
#define I2C_DATA_READ_COMMAND 0xAC, 0x33, 0x00 // Command for read sensor data (temperature and humidity).
#define I2C_RESET_COMMAND 0xBA                 // Command for reset sensor.
#define I2C_INIT_COMMAND 0x00, 0x08, 0x00      // Command for initialize sensor. First byte is depend of sensor type.
#define I2C_INIT_AHT1X_FIRST_BYTE 0xE1         // First byte for command for initialize sensor. For AHT1X series.
#define I2C_INIT_AHT2X_FIRST_BYTE 0xBE         // First byte for command for initialize sensor. For AHT2X/3X series.
#define I2C_STATUS_READ_COMMAND 0x71           // Command for read sensor status.

static zh_aht_init_config_t _init_config = {0};
static bool _is_initialized = false;
#ifndef CONFIG_IDF_TARGET_ESP8266
static i2c_master_dev_handle_t _aht_handle = {0};
#endif

static const char *TAG = "zh_aht";

static uint8_t _calc_crc(const uint8_t *buf, size_t len);

esp_err_t zh_aht_init(const zh_aht_init_config_t *config)
{
    ESP_LOGI(TAG, "AHT initialization begin.");
    if (config == NULL)
    {
        ESP_LOGE(TAG, "AHT initialization fail. Invalid argument.");
        return ESP_ERR_INVALID_ARG;
    }
    _init_config = *config;
#ifndef CONFIG_IDF_TARGET_ESP8266
    i2c_device_config_t aht_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = _init_config.i2c_address,
        .scl_speed_hz = 100000,
    };
    i2c_master_bus_add_device(_init_config.i2c_handle, &aht_config, &_aht_handle);
    if (i2c_master_probe(_init_config.i2c_handle, _init_config.i2c_address, 1000 / portTICK_PERIOD_MS) != ESP_OK)
    {
        ESP_LOGE(TAG, "AHT initialization fail. Sensor not connected or not responded.");
        return ESP_ERR_NOT_FOUND;
    }
#endif
    esp_err_t esp_err = ESP_OK;
    uint8_t command = I2C_STATUS_READ_COMMAND;
    uint8_t sensor_data = 0;
#ifdef CONFIG_IDF_TARGET_ESP8266
    i2c_cmd_handle_t i2c_cmd_handle = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd_handle);
    i2c_master_write_byte(i2c_cmd_handle, _init_config.i2c_address << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd_handle, command, true);
    i2c_master_stop(i2c_cmd_handle);
    esp_err = i2c_master_cmd_begin(_init_config.i2c_port, i2c_cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_cmd_handle);
#else
    esp_err = i2c_master_transmit(_aht_handle, &command, sizeof(command), 1000 / portTICK_PERIOD_MS);
#endif
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "AHT initialization fail. I2C driver error at line %d.", __LINE__);
        return ESP_ERR_INVALID_RESPONSE;
    }
#ifdef CONFIG_IDF_TARGET_ESP8266
    i2c_cmd_handle = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd_handle);
    i2c_master_write_byte(i2c_cmd_handle, _init_config.i2c_address << 1 | I2C_MASTER_READ, true);
    i2c_master_read_byte(i2c_cmd_handle, &sensor_data, I2C_MASTER_NACK);
    i2c_master_stop(i2c_cmd_handle);
    esp_err = i2c_master_cmd_begin(_init_config.i2c_port, i2c_cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_cmd_handle);
#else
    esp_err = i2c_master_receive(_aht_handle, &sensor_data, sizeof(sensor_data), 1000 / portTICK_PERIOD_MS);
#endif
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "AHT initialization fail. I2C driver error at line %d.", __LINE__);
        return ESP_ERR_INVALID_RESPONSE;
    }
    if ((sensor_data & 0x08) == 0) // If sensor is not calibrated.
    {
        uint8_t command[] = {I2C_INIT_COMMAND};
        if (_init_config.sensor_type == ZH_AHT1X)
        {
            command[0] = I2C_INIT_AHT1X_FIRST_BYTE;
        }
        else
        {
            command[0] = I2C_INIT_AHT2X_FIRST_BYTE;
        }
#ifdef CONFIG_IDF_TARGET_ESP8266
        i2c_cmd_handle_t i2c_cmd_handle = i2c_cmd_link_create();
        i2c_master_start(i2c_cmd_handle);
        i2c_master_write_byte(i2c_cmd_handle, _init_config.i2c_address << 1 | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(i2c_cmd_handle, command[0], true);
        i2c_master_write_byte(i2c_cmd_handle, command[1], true);
        i2c_master_write_byte(i2c_cmd_handle, command[2], true);
        i2c_master_stop(i2c_cmd_handle);
        esp_err = i2c_master_cmd_begin(_init_config.i2c_port, i2c_cmd_handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(i2c_cmd_handle);
#else
        esp_err = i2c_master_transmit(_aht_handle, command, sizeof(command), 1000 / portTICK_PERIOD_MS);
#endif
        if (esp_err != ESP_OK)
        {
            ESP_LOGE(TAG, "AHT initialization fail. I2C driver error at line %d.", __LINE__);
            return ESP_ERR_INVALID_RESPONSE;
        }
    }
    ESP_LOGI(TAG, "AHT initialization success.");
    _is_initialized = true;
    return ESP_OK;
}

esp_err_t zh_aht_read(float *humidity, float *temperature)
{
    ESP_LOGI(TAG, "AHT read begin.");
    if (humidity == NULL || temperature == NULL)
    {
        ESP_LOGE(TAG, "AHT read fail. Invalid argument.");
        return ESP_ERR_INVALID_ARG;
    }
    if (_is_initialized == false)
    {
        ESP_LOGE(TAG, "AHT read fail. AHT not initialized.");
        return ESP_ERR_NOT_FOUND;
    }
    esp_err_t esp_err = ESP_OK;
    uint8_t sensor_data[I2C_MAX_DATA_SIZE] = {0};
    uint8_t command[] = {I2C_DATA_READ_COMMAND};
#ifdef CONFIG_IDF_TARGET_ESP8266
    i2c_cmd_handle_t i2c_cmd_handle = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd_handle);
    i2c_master_write_byte(i2c_cmd_handle, _init_config.i2c_address << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd_handle, command[0], true);
    i2c_master_write_byte(i2c_cmd_handle, command[1], true);
    i2c_master_write_byte(i2c_cmd_handle, command[2], true);
    i2c_master_stop(i2c_cmd_handle);
    esp_err = i2c_master_cmd_begin(_init_config.i2c_port, i2c_cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_cmd_handle);
#else
    esp_err = i2c_master_transmit(_aht_handle, command, sizeof(command), 1000 / portTICK_PERIOD_MS);
#endif
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "AHT read fail. I2C driver error at line %d.", __LINE__);
        return ESP_ERR_INVALID_RESPONSE;
    }
    vTaskDelay(MEASUREMENT_TIME / portTICK_PERIOD_MS);
#ifdef CONFIG_IDF_TARGET_ESP8266
    i2c_cmd_handle = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd_handle);
    i2c_master_write_byte(i2c_cmd_handle, _init_config.i2c_address << 1 | I2C_MASTER_READ, true);
    for (uint8_t i = 0; i < sizeof(sensor_data); ++i)
    {
        i2c_master_read_byte(i2c_cmd_handle, &sensor_data[i], i == (sizeof(sensor_data) - 1) ? I2C_MASTER_NACK : I2C_MASTER_ACK);
    }
    i2c_master_stop(i2c_cmd_handle);
    esp_err = i2c_master_cmd_begin(_init_config.i2c_port, i2c_cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_cmd_handle);
#else
    esp_err = i2c_master_receive(_aht_handle, sensor_data, sizeof(sensor_data), 1000 / portTICK_PERIOD_MS);
#endif
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "AHT read fail. I2C driver error at line %d.", __LINE__);
        return ESP_ERR_INVALID_RESPONSE;
    }
    if ((sensor_data[0] & 0x40) != 0) // If sensor is busy in measurement.
    {
        ESP_LOGE(TAG, "AHT read fail. Timeout exceeded.");
        return ESP_ERR_TIMEOUT;
    }
    if (_init_config.sensor_type != ZH_AHT1X)
    {
        if (_calc_crc(sensor_data, I2C_MAX_DATA_SIZE - 1) != sensor_data[6])
        {
            ESP_LOGE(TAG, "AHT read fail. Invalid CRC.");
            return ESP_ERR_INVALID_CRC;
        }
    }
    *humidity = (((((uint32_t)sensor_data[1]) << 16) | (((uint32_t)sensor_data[2]) << 8) | (((uint32_t)sensor_data[3]) << 0)) >> 4) / 1048576.0 * 100.0;
    *temperature = (((((uint32_t)sensor_data[3]) << 16) | (((uint32_t)sensor_data[4]) << 8) | (((uint32_t)sensor_data[5]) << 0)) & 0xFFFFF) / 1048576.0 * 200.0 - 50.0;
    ESP_LOGI(TAG, "AHT read success.");
    return ESP_OK;
}

esp_err_t zh_aht_reset(void)
{
    ESP_LOGI(TAG, "AHT reset begin.");
    if (_is_initialized == false)
    {
        ESP_LOGE(TAG, "AHT reset fail. AHT not initialized.");
        return ESP_ERR_NOT_FOUND;
    }
    esp_err_t esp_err = ESP_OK;
    uint8_t command = I2C_RESET_COMMAND;
#ifdef CONFIG_IDF_TARGET_ESP8266
    i2c_cmd_handle_t i2c_cmd_handle = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd_handle);
    i2c_master_write_byte(i2c_cmd_handle, _init_config.i2c_address << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd_handle, command, true);
    i2c_master_stop(i2c_cmd_handle);
    esp_err = i2c_master_cmd_begin(_init_config.i2c_port, i2c_cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(i2c_cmd_handle);
#else
    esp_err = i2c_master_transmit(_aht_handle, &command, sizeof(command), 1000 / portTICK_PERIOD_MS);
#endif
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "AHT reset fail. I2C driver error at line %d.", __LINE__);
        return ESP_ERR_INVALID_RESPONSE;
    }
    vTaskDelay(RESET_TIME / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "AHT reset success.");
    return ESP_OK;
}

static uint8_t _calc_crc(const uint8_t *buf, size_t len)
{
    uint8_t crc = 0xFF;
    for (uint8_t byte = 0; byte < len; byte++)
    {
        crc ^= buf[byte];
        for (uint8_t i = 8; i > 0; --i)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc;
}