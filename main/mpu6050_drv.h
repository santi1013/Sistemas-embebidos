#ifndef MPU6050_DRV_H
#define MPU6050_DRV_H

#include "esp_err.h"
#include "data_types.h"

/**
 * @file mpu6050_drv.h
 * @brief Interfaz pública del driver del sensor MPU6050.
 *
 * Este archivo declara las funciones necesarias para inicializar el sensor
 * MPU6050 y obtener muestras de aceleración, velocidad angular y temperatura.
 *
 * El módulo está diseñado para trabajar sobre I2C dentro del entorno ESP-IDF
 * y entregar los datos en una estructura común del proyecto, de modo que
 * puedan ser procesados por otras tareas del sistema.
 */

/**
 * @brief Inicializa el sensor MPU6050.
 *
 * Esta función configura la comunicación I2C necesaria para acceder al sensor,
 * verifica su identificación mediante el registro WHO_AM_I y lo saca del modo
 * de bajo consumo para dejarlo listo para operar.
 *
 * Debe ejecutarse una sola vez al iniciar el sistema, antes de intentar leer
 * cualquier dato del sensor.
 *
 * @return
 * - ESP_OK si el sensor fue inicializado correctamente.
 * - Código de error si falla la configuración del bus I2C, la verificación
 *   del sensor o su activación.
 */
esp_err_t mpu6050_init(void);

/**
 * @brief Lee una muestra del MPU6050 y la almacena en una estructura de salida.
 *
 * Esta función obtiene los datos crudos del acelerómetro, giroscopio y
 * temperatura, los convierte a unidades físicas y los guarda en una variable
 * de tipo mpu_raw_data_t junto con una marca de tiempo.
 *
 * Su propósito es ofrecer una lectura completa del estado instantáneo del
 * sensor en un formato listo para ser procesado por el resto del sistema.
 *
 * @param out_data Puntero a la estructura donde se almacenará la muestra leída.
 *
 * @return
 * - ESP_OK si la lectura fue exitosa.
 * - ESP_ERR_INVALID_ARG si el puntero de salida es nulo.
 * - Código de error si ocurre una falla durante la lectura por I2C.
 */
esp_err_t mpu6050_read_raw(mpu_raw_data_t *out_data);

#endif