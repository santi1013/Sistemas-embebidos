#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"

/**
 * @file task_manager.h
 * @brief Interfaz pública del gestor de tareas y colas del sistema.
 *
 * Este archivo declara los recursos globales y las funciones encargadas de
 * preparar la arquitectura multitarea del proyecto basada en FreeRTOS.
 *
 * El módulo task_manager organiza el flujo principal del nodo mediante colas
 * y tareas independientes, permitiendo separar la lectura del MPU6050, el
 * procesamiento de datos y la escritura en la microSD.
 */

/**
 * @brief Cola global para transportar muestras crudas del MPU6050.
 *
 * Esta cola comunica la tarea de adquisición del sensor con la tarea de
 * procesamiento o formateo. En ella se almacenan estructuras con lecturas
 * completas del MPU6050.
 */
extern QueueHandle_t g_raw_queue;

/**
 * @brief Cola global para transportar registros listos para almacenamiento.
 *
 * Esta cola comunica la tarea de procesamiento con la tarea encargada de
 * escribir en la microSD. En ella se almacenan registros ya convertidos
 * a formato de texto.
 */
extern QueueHandle_t g_log_queue;

/**
 * @brief Inicializa las colas principales del sistema.
 *
 * Esta función crea las colas que serán usadas para intercambiar datos entre
 * las tareas de FreeRTOS. Debe ejecutarse antes de crear las tareas del nodo.
 *
 * @return
 * - ESP_OK si las colas fueron creadas correctamente.
 * - ESP_FAIL si alguna cola no pudo inicializarse.
 */
esp_err_t task_manager_init(void);

/**
 * @brief Crea las tareas principales del nodo.
 *
 * Esta función lanza las tareas encargadas de la lectura del sensor, el
 * procesamiento de los datos y la escritura en la microSD.
 *
 * Debe llamarse después de que las colas y los módulos base del sistema
 * hayan sido inicializados correctamente.
 *
 * @return
 * - ESP_OK si todas las tareas fueron creadas con éxito.
 * - ESP_FAIL si alguna tarea no pudo crearse.
 */
esp_err_t task_manager_create_tasks(void);

#endif