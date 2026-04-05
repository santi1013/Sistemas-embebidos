#ifndef QMI8658_H
#define QMI8658_H

#include "esp_err.h"
#include "app_types.h"

esp_err_t qmi8658_inicializar_i2c(void);
esp_err_t qmi8658_inicializar_sensor(void);

esp_err_t qmi8658_leer_acelerometro(Acelerometro_t *acelerometro);
esp_err_t qmi8658_leer_giroscopio(Giroscopio_t *giroscopio);

#endif