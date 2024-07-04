#pragma once

#include "esp_err.h"
#include "esp_log.h"
#ifdef CONFIG_IDF_TARGET_ESP8266
#include "driver/i2c.h"
#else
#include "driver/i2c_master.h"
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ZH_AHT_INIT_CONFIG_DEFAULT() \
    {                                \
        .i2c_port = 0                \
    }

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct // Structure for initial initialization of AHT sensor.
    {
        bool i2c_port; // I2C port.
#ifndef CONFIG_IDF_TARGET_ESP8266
        i2c_master_bus_handle_t i2c_handle; // Unique I2C bus handle.
#endif

    } zh_aht_init_config_t;

    /**
     * @brief Initialize AHT sensor.
     *
     * @param[in] config Pointer to AHT initialized configuration structure. Can point to a temporary variable.
     *
     * @attention I2C driver must be initialized first.
     *
     * @note Before initialize the sensor recommend initialize zh_aht_init_config_t structure with default values.
     *
     * @code zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT() @endcode
     *
     * @return
     *              - ESP_OK if initialization was success
     *              - ESP_ERR_INVALID_ARG if parameter error
     *              - ESP_ERR_NOT_FOUND if sensor not connected or not responded
     *              - ESP_ERR_INVALID_RESPONSE if I2C driver error
     */
    esp_err_t zh_aht_init(const zh_aht_init_config_t *config);

    /**
     * @brief Read AHT sensor.
     *
     * @param[out] humidity Pointer for AHT sensor reading data of humidity.
     * @param[out] temperature Pointer for AHT sensor reading data of temperature.
     *
     * @return
     *              - ESP_OK if read was success
     *              - ESP_ERR_INVALID_ARG if parameter error
     *              - ESP_ERR_NOT_FOUND if sensor is not initialized
     *              - ESP_ERR_INVALID_CRC if check CRC is fail
     *              - ESP_ERR_INVALID_RESPONSE if I2C driver error
     *              - ESP_ERR_TIMEOUT if sensor has not responded
     */
    esp_err_t zh_aht_read(float *humidity, float *temperature);

    /**
     * @brief Reset AHT sensor.
     *
     * @return
     *              - ESP_OK if reset was success
     *              - ESP_ERR_NOT_FOUND if sensor is not initialized
     *              - ESP_ERR_INVALID_RESPONSE if I2C driver error
     */
    esp_err_t zh_aht_reset(void);

#ifdef __cplusplus
}
#endif