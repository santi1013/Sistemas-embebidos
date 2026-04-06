#include "task_manager.h"
#include "config.h"
#include "data_types.h"
#include "mpu6050_drv.h"
#include "file_logger.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "freertos/task.h"
#include "esp_log.h"

/**
 * @file task_manager.c
 * @brief Implementación del gestor de colas y tareas de FreeRTOS.
 *
 * Este módulo coordina el flujo principal de procesamiento del nodo. Su función
 * es crear las colas de comunicación entre tareas y lanzar las tareas que
 * componen la lógica del sistema.
 *
 * La secuencia general de trabajo es:
 * - una tarea lee periódicamente el MPU6050,
 * - otra tarea toma esos datos y calcula pitch y roll,
 * - una tercera tarea recibe los registros ya formateados y los escribe en la SD.
 *
 * De esta manera se separa la adquisición, el procesamiento y el almacenamiento,
 * logrando un diseño más ordenado y fácil de depurar.
 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Factor de conversión de radianes a grados.
 *
 * Se utiliza para transformar el resultado de las funciones trigonométricas
 * al calcular pitch y roll.
 */
#define RAD_TO_DEG (180.0f / (float)M_PI)

/**
 * @brief Cola que transporta muestras crudas del MPU6050.
 *
 * En esta cola la tarea de lectura del sensor deposita estructuras del tipo
 * mpu_raw_data_t para que luego sean procesadas por la tarea de formateo.
 */
QueueHandle_t g_raw_queue = NULL;

/**
 * @brief Cola que transporta registros listos para guardar.
 *
 * En esta cola la tarea de formateo deposita estructuras del tipo log_record_t,
 * ya convertidas a texto, para que la tarea de escritura las almacene en la SD.
 */
QueueHandle_t g_log_queue = NULL;

/** @brief Etiqueta usada para mensajes de depuración del gestor de tareas. */
static const char *TAG = "TASK_MANAGER";

/**
 * @brief Tarea encargada de leer periódicamente el MPU6050.
 *
 * Esta tarea ejecuta la adquisición local del sensor. En cada iteración intenta
 * obtener una nueva muestra del MPU6050 y, si la lectura es correcta, envía esa
 * muestra a la cola `g_raw_queue`.
 *
 * Su lógica es actuar como productor de datos crudos dentro del sistema. Usa
 * `vTaskDelayUntil()` para mantener un periodo de muestreo constante definido
 * por `SENSOR_SAMPLE_PERIOD_MS`.
 *
 * @param pvParameters Parámetro genérico de FreeRTOS, no utilizado en esta tarea.
 */
static void sensor_task(void *pvParameters)
{
    mpu_raw_data_t sample;
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t sample_count = 0;

    ESP_LOGI("SENSOR_TASK", "Tarea MPU6050 iniciada");

    while (1) {
        if (mpu6050_read_raw(&sample) == ESP_OK) {
            if (xQueueSend(g_raw_queue, &sample, 0) != pdPASS) {
                ESP_LOGW("SENSOR_TASK", "g_raw_queue llena, muestra descartada");
            } else {
                sample_count++;
                if ((sample_count % 10) == 0) {
                    ESP_LOGI("SENSOR_TASK",
                             "Enviadas %lu muestras a g_raw_queue | ax=%.3f ay=%.3f az=%.3f",
                             (unsigned long)sample_count,
                             sample.ax, sample.ay, sample.az);
                }
            }
        } else {
            ESP_LOGE("SENSOR_TASK", "Error leyendo MPU6050");
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_SAMPLE_PERIOD_MS));
    }
}

/**
 * @brief Tarea encargada de procesar las muestras del MPU6050 y formatearlas.
 *
 * Esta tarea consume datos desde `g_raw_queue`. Cada vez que recibe una muestra,
 * calcula los ángulos de pitch y roll usando los valores de aceleración y luego
 * construye una línea de texto lista para ser almacenada en archivo.
 *
 * La lógica de esta tarea es convertir la información numérica cruda en un
 * registro estructurado de texto, desacoplando así el procesamiento matemático
 * de la etapa de escritura en la microSD.
 *
 * @param pvParameters Parámetro genérico de FreeRTOS, no utilizado en esta tarea.
 */
static void format_local_task(void *pvParameters)
{
    mpu_raw_data_t sample;
    log_record_t record;

    float pitch;
    float roll;
    uint32_t format_count = 0;

    ESP_LOGI("FORMAT_LOCAL", "Tarea de formateo local iniciada");

    while (1) {
        if (xQueueReceive(g_raw_queue, &sample, portMAX_DELAY) == pdPASS) {

            roll = atan2f(sample.ay, sample.az) * RAD_TO_DEG;
            pitch = atan2f(-sample.ax,
                           sqrtf((sample.ay * sample.ay) + (sample.az * sample.az))) * RAD_TO_DEG;

            record.timestamp_ms = sample.timestamp_ms;

            snprintf(record.line, sizeof(record.line),
                     "%lld,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.2f,%.2f,%.2f\n",
                     (long long)sample.timestamp_ms,
                     sample.ax, sample.ay, sample.az,
                     sample.gx, sample.gy, sample.gz,
                     sample.temperature,
                     pitch, roll);

            if (xQueueSend(g_log_queue, &record, portMAX_DELAY) != pdPASS) {
                ESP_LOGW("FORMAT_LOCAL", "g_log_queue llena");
            } else {
                format_count++;
                if ((format_count % 10) == 0) {
                    ESP_LOGI("FORMAT_LOCAL",
                             "Enviados %lu registros a g_log_queue | pitch=%.2f roll=%.2f",
                             (unsigned long)format_count,
                             pitch, roll);
                }
            }
        }
    }
}

/**
 * @brief Tarea encargada de escribir registros en la microSD.
 *
 * Esta tarea inicializa primero el módulo `file_logger` y luego queda bloqueada
 * esperando registros en la cola `g_log_queue`. Cada registro recibido se envía
 * al módulo de escritura para ser almacenado en el archivo de log.
 *
 * Su lógica es funcionar como consumidor final del pipeline del sistema,
 * recibiendo registros ya formateados y delegando la escritura física al
 * módulo de almacenamiento.
 *
 * @param pvParameters Parámetro genérico de FreeRTOS, no utilizado en esta tarea.
 */
static void sd_writer_task(void *pvParameters)
{
    log_record_t record;
    uint32_t write_count = 0;

    ESP_LOGI("SD_TASK", "Tarea de escritura SD iniciada");

    if (file_logger_init() != ESP_OK) {
        ESP_LOGE("SD_TASK", "No se pudo iniciar file_logger");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI("SD_TASK", "Esperando registros en g_log_queue...");

    while (1) {
        if (xQueueReceive(g_log_queue, &record, portMAX_DELAY) == pdPASS) {
            if (file_logger_write_record(&record) != ESP_OK) {
                ESP_LOGE("SD_TASK", "Error escribiendo registro");
            } else {
                write_count++;
                if ((write_count % 10) == 0) {
                    ESP_LOGI("SD_TASK",
                             "Escritos %lu registros en microSD. Ultimo timestamp=%lld",
                             (unsigned long)write_count,
                             (long long)record.timestamp_ms);
                }
            }
        }
    }
}

/**
 * @brief Inicializa las colas de comunicación del sistema.
 *
 * Esta función crea las dos colas principales usadas por el pipeline del nodo:
 * una para transportar muestras crudas del sensor y otra para transportar
 * registros listos para almacenamiento.
 *
 * La lógica de esta inicialización es dejar preparados los canales de
 * comunicación antes de lanzar cualquier tarea, evitando que una tarea empiece
 * a ejecutarse sin que sus recursos de intercambio existan.
 *
 * @return
 * - ESP_OK si ambas colas fueron creadas correctamente.
 * - ESP_FAIL si alguna de las colas no pudo crearse.
 */
esp_err_t task_manager_init(void)
{
    g_raw_queue = xQueueCreate(RAW_QUEUE_LENGTH, sizeof(mpu_raw_data_t));
    if (g_raw_queue == NULL) {
        ESP_LOGE(TAG, "No se pudo crear g_raw_queue");
        return ESP_FAIL;
    }

    g_log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(log_record_t));
    if (g_log_queue == NULL) {
        ESP_LOGE(TAG, "No se pudo crear g_log_queue");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Colas creadas correctamente");
    return ESP_OK;
}

/**
 * @brief Crea las tareas principales del sistema.
 *
 * Esta función lanza las tres tareas que componen la lógica operativa del nodo:
 * - `sensor_task` para adquisición del MPU6050,
 * - `format_local_task` para cálculo y formateo,
 * - `sd_writer_task` para escritura en microSD.
 *
 * La lógica de creación sigue el orden natural del flujo de datos, de manera
 * que el sistema quede listo para capturar, procesar y almacenar información
 * de forma continua.
 *
 * @return
 * - ESP_OK si todas las tareas fueron creadas correctamente.
 * - ESP_FAIL si falla la creación de alguna tarea.
 */
esp_err_t task_manager_create_tasks(void)
{
    BaseType_t ret;

    ret = xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK, NULL, SENSOR_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear sensor_task");
        return ESP_FAIL;
    }

    ret = xTaskCreate(format_local_task, "format_local_task", FORMAT_TASK_STACK, NULL, FORMAT_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear format_local_task");
        return ESP_FAIL;
    }

    ret = xTaskCreate(sd_writer_task, "sd_writer_task", SD_TASK_STACK, NULL, SD_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear sd_writer_task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Tareas creadas correctamente");
    return ESP_OK;
}