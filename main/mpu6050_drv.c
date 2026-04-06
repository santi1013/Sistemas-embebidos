#include "mpu6050_drv.h"
#include "config.h"

#include <string.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_timer.h"

/**
 * @file mpu6050_drv.c
 * @brief Implementación del driver básico para el sensor MPU6050.
 *
 * Este módulo se encarga de inicializar la comunicación I2C con el MPU6050,
 * verificar la presencia del sensor, despertarlo desde su estado de reposo
 * y leer sus registros principales de aceleración, giroscopio y temperatura.
 *
 * La lógica general del driver es:
 * - configurar el bus I2C del ESP32,
 * - leer el registro WHO_AM_I para validar la respuesta del sensor,
 * - escribir en PWR_MGMT_1 para activar el MPU6050,
 * - leer los 14 bytes consecutivos de datos crudos,
 * - y convertir esos valores a unidades físicas utilizables por el resto del sistema.
 */

/** @brief Etiqueta usada para mensajes de depuración del módulo MPU6050. */
static const char *TAG = "MPU6050";

/* ============================================================================
 * REGISTROS INTERNOS DEL MPU6050
 * ============================================================================
 */

/**
 * @brief Dirección del registro WHO_AM_I del MPU6050.
 *
 * Este registro permite verificar la identidad del dispositivo conectado al
 * bus I2C.
 */
#define MPU6050_REG_WHO_AM_I       0x75

/**
 * @brief Dirección del registro de gestión de energía 1.
 *
 * Se utiliza para sacar al sensor del modo sleep y dejarlo operativo.
 */
#define MPU6050_REG_PWR_MGMT_1     0x6B

/**
 * @brief Dirección inicial del bloque de datos del acelerómetro.
 *
 * A partir de este registro se leen en secuencia los datos de aceleración,
 * temperatura y giroscopio.
 */
#define MPU6050_REG_ACCEL_XOUT_H   0x3B

/**
 * @brief Valor esperado en el registro WHO_AM_I para un MPU6050.
 */
#define MPU6050_WHO_AM_I_EXPECTED  0x68

/**
 * @brief Escribe un byte en un registro interno del MPU6050.
 *
 * Esta función auxiliar construye un pequeño buffer con la dirección del
 * registro y el dato a escribir, y luego lo transmite mediante I2C.
 *
 * Su lógica se usa principalmente durante la inicialización del sensor,
 * por ejemplo para modificar registros de control como PWR_MGMT_1.
 *
 * @param reg_addr Dirección del registro a modificar.
 * @param data Dato de 8 bits que se desea escribir.
 *
 * @return
 * - ESP_OK si la escritura fue exitosa.
 * - Código de error de ESP-IDF si la transacción I2C falla.
 */
static esp_err_t mpu6050_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = { reg_addr, data };

    return i2c_master_write_to_device(
        I2C_MASTER_NUM,
        MPU6050_I2C_ADDR,
        write_buf,
        sizeof(write_buf),
        pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)
    );
}

/**
 * @brief Lee uno o varios bytes desde un registro del MPU6050.
 *
 * Esta función auxiliar primero envía la dirección del registro deseado y
 * luego realiza una lectura consecutiva de la cantidad de bytes indicada.
 *
 * Se utiliza tanto para consultar registros de identificación como para leer
 * el bloque completo de datos del sensor.
 *
 * @param reg_addr Dirección del registro inicial de lectura.
 * @param data Puntero al buffer donde se almacenarán los datos leídos.
 * @param len Cantidad de bytes a leer.
 *
 * @return
 * - ESP_OK si la lectura fue exitosa.
 * - Código de error de ESP-IDF si la transacción I2C falla.
 */
static esp_err_t mpu6050_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(
        I2C_MASTER_NUM,
        MPU6050_I2C_ADDR,
        &reg_addr,
        1,
        data,
        len,
        pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)
    );
}

/**
 * @brief Inicializa el bus I2C maestro usado por el MPU6050.
 *
 * Esta función configura el periférico I2C del ESP32 en modo maestro usando
 * los pines y parámetros definidos en el archivo de configuración del proyecto.
 * Después aplica la configuración e instala el driver I2C.
 *
 * Su lógica deja lista la capa física de comunicación antes de interactuar
 * con cualquier registro del sensor.
 *
 * @return
 * - ESP_OK si la configuración e instalación del bus fueron correctas.
 * - Código de error si la configuración del puerto o la instalación del driver falla.
 */
static esp_err_t mpu6050_i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0
    };

    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        return ret;
    }

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

/**
 * @brief Inicializa el sensor MPU6050.
 *
 * Esta función prepara tanto la interfaz I2C como el propio sensor. Primero
 * inicializa el bus maestro, luego lee el registro WHO_AM_I para comprobar
 * la respuesta del dispositivo y finalmente escribe en PWR_MGMT_1 para sacar
 * al MPU6050 del modo sleep.
 *
 * La lógica de esta función garantiza que el sensor quede listo para comenzar
 * a entregar datos crudos de aceleración, giroscopio y temperatura.
 *
 * @return
 * - ESP_OK si el sensor fue inicializado correctamente.
 * - Código de error si falla la inicialización del bus, la lectura de identificación
 *   o la escritura del registro de activación.
 */
esp_err_t mpu6050_init(void)
{
    esp_err_t ret;
    uint8_t who_am_i = 0;

    ret = mpu6050_i2c_master_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Error inicializando I2C: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = mpu6050_read_bytes(MPU6050_REG_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo leer WHO_AM_I: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", who_am_i);

    if (who_am_i != MPU6050_WHO_AM_I_EXPECTED) {
        ESP_LOGW(TAG, "Valor WHO_AM_I inesperado");
    }

    /* Despertar el sensor: SLEEP = 0 en PWR_MGMT_1 */
    ret = mpu6050_write_byte(MPU6050_REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo escribir PWR_MGMT_1: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 inicializado correctamente");
    return ESP_OK;
}

/**
 * @brief Lee una muestra cruda del MPU6050 y la convierte a unidades físicas.
 *
 * Esta función realiza una lectura continua de 14 bytes desde el registro
 * ACCEL_XOUT_H. A partir de ese bloque obtiene:
 * - aceleración en los ejes X, Y y Z,
 * - temperatura interna,
 * - velocidad angular en los ejes X, Y y Z.
 *
 * Luego interpreta los bytes como enteros de 16 bits con signo y aplica los
 * factores de escala por defecto del sensor al arrancar:
 * - acelerómetro en rango ±2g,
 * - giroscopio en rango ±250 °/s.
 *
 * Finalmente también agrega una marca de tiempo en milisegundos usando el
 * temporizador del sistema.
 *
 * @param out_data Puntero a la estructura donde se guardará la muestra convertida.
 *
 * @return
 * - ESP_OK si la lectura y conversión fueron exitosas.
 * - ESP_ERR_INVALID_ARG si el puntero de salida es nulo.
 * - Código de error si la lectura I2C falla.
 */
esp_err_t mpu6050_read_raw(mpu_raw_data_t *out_data)
{
    uint8_t raw_buf[14];
    esp_err_t ret;

    if (out_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ret = mpu6050_read_bytes(MPU6050_REG_ACCEL_XOUT_H, raw_buf, sizeof(raw_buf));
    if (ret != ESP_OK) {
        return ret;
    }

    int16_t ax_raw = (int16_t)((raw_buf[0] << 8) | raw_buf[1]);
    int16_t ay_raw = (int16_t)((raw_buf[2] << 8) | raw_buf[3]);
    int16_t az_raw = (int16_t)((raw_buf[4] << 8) | raw_buf[5]);
    int16_t temp_raw = (int16_t)((raw_buf[6] << 8) | raw_buf[7]);
    int16_t gx_raw = (int16_t)((raw_buf[8] << 8) | raw_buf[9]);
    int16_t gy_raw = (int16_t)((raw_buf[10] << 8) | raw_buf[11]);
    int16_t gz_raw = (int16_t)((raw_buf[12] << 8) | raw_buf[13]);

    /* Escalas por defecto al arrancar:
       acelerómetro ±2g -> 16384 LSB/g
       giroscopio ±250 °/s -> 131 LSB/(°/s) */
    out_data->timestamp_ms = esp_timer_get_time() / 1000;
    out_data->ax = ax_raw / 16384.0f;
    out_data->ay = ay_raw / 16384.0f;
    out_data->az = az_raw / 16384.0f;
    out_data->gx = gx_raw / 131.0f;
    out_data->gy = gy_raw / 131.0f;
    out_data->gz = gz_raw / 131.0f;
    out_data->temperature = (temp_raw / 340.0f) + 36.53f;

    return ESP_OK;
}