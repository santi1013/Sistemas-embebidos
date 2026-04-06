#include "file_logger.h"
#include "sdcard_drv.h"
#include "config.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"

/**
 * @file file_logger.c
 * @brief Implementación del módulo encargado de registrar datos en archivo.
 *
 * Este módulo administra la apertura del archivo de log en la microSD y la
 * escritura de los registros generados por el sistema. Su objetivo es separar
 * la lógica de almacenamiento de la lógica de adquisición y procesamiento de
 * datos del MPU6050.
 *
 * La idea general es:
 * - verificar que la microSD esté montada,
 * - abrir el archivo de salida,
 * - escribir el encabezado solo si el archivo está vacío,
 * - y luego agregar cada nuevo registro que llegue desde las tareas del sistema.
 */

/** @brief Etiqueta usada para mensajes de depuración del módulo. */
static const char *TAG = "FILE_LOGGER";

/**
 * @brief Puntero global al archivo de log actualmente abierto.
 *
 * Esta variable mantiene abierto el archivo para evitar estar abriendo y
 * cerrando el recurso en cada escritura.
 */
static FILE *g_log_file = NULL;

/**
 * @brief Inicializa el archivo de log en la microSD.
 *
 * Esta función comprueba primero que la tarjeta SD esté montada. Después
 * verifica si el archivo ya existe y si contiene información, usando `stat()`.
 * Si el archivo está vacío, escribe el encabezado con los nombres de las
 * columnas; si ya contiene datos, simplemente lo abre para seguir agregando
 * registros al final.
 *
 * Su lógica permite que el archivo conserve información previa y evita
 * duplicar el encabezado en cada reinicio del sistema.
 *
 * @return
 * - ESP_OK si el archivo quedó listo para escritura.
 * - ESP_FAIL si la SD no está montada, si el archivo no pudo abrirse
 *   o si falló la escritura del encabezado.
 */
esp_err_t file_logger_init(void)
{
    struct stat st;
    bool file_is_empty = true;

    ESP_LOGI(TAG, "Inicializando file_logger...");

    if (!sdcard_is_mounted()) {
        ESP_LOGE(TAG, "La SD no esta montada");
        return ESP_FAIL;
    }

    if (stat(LOG_FILE_PATH, &st) == 0 && st.st_size > 0) {
        file_is_empty = false;
    }

    g_log_file = sdcard_open_log_file(LOG_FILE_PATH);
    if (g_log_file == NULL) {
        ESP_LOGE(TAG, "No se pudo abrir archivo de log");
        return ESP_FAIL;
    }

    if (file_is_empty) {
        const char *header = "timestamp_ms,ax,ay,az,gx,gy,gz,temp,pitch,roll\n";
        if (sdcard_append_line(g_log_file, header) != ESP_OK) {
            ESP_LOGE(TAG, "No se pudo escribir encabezado");
            sdcard_close_file(g_log_file);
            g_log_file = NULL;
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "Encabezado escrito correctamente");
    } else {
        ESP_LOGI(TAG, "El archivo ya existe, no se escribe encabezado");
    }

    ESP_LOGI(TAG, "Archivo de log listo: %s", LOG_FILE_PATH);
    return ESP_OK;
}

/**
 * @brief Escribe un registro formateado en el archivo de log.
 *
 * Esta función recibe un registro ya convertido a texto y lo agrega al archivo
 * abierto en la microSD. Antes de escribir, valida que el puntero al registro
 * sea válido y que el archivo haya sido inicializado correctamente.
 *
 * Su lógica es delegar la escritura real al driver de la SD, manteniendo este
 * módulo como intermediario entre la aplicación y el sistema de archivos.
 *
 * @param record Puntero al registro que contiene la línea de texto a guardar.
 *
 * @return
 * - ESP_OK si el registro fue escrito correctamente.
 * - ESP_ERR_INVALID_ARG si el puntero recibido es nulo.
 * - ESP_FAIL si el archivo no está abierto o si ocurre un error de escritura.
 */
esp_err_t file_logger_write_record(const log_record_t *record)
{
    if (record == NULL) {
        ESP_LOGE(TAG, "record == NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (g_log_file == NULL) {
        ESP_LOGE(TAG, "g_log_file == NULL");
        return ESP_FAIL;
    }

    return sdcard_append_line(g_log_file, record->line);
}

/**
 * @brief Cierra el archivo de log actualmente abierto.
 *
 * Esta función libera el recurso asociado al archivo si se encuentra abierto
 * y reinicia el puntero global a NULL para evitar accesos inválidos.
 *
 * Se utiliza cuando se quiere finalizar de forma segura la sesión de escritura
 * sobre la microSD.
 */
void file_logger_close(void)
{
    if (g_log_file != NULL) {
        sdcard_close_file(g_log_file);
        g_log_file = NULL;
    }
}