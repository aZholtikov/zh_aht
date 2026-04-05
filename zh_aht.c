#include "zh_aht.h"

static const char *TAG = "zh_aht";

#define ZH_LOGI(msg, ...) ESP_LOGI(TAG, msg, ##__VA_ARGS__)
#define ZH_LOGE(msg, err, ...) ESP_LOGE(TAG, "[%s:%d:%s] " msg, __FILE__, __LINE__, esp_err_to_name(err), ##__VA_ARGS__)

#define ZH_ERROR_CHECK(cond, err, cleanup, msg, ...) \
    if (!(cond))                                     \
    {                                                \
        ZH_LOGE(msg, err, ##__VA_ARGS__);            \
        cleanup;                                     \
        return err;                                  \
    }

static const uint8_t _aht_reset_command = 0xBA;
static const uint8_t _aht_read_command[] = {0xAC, 0x33, 0x00};
static const uint8_t _aht_init_command[] = {0xBE, 0x08, 0x00};
static zh_aht_stats_t _stats = {0};

static esp_err_t _zh_aht_validate_config(const zh_aht_init_config_t *config);
static esp_err_t _zh_aht_i2c_init(const zh_aht_init_config_t *config, zh_aht_handle_t *handle);
static uint8_t _zh_aht_calc_crc(const uint8_t *buf, size_t len);

esp_err_t zh_aht_init(const zh_aht_init_config_t *config, zh_aht_handle_t *handle)
{
    ZH_LOGI("AHT initialization begin.");
    ZH_ERROR_CHECK(config != NULL && handle != NULL, ESP_ERR_INVALID_ARG, NULL, "AHT initialization failed. Invalid argument.");
    ZH_ERROR_CHECK(_zh_aht_validate_config(config) == ESP_OK, ESP_FAIL, NULL, "AHT initialization failed. Initial configuration check failed.");
    ZH_ERROR_CHECK(_zh_aht_i2c_init(config, handle) == ESP_OK, ESP_FAIL, NULL, "AHT initialization failed. Failed to add I2C device.");
    handle->is_initialized = true;
    ZH_LOGI("AHT initialization completed successfully.");
    return ESP_OK;
}

esp_err_t zh_aht_read(zh_aht_handle_t *handle, float *humidity, float *temperature) // -V2008
{
    ZH_LOGI("AHT read begin.");
    ZH_ERROR_CHECK(humidity != NULL && temperature != NULL && handle != NULL, ESP_ERR_INVALID_ARG, NULL, "AHT read fail. Invalid argument.");
    ZH_ERROR_CHECK(handle->is_initialized == true, ESP_ERR_NOT_FOUND, NULL, "AHT read fail. AHT not initialized.");
    uint8_t sensor_data[7] = {0};
    ZH_ERROR_CHECK(i2c_master_transmit(handle->dev_handle, _aht_read_command, sizeof(_aht_read_command), 1000 / portTICK_PERIOD_MS) == ESP_OK, ESP_FAIL,
                   ++_stats.i2c_driver_error, "AHT read fail. I2C driver error.");
    vTaskDelay(80 / portTICK_PERIOD_MS);
    ZH_ERROR_CHECK(i2c_master_receive(handle->dev_handle, sensor_data, sizeof(sensor_data), 1000 / portTICK_PERIOD_MS) == ESP_OK, ESP_FAIL,
                   ++_stats.i2c_driver_error, "AHT read fail. I2C driver error.");
    ZH_ERROR_CHECK((sensor_data[0] & 0x80) == 0, ESP_ERR_TIMEOUT, NULL, "AHT read fail. Timeout exceeded.");
    ZH_ERROR_CHECK(_zh_aht_calc_crc(sensor_data, 6) == sensor_data[6], ESP_ERR_INVALID_CRC, NULL, "AHT read fail. Invalid CRC.");
    *humidity = (((((uint32_t)sensor_data[1]) << 16) | (((uint32_t)sensor_data[2]) << 8) | (((uint32_t)sensor_data[3]) << 0)) >> 4) / 1048576.0 * 100.0;
    *temperature = (((((uint32_t)sensor_data[3]) << 16) | (((uint32_t)sensor_data[4]) << 8) | (((uint32_t)sensor_data[5]) << 0)) & 0xFFFFF) / 1048576.0 * 200.0 - 50.0;
    ZH_LOGI("AHT read completed successfully.");
    return ESP_OK;
}

esp_err_t zh_aht_reset(zh_aht_handle_t *handle)
{
    ZH_LOGI("AHT reset begin.");
    ZH_ERROR_CHECK(handle != NULL, ESP_ERR_INVALID_ARG, NULL, "AHT reset fail. Invalid argument.");
    ZH_ERROR_CHECK(handle->is_initialized == true, ESP_ERR_NOT_FOUND, NULL, "AHT reset fail. AHT not initialized.");
    ZH_ERROR_CHECK(i2c_master_transmit(handle->dev_handle, &_aht_reset_command, sizeof(_aht_reset_command), 1000 / portTICK_PERIOD_MS) == ESP_OK, ESP_FAIL,
                   ++_stats.i2c_driver_error, "AHT reset fail. I2C driver error.");
    vTaskDelay(20 / portTICK_PERIOD_MS);
    ZH_LOGI("AHT reset completed successfully.");
    return ESP_OK;
}

const zh_aht_stats_t *zh_aht_get_stats(void)
{
    return &_stats;
}

void zh_aht_reset_stats(void)
{
    ZH_LOGI("Error statistic reset started.");
    _stats.i2c_driver_error = 0;
    ZH_LOGI("Error statistic reset successfully.");
}

static esp_err_t _zh_aht_validate_config(const zh_aht_init_config_t *config)
{
    ZH_ERROR_CHECK(config->i2c_address == 0x38 || config->i2c_address == 0x39, ESP_ERR_INVALID_ARG, NULL, "Invalid I2C address.");
    ZH_ERROR_CHECK(config->i2c_frequency <= 400000, ESP_ERR_INVALID_ARG, NULL, "Invalid I2C frequency.");
    ZH_ERROR_CHECK(config->i2c_handle != NULL, ESP_ERR_INVALID_ARG, NULL, "Invalid I2C handle.");
    return ESP_OK;
}

static esp_err_t _zh_aht_i2c_init(const zh_aht_init_config_t *config, zh_aht_handle_t *handle) // -V2008
{
    i2c_device_config_t aht_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->i2c_address,
        .scl_speed_hz = config->i2c_frequency,
    };
    i2c_master_dev_handle_t _dev_handle = NULL;
    ZH_ERROR_CHECK(i2c_master_bus_add_device(config->i2c_handle, &aht_config, &_dev_handle) == ESP_OK, ESP_FAIL, NULL, "Add I2C device failed.");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ZH_ERROR_CHECK(i2c_master_probe(config->i2c_handle, config->i2c_address, 1000 / portTICK_PERIOD_MS) == ESP_OK, ESP_ERR_NOT_FOUND,
                   ZH_ERROR_CHECK(i2c_master_bus_rm_device(_dev_handle) == ESP_OK, ESP_FAIL, NULL, "I2C remove device failed."), "Sensor not connected or not responded.");
    uint8_t sensor_data = 0;
    ZH_ERROR_CHECK(i2c_master_receive(_dev_handle, &sensor_data, sizeof(sensor_data), 1000 / portTICK_PERIOD_MS) == ESP_OK, ESP_FAIL,
                   ZH_ERROR_CHECK(i2c_master_bus_rm_device(_dev_handle) == ESP_OK, ESP_FAIL, NULL, "I2C remove device failed."), "I2C driver error.");
    if ((sensor_data & 0x08) == 0)
    {
        ZH_ERROR_CHECK(i2c_master_transmit(_dev_handle, _aht_init_command, sizeof(_aht_init_command), 1000 / portTICK_PERIOD_MS) == ESP_OK, ESP_FAIL,
                       ZH_ERROR_CHECK(i2c_master_bus_rm_device(_dev_handle) == ESP_OK, ESP_FAIL, NULL, "I2C remove device failed."), "I2C driver error.");
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    handle->dev_handle = _dev_handle;
    return ESP_OK;
}

static uint8_t _zh_aht_calc_crc(const uint8_t *buf, size_t len)
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