#include "qmi8658.h"
#include "app_config.h"

#include <stdio.h>
#include <stdint.h>

#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG_QMI8658 = "QMI8658";

/* ============================================================
 * REGISTROS QMI8658
 * ============================================================ */
#define QMI8658_REG_WHO_AM_I            0x00
#define QMI8658_REG_CTRL1               0x02
#define QMI8658_REG_CTRL2               0x03
#define QMI8658_REG_CTRL3               0x04
#define QMI8658_REG_CTRL7               0x08

#define QMI8658_REG_AX_L                0x35
#define QMI8658_REG_GX_L                0x3B

/* ============================================================
 * CONFIGURACION INTERNA
 * ============================================================ */
/*
 * Configuracion usada:
 * - Acelerometro: ±4g
 * - Giroscopio : ±512 dps
 *
 * Factores de conversion aproximados:
 * - ±4g   -> 8192 LSB/g
 * - ±512  -> 64 LSB/dps
 */
#define QMI8658_ACCEL_LSB_4G            8192.0f
#define QMI8658_GYRO_LSB_512DPS         64.0f

/* ============================================================
 * FUNCIONES PRIVADAS I2C
 * ============================================================ */
static esp_err_t qmi8658_escribir_registro(uint8_t registro, uint8_t valor)
{
    uint8_t datos_tx[2] = {registro, valor};

    return i2c_master_write_to_device(
        I2C_PUERTO_QMI8658,
        QMI8658_DIRECCION_I2C,
        datos_tx,
        sizeof(datos_tx),
        pdMS_TO_TICKS(I2C_TIMEOUT_MS)
    );
}

static esp_err_t qmi8658_leer_registros(uint8_t registro_inicial, uint8_t *buffer, size_t longitud)
{
    return i2c_master_write_read_device(
        I2C_PUERTO_QMI8658,
        QMI8658_DIRECCION_I2C,
        &registro_inicial,
        1,
        buffer,
        longitud,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS)
    );
}

static int16_t combinar_bytes(uint8_t lsb, uint8_t msb)
{
    return (int16_t)(((uint16_t)msb << 8) | lsb);
}

/* ============================================================
 * API PUBLICA
 * ============================================================ */
esp_err_t qmi8658_inicializar_i2c(void)
{
    i2c_config_t configuracion_i2c_qmi8658 = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_GPIO_SDA_QMI8658,
        .scl_io_num = I2C_GPIO_SCL_QMI8658,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FRECUENCIA_QMI8658_HZ,
        .clk_flags = 0
    };

    esp_err_t estado = i2c_param_config(I2C_PUERTO_QMI8658, &configuracion_i2c_qmi8658);
    if (estado != ESP_OK) {
        ESP_LOGE(TAG_QMI8658, "Error configurando I2C: %s", esp_err_to_name(estado));
        return estado;
    }

    estado = i2c_driver_install(I2C_PUERTO_QMI8658, I2C_MODE_MASTER, 0, 0, 0);
    if (estado != ESP_OK && estado != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG_QMI8658, "Error instalando driver I2C: %s", esp_err_to_name(estado));
        return estado;
    }

    ESP_LOGI(TAG_QMI8658, "I2C QMI8658 inicializado");
    return ESP_OK;
}

esp_err_t qmi8658_inicializar_sensor(void)
{
    uint8_t who_am_i = 0;
    esp_err_t estado = qmi8658_leer_registros(QMI8658_REG_WHO_AM_I, &who_am_i, 1);
    if (estado != ESP_OK) {
        ESP_LOGE(TAG_QMI8658, "QMI8658 no responde por I2C");
        return estado;
    }

    ESP_LOGI(TAG_QMI8658, "WHO_AM_I = 0x%02X", who_am_i);

    /*
     * CTRL1: configuracion basica de interfaz
     * Este valor suele funcionar en configuraciones simples.
     */
    estado = qmi8658_escribir_registro(QMI8658_REG_CTRL1, 0x40);
    if (estado != ESP_OK) {
        return estado;
    }

    /*
     * CTRL2: acelerometro
     * 0x23 -> ejemplo comun para activar accelerometro con ±4g y ODR media
     */
    estado = qmi8658_escribir_registro(QMI8658_REG_CTRL2, 0x23);
    if (estado != ESP_OK) {
        return estado;
    }

    /*
     * CTRL3: giroscopio
     * 0x53 -> ejemplo comun para activar gyro con ±512 dps y ODR media
     */
    estado = qmi8658_escribir_registro(QMI8658_REG_CTRL3, 0x53);
    if (estado != ESP_OK) {
        return estado;
    }

    /*
     * CTRL7: habilitar acelerometro y giroscopio
     * bit1 accel, bit2 gyro en configuraciones tipicas
     */
    estado = qmi8658_escribir_registro(QMI8658_REG_CTRL7, 0x03);
    if (estado != ESP_OK) {
        return estado;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG_QMI8658, "QMI8658 configurado");
    return ESP_OK;
}

esp_err_t qmi8658_leer_acelerometro(Acelerometro_t *acelerometro)
{
    if (acelerometro == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t datos_crudos[6];
    esp_err_t estado = qmi8658_leer_registros(QMI8658_REG_AX_L, datos_crudos, sizeof(datos_crudos));
    if (estado != ESP_OK) {
        return estado;
    }

    int16_t ax_raw = combinar_bytes(datos_crudos[0], datos_crudos[1]);
    int16_t ay_raw = combinar_bytes(datos_crudos[2], datos_crudos[3]);
    int16_t az_raw = combinar_bytes(datos_crudos[4], datos_crudos[5]);

    acelerometro->ax = (float)ax_raw / QMI8658_ACCEL_LSB_4G;
    acelerometro->ay = (float)ay_raw / QMI8658_ACCEL_LSB_4G;
    acelerometro->az = (float)az_raw / QMI8658_ACCEL_LSB_4G;

    return ESP_OK;
}

esp_err_t qmi8658_leer_giroscopio(Giroscopio_t *giroscopio)
{
    if (giroscopio == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t datos_crudos[6];
    esp_err_t estado = qmi8658_leer_registros(QMI8658_REG_GX_L, datos_crudos, sizeof(datos_crudos));
    if (estado != ESP_OK) {
        return estado;
    }

    int16_t gx_raw = combinar_bytes(datos_crudos[0], datos_crudos[1]);
    int16_t gy_raw = combinar_bytes(datos_crudos[2], datos_crudos[3]);
    int16_t gz_raw = combinar_bytes(datos_crudos[4], datos_crudos[5]);

    giroscopio->gx = (float)gx_raw / QMI8658_GYRO_LSB_512DPS;
    giroscopio->gy = (float)gy_raw / QMI8658_GYRO_LSB_512DPS;
    giroscopio->gz = (float)gz_raw / QMI8658_GYRO_LSB_512DPS;

    return ESP_OK;
}