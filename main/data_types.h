#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @file data_types.h
 * @brief Definición de estructuras de datos compartidas en el proyecto.
 *
 * Este archivo reúne los tipos de datos principales usados para transportar
 * información entre los módulos del sistema y entre las tareas de FreeRTOS.
 *
 * Aquí se definen las estructuras para almacenar:
 * - las lecturas crudas obtenidas del MPU6050,
 * - y los registros ya formateados que serán enviados al sistema de
 *   almacenamiento en microSD.
 */

/**
 * @brief Estructura que almacena una muestra cruda leída desde el MPU6050.
 *
 * Esta estructura contiene una lectura completa del sensor en un instante
 * determinado. Incluye la marca de tiempo en milisegundos y los valores de
 * aceleración, velocidad angular y temperatura ya convertidos a unidades de
 * ingeniería.
 *
 * Su función principal es servir como contenedor intermedio entre la tarea
 * de adquisición del sensor y las tareas de procesamiento o formateo.
 */
typedef struct {
    int64_t timestamp_ms;   /**< Marca de tiempo de la muestra en milisegundos. */
    float ax;               /**< Aceleración en el eje X, en g. */
    float ay;               /**< Aceleración en el eje Y, en g. */
    float az;               /**< Aceleración en el eje Z, en g. */
    float gx;               /**< Velocidad angular en el eje X, en °/s. */
    float gy;               /**< Velocidad angular en el eje Y, en °/s. */
    float gz;               /**< Velocidad angular en el eje Z, en °/s. */
    float temperature;      /**< Temperatura interna del MPU6050, en °C. */
} mpu_raw_data_t;

/**
 * @brief Estructura que representa un registro listo para guardar en archivo.
 *
 * Esta estructura se utiliza después del procesamiento de los datos del
 * sensor. Contiene la marca de tiempo asociada al registro y una línea de
 * texto ya formateada para ser escrita directamente en el archivo de log
 * dentro de la microSD.
 *
 * Su lógica es desacoplar el procesamiento numérico de la etapa de escritura,
 * permitiendo que una tarea genere el contenido y otra lo almacene.
 */
typedef struct {
    int64_t timestamp_ms;   /**< Marca de tiempo del registro en milisegundos. */
    char line[200];         /**< Línea de texto formateada para guardar en el archivo. */
} log_record_t;

#endif