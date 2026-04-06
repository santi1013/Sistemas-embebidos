#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"

#include "task_manager.h"
#include "mpu6050_drv.h"
#include "sdcard_drv.h"
#include "config.h"

/**
 * @file main.c
 * @brief Punto de entrada principal del nodo local con MPU6050 y microSD.
 *
 * Este archivo contiene la función principal de arranque de la aplicación en
 * ESP-IDF. Su responsabilidad es inicializar los módulos base del sistema y
 * poner en marcha las tareas de FreeRTOS necesarias para la operación del nodo.
 *
 * La lógica general de inicio es:
 * - crear las colas de comunicación entre tareas,
 * - inicializar el sensor MPU6050,
 * - inicializar y montar la microSD,
 * - ejecutar una prueba directa de escritura en la tarjeta,
 * - crear las tareas encargadas de lectura, formateo y escritura,
 * - y dejar el sistema ejecutándose de forma autónoma.
 *
 * Este nodo trabaja de forma local, sin comunicación con otros ESP, y está
 * orientado a adquirir datos del MPU6050 y almacenarlos en archivo.
 */

/** @brief Etiqueta usada para mensajes de depuración del módulo principal. */
static const char *TAG = "APP";

/**
 * @brief Función principal de la aplicación en ESP-IDF.
 *
 * Esta función es el punto de entrada del programa una vez el ESP32 ha
 * arrancado. Se encarga de ejecutar en orden la secuencia de inicialización
 * del sistema.
 *
 * Primero crea las colas que permitirán intercambiar datos entre las tareas.
 * Luego inicializa el MPU6050 para habilitar la lectura del sensor, monta la
 * microSD para preparar el almacenamiento y ejecuta una prueba básica de
 * escritura sobre la tarjeta usando la ruta definida en `SD_TEST_FILE_PATH`.
 *
 * Si la prueba de la microSD es exitosa, crea las tareas del sistema mediante
 * el gestor de tareas. Si alguna etapa falla, la función registra el error y
 * detiene la secuencia de arranque para evitar que el sistema continúe en un
 * estado inconsistente.
 *
 * @note
 * Esta función no entra en un bucle infinito explícito porque, una vez creadas
 * las tareas de FreeRTOS, el control principal queda delegado al planificador
 * del sistema operativo.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando nodo MPU6050 + microSD...");

    if (task_manager_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error creando colas del sistema");
        return;
    }

    if (mpu6050_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando MPU6050");
        return;
    }

    if (sdcard_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando microSD");
        return;
    }

    if (sdcard_self_test(SD_TEST_FILE_PATH) != ESP_OK) {
        ESP_LOGE(TAG, "La prueba directa de microSD fallo");
        return;
    }

    if (task_manager_create_tasks() != ESP_OK) {
        ESP_LOGE(TAG, "Error creando tareas");
        return;
    }

    ESP_LOGI(TAG, "Sistema local iniciado correctamente");
}