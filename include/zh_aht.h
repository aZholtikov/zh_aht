/**
 * @file zh_aht.h
 */

#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

/**
 * @brief AHT sensor initial default values.
 */
#define ZH_AHT_INIT_CONFIG_DEFAULT() \
    {                                \
    }

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Structure for initial initialization of AHT sensor.
     */
    typedef struct
    {
        i2c_master_bus_handle_t i2c_handle; /*!< Unique I2C bus handle. */
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
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_aht_init(const zh_aht_init_config_t *config);

    /**
     * @brief Read AHT sensor.
     *
     * @param[out] humidity Pointer for AHT sensor reading data of humidity.
     * @param[out] temperature Pointer for AHT sensor reading data of temperature.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_aht_read(float *humidity, float *temperature);

    /**
     * @brief Reset AHT sensor.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_aht_reset(void);

#ifdef __cplusplus
}
#endif