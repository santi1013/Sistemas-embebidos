#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tareas.h"

void app_main(void)
{
    tareas_inicializar();

    while (1) {
        tareas_ejecutar_maquina_estados();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}