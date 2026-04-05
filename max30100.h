#ifndef MAX30100_H
#define MAX30100_H

#include <stdbool.h>
#include "esp_err.h"
#include "app_types.h"

esp_err_t max30100_inicializar_i2c(void);
esp_err_t max30100_inicializar_sensor(void);
esp_err_t max30100_leer_datos_crudos(MuestraMax30100_t *muestra);
void max30100_monitorear_signos_vitales(bool modo_alerta, ResultadoRitmo_t *resultado);

#endif