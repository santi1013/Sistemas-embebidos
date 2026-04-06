#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include "esp_err.h"
#include "data_types.h"

/**
 * @file file_logger.h
 * @brief Interfaz pública del módulo de registro en archivo.
 *
 * Este archivo declara las funciones encargadas de inicializar el sistema de
 * almacenamiento de registros en la microSD y de escribir datos formateados
 * en el archivo de log del proyecto.
 *
 * El módulo file_logger actúa como una capa intermedia entre las tareas que
 * generan información y el driver de acceso a la tarjeta SD. Su objetivo es
 * simplificar la escritura de registros y mantener separada la lógica de
 * adquisición/procesamiento de la lógica de almacenamiento.
 */

/**
 * @brief Inicializa el módulo de escritura en archivo.
 *
 * Esta función prepara el archivo de log para comenzar a almacenar registros.
 * Internamente verifica que la microSD esté montada, abre el archivo definido
 * en la configuración del proyecto y, si corresponde, escribe el encabezado
 * del archivo.
 *
 * Debe ejecutarse antes de llamar a la función de escritura de registros.
 *
 * @return
 * - ESP_OK si la inicialización fue exitosa.
 * - ESP_FAIL si la SD no está montada, si el archivo no pudo abrirse
 *   o si falló la escritura del encabezado.
 */
esp_err_t file_logger_init(void);

/**
 * @brief Escribe un registro formateado en el archivo de log.
 *
 * Esta función recibe una estructura de tipo log_record_t que contiene una
 * línea de texto ya construida y lista para ser almacenada. El contenido es
 * enviado al driver de la microSD para su escritura en el archivo.
 *
 * @param record Puntero al registro que se desea guardar.
 *
 * @return
 * - ESP_OK si el registro fue escrito correctamente.
 * - ESP_ERR_INVALID_ARG si el puntero recibido es nulo.
 * - ESP_FAIL si el archivo no está disponible o si ocurre un error de escritura.
 */
esp_err_t file_logger_write_record(const log_record_t *record);

/**
 * @brief Cierra el archivo de log actualmente abierto.
 *
 * Esta función libera el recurso asociado al archivo de salida y deja el
 * módulo en un estado seguro. Se utiliza al finalizar el proceso de registro
 * o cuando se desea cerrar ordenadamente el acceso al archivo.
 */
void file_logger_close(void);

#endif