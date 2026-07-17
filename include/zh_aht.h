/**
 * @file zh_aht.h
 *
 * @brief ESP-IDF driver for AHT family sensors (AHT10, AHT15, AHT20, AHT21, AHT25, AHT30, AHT40).
 *
 * This module provides a high-level interface to control AHT family I2C temperature & humidity sensors.
 * All devices (AHT10, AHT15, AHT20, AHT21, AHT25, AHT30, AHT40) share the same command set, data format, and CRC algorithm.
 *
 * @note Supported devices: AHT10, AHT15, AHT20, AHT21, AHT25, AHT30, AHT40, and compatible clones.
 * @note Default I2C address: 0x38 (ADR tied to GND). Some variants (e.g., AHT20/30 on certain boards) may use 0x39 (ADR tied to VDD). For AHT40 use 0x44.
 * @note Measurement range and accuracy differ between variants, but communication protocol is identical.
 */

#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

/**
 * @brief Default AHT initialization configuration.
 *
 * @note I2C frequency: 400 kHz (fast mode — supported by all AHT variants).
 * @note I2C address: 0x38 (ADR = GND). If ADR is pulled high (VDD), use 0x39. For AHT40 use 0x44.
 *
 * @warning AHT20/30 may be configured to 0x39 by hardware — check your board!
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
     * @brief Opaque handle type for AHT family sensor instance.
     */
    typedef struct _zh_aht_handle_t zh_aht_handle_t;

    /**
     * @brief AHT sensor initialization configuration structure.
     */
    typedef struct
    {
        i2c_master_bus_handle_t i2c_handle; /*!< I2C bus handle (created via i2c_master_bus_create() or i2c_master_bus_alloc_handle()). */
        uint8_t i2c_address;                /*!< I2C address: 0x38 (ADR=GND) or 0x39 (ADR=VDD) or 0x44 (AHT40). */
        uint32_t i2c_frequency;             /*!< I2C clock frequency in Hz (≤ 400000). */
    } zh_aht_init_config_t;

    /**
     * @brief Error statistics structure.
     */
    typedef struct
    {
        uint32_t i2c_driver_error; /*!< Number of I2C errors in zh_aht_read()/zh_aht_reset(). */
    } zh_aht_stats_t;

    /**
     * @brief Initialize an AHT family sensor and allocate handle.
     *
     * Validates configuration, allocates handle, registers device, probes sensor, and triggers calibration if needed.
     *
     * @param[in] config Pointer to initialized configuration (must be non `NULL`, valid).
     * @param[out] handle Double pointer to handle structure (`zh_aht_handle_t **`). On success, `*handle` points to newly allocated structure.
     *
     * @return ESP_OK on success.
     * @return ESP_ERR_INVALID_ARG if `config == NULL` or `handle == NULL`.
     * @return ESP_ERR_INVALID_STATE if `*handle != NULL` (already initialized).
     * @return ESP_ERR_NO_MEM if handle allocation failed.
     * @return ESP_FAIL if I2C registration failed.
     * @return ESP_ERR_NOT_FOUND if sensor did not respond.
     */
    esp_err_t zh_aht_init(const zh_aht_init_config_t *config, zh_aht_handle_t **handle);

    /**
     * @brief Deinitialize the AHT sensor and release resources.
     *
     * Removes I2C device, frees handle, sets `*handle = NULL`.
     *
     * @param[in,out] handle Double pointer to handle structure (`zh_aht_handle_t **`). Must not be `NULL`. Set to `NULL` on exit.
     *
     * @return ESP_OK on success.
     * @return ESP_ERR_INVALID_ARG if `handle == NULL` or `*handle == NULL` (not initialized).
     * @return ESP_FAIL if failed to remove I2C device.
     */
    esp_err_t zh_aht_deinit(zh_aht_handle_t **handle);

    /**
     * @brief Trigger measurement and read temperature & humidity.
     *
     * Sends read command, waits 80 ms, validates CRC/status, and converts to physical units.
     *
     * @param[in] handle Double pointer to handle structure (`zh_aht_handle_t **`). Must not be `NULL`.
     * @param[out] humidity Pointer to store humidity (%RH).
     * @param[out] temperature Pointer to store temperature (°C).
     *
     * @return ESP_OK on success.
     * @return ESP_ERR_INVALID_ARG if `handle == NULL` or `*handle == NULL` (not initialized) or `humidity == NULL` or `temperature == NULL`.
     * @return ESP_FAIL if I2C error.
     * @return ESP_ERR_TIMEOUT if sensor still busy.
     * @return ESP_ERR_INVALID_CRC if CRC mismatch.
     */
    esp_err_t zh_aht_read(zh_aht_handle_t **handle, float *humidity, float *temperature);

    /**
     * @brief Soft reset the AHT sensor.
     *
     * Sends reset command and waits 20 ms for recovery.
     *
     * @param[in] handle Double pointer to handle structure (`zh_aht_handle_t **`). Must not be `NULL`.
     *
     * @return ESP_OK on success.
     * @return ESP_ERR_INVALID_ARG if `handle == NULL` or `*handle == NULL` (not initialized).
     * @return ESP_FAIL if I2C error.
     */
    esp_err_t zh_aht_reset(zh_aht_handle_t **handle);

    /**
     * @brief Get pointer to internal error statistics.
     *
     * @return Pointer to static statistics structure.
     */
    const zh_aht_stats_t *zh_aht_get_stats(void);

    /**
     * @brief Reset error statistics counter.
     *
     * Sets `i2c_driver_error = 0`.
     */
    void zh_aht_reset_stats(void);

#ifdef __cplusplus
}
#endif