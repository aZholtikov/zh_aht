/**
 * @file zh_aht.h
 *
 * @brief Driver for AHT series temperature and humidity sensors (AHT10/AHT15/AHT20/AHT21/AHT25/AHT30/AHT40)
 *
 * Provides I2C-based communication with AHT sensors including initialization,
 * data reading with CRC validation, reset functionality, and error statistics.
 *
 * Key features:
 * - I2C interface with configurable address and frequency
 * - Automatic sensor initialization and probe detection
 * - CRC8 data integrity verification
 * - Error statistics tracking
 *
 * @note Requires ESP-IDF v5.0+ with I2C master driver
 * @note Enable I2C_ISR_IRAM_SAFE and I2C_MASTER_ISR_HANDLER_IN_IRAM in menuconfig
 */

#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define ZH_AHT_INIT_CONFIG_DEFAULT() \
    {                                \
        .i2c_frequency = 400000,     \
        .i2c_address = 0x38}

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle for AHT sensor driver.
     *
     * Incomplete type that encapsulates internal sensor state.
     * Users interact with this handle exclusively through the public API functions.c
     */
    typedef struct _zh_aht_handle_t zh_aht_handle_t;

    /**
     * @brief Configuration structure for AHT sensor initialization
     */
    typedef struct
    {
        i2c_master_bus_handle_t i2c_handle; /*!< I2C bus handle for sensor connection */
        uint8_t i2c_address;                /*!< I2C device address (0x38, 0x39, or 0x44) */
        uint32_t i2c_frequency;             /*!< I2C clock frequency (up to 400000 Hz) */
    } zh_aht_init_config_t;

    /**
     * @brief Structure containing sensor error statistics
     */
    typedef struct
    {
        uint32_t i2c_driver_error; /*!< Counter of I2C driver errors */
    } zh_aht_stats_t;

    /**
     * @brief Initialize AHT sensor and establish I2C communication
     *
     * Allocates handle, configures I2C device, probes sensor presence,
     * and sends initialization command if required.
     *
     * @param[in] config Pointer to initialization configuration (must not be NULL)
     * @param[out] handle Pointer to receive the created sensor handle (must be NULL)
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if configuration parameters are invalid
     * @return ESP_ERR_INVALID_STATE if handle is already initialized
     * @return ESP_ERR_NO_MEM if memory allocation fails
     * @return ESP_FAIL if I2C initialization fails
     */
    esp_err_t zh_aht_init(const zh_aht_init_config_t *config, zh_aht_handle_t **handle);

    /**
     * @brief Deinitialize AHT sensor and release resources
     *
     * Removes I2C device and frees the sensor handle.
     *
     * @param[in,out] handle Pointer to sensor handle to deinitialize (must not be NULL)
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if handle is NULL
     */
    esp_err_t zh_aht_deinit(zh_aht_handle_t **handle);

    /**
     * @brief Read humidity and temperature from AHT sensor
     *
     * Sends read command, receives 7-byte response with CRC validation,
     * and converts raw data to engineering units.
     *
     * @param[in] handle Pointer to sensor handle (must not be NULL)
     * @param[out] humidity Pointer to store humidity value in percentage (0-100%) (must not be NULL)
     * @param[out] temperature Pointer to store temperature value in Celsius (-50 to +150°C) (must not be NULL)
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if parameters are NULL
     * @return ESP_ERR_TIMEOUT if sensor does not respond within timeout
     * @return ESP_ERR_INVALID_CRC if data integrity check fails
     * @return ESP_FAIL on I2C communication errors
     */
    esp_err_t zh_aht_read(zh_aht_handle_t **handle, float *humidity, float *temperature);

    /**
     * @brief Reset AHT sensor via I2C command
     *
     * Sends reset command and waits for sensor recovery.
     *
     * @param[in] handle Pointer to sensor handle (must not be NULL)
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if handle is NULL
     * @return ESP_FAIL on I2C communication errors
     */
    esp_err_t zh_aht_reset(zh_aht_handle_t **handle);

    /**
     * @brief Get pointer to sensor error statistics
     *
     * Returns read-only pointer to global statistics structure.
     *
     * @return Pointer to statistics structure (valid until reset)
     */
    const zh_aht_stats_t *zh_aht_get_stats(void);

    /**
     * @brief Reset all error statistics to zero
     *
     * Clears all error counters tracked in the global statistics structure.
     */
    void zh_aht_reset_stats(void);

#ifdef __cplusplus
}
#endif