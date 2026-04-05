/**
 * @file zh_aht.h
 */

#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

/**
 * @brief AHT sensor initial default values.
 */
#define ZH_AHT_INIT_CONFIG_DEFAULT() \
    {                                \
        .i2c_frequency = 400000,     \
        .i2c_address = 0x38}

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
        uint8_t i2c_address;                /*!< Sensor I2C address. */
        uint32_t i2c_frequency;             /*!< Sensor I2C frequency. */
    } zh_aht_init_config_t;

    /**
     * @brief AHT sensor handle.
     */
    typedef struct
    {
        bool is_initialized;                /*!< Sensor initialization flag. */
        i2c_master_dev_handle_t dev_handle; /*!< Unique I2C device handle. */
    } zh_aht_handle_t;

    /**
     * @brief Structure for error statistics storage.
     */
    typedef struct
    {
        uint32_t i2c_driver_error; /*!< Number of i2c driver error. */
    } zh_aht_stats_t;

    /**
     * @brief Initialize AHT sensor.
     *
     * @param[in] config Pointer to AHT initialized configuration structure. Can point to a temporary variable.
     * @param[out] handle Pointer to unique AHT handle.
     *
     * @attention I2C driver must be initialized first.
     *
     * @note Before initialize the sensor recommend initialize zh_aht_init_config_t structure with default values.
     *
     * @code zh_aht_init_config_t config = ZH_AHT_INIT_CONFIG_DEFAULT() @endcode
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_aht_init(const zh_aht_init_config_t *config, zh_aht_handle_t *handle);

    /**
     * @brief Read AHT sensor.
     *
     * @param[in] handle Pointer to unique AHT handle.
     * @param[out] humidity Pointer for AHT sensor reading data of humidity.
     * @param[out] temperature Pointer for AHT sensor reading data of temperature.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_aht_read(zh_aht_handle_t *handle, float *humidity, float *temperature);

    /**
     * @brief Reset AHT sensor.
     *
     * @param[in] handle Pointer to unique AHT handle.
     *
     * @return ESP_OK if success or an error code otherwise.
     */
    esp_err_t zh_aht_reset(zh_aht_handle_t *handle);

    /**
     * @brief Get error statistics.
     *
     * @return Pointer to the statistics structure.
     */
    const zh_aht_stats_t *zh_aht_get_stats(void);

    /**
     * @brief Reset error statistics.
     */
    void zh_aht_reset_stats(void);

#ifdef __cplusplus
}
#endif