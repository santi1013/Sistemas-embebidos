#ifndef SDCARD_DRV_H
#define SDCARD_DRV_H

#include <stdio.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @file sdcard_drv.h
 * @brief Interfaz pública del driver de acceso a la microSD.
 */

esp_err_t sdcard_init(void);
bool sdcard_is_mounted(void);
FILE *sdcard_open_log_file(const char *path);
esp_err_t sdcard_append_line(FILE *file, const char *line);
void sdcard_close_file(FILE *file);

/**
 * @brief Realiza una prueba básica de escritura y lectura en la microSD.
 *
 * @param path Ruta del archivo de prueba.
 * @return ESP_OK si la prueba fue exitosa, ESP_FAIL en caso contrario.
 */
esp_err_t sdcard_self_test(const char *path);

#endif