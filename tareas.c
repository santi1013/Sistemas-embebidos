#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app_config.h"
#include "app_types.h"
#include "max30100.h"
#include "tareas.h"
#include "qmi8658.h"
#include "sd.h"

static const char *TAG_TAREAS = "TAREAS";

/* ============================================================
 * CONTEXTO PRIVADO DEL SISTEMA
 * ============================================================ */

static ContextoSistema_t contexto_sistema = {
    .estado_actual = ESTADO_RITMO,
    .contador_alertas_amarillas = 0,
    .contador_alertas_rojas = 0
};

/* ============================================================
 * FUNCIONES PRIVADAS
 * ============================================================ */

static void cambiar_al_siguiente_estado(void)
{
    switch (contexto_sistema.estado_actual) {
        case ESTADO_RITMO:
            contexto_sistema.estado_actual = ESTADO_SPO2;
            break;

        case ESTADO_SPO2:
            contexto_sistema.estado_actual = ESTADO_ACELEROMETRO;
            break;

        case ESTADO_ACELEROMETRO:
            contexto_sistema.estado_actual = ESTADO_GIROSCOPIO;
            break;

        case ESTADO_GIROSCOPIO:
            contexto_sistema.estado_actual = ESTADO_RITMO;
            break;

        default:
            contexto_sistema.estado_actual = ESTADO_RITMO;
            break;
    }
}

static void ejecutar_estado_ritmo(void)
{
    ResultadoRitmo_t resultado_ritmo;
    memset(&resultado_ritmo, 0, sizeof(ResultadoRitmo_t));

    ESP_LOGI(TAG_TAREAS, "==============================");
    ESP_LOGI(TAG_TAREAS, "ESTADO: RITMO CARDIACO");
    ESP_LOGI(TAG_TAREAS, "==============================");

    max30100_monitorear_signos_vitales(false, &resultado_ritmo);

    sd_logger_printf("\n----- RITMO CARDIACO -----\n");
    sd_logger_printf("Estado lectura   : %s\n", resultado_ritmo.estado_texto);
    sd_logger_printf("Contacto         : %s\n", resultado_ritmo.contacto_detectado ? "SI" : "NO");
    sd_logger_printf("Valido           : %s\n", resultado_ritmo.resultado_valido ? "SI" : "NO");
    sd_logger_printf("IR promedio      : %.2f\n", resultado_ritmo.ir_promedio);
    sd_logger_printf("Calidad senal    : %d\n", resultado_ritmo.calidad_senal);
    sd_logger_printf("BPM calculado    : %d\n", resultado_ritmo.bpm_calculado);
    sd_logger_printf("--------------------------\n\n");

    if (resultado_ritmo.resultado_valido) {
        if ((float)resultado_ritmo.bpm_calculado < RITMO_MIN_LPM ||
            (float)resultado_ritmo.bpm_calculado > RITMO_MAX_LPM) {
            contexto_sistema.contador_alertas_rojas++;
            sd_logger_printf("ALERTA ROJA: Ritmo cardiaco fuera de rango\n");
            sd_logger_printf("Total alertas rojas: %lu\n\n",
                             (unsigned long)contexto_sistema.contador_alertas_rojas);
        }
    }
}

static void ejecutar_estado_spo2(void)
{
    ResultadoRitmo_t resultado_spo2;
    memset(&resultado_spo2, 0, sizeof(ResultadoRitmo_t));

    ESP_LOGI(TAG_TAREAS, "==============================");
    ESP_LOGI(TAG_TAREAS, "ESTADO: SATURACION DE OXIGENO");
    ESP_LOGI(TAG_TAREAS, "==============================");

    max30100_monitorear_signos_vitales(false, &resultado_spo2);

    sd_logger_printf("\n----- SATURACION DE OXIGENO -----\n");
    sd_logger_printf("Estado lectura   : %s\n", resultado_spo2.estado_texto);
    sd_logger_printf("Contacto         : %s\n", resultado_spo2.contacto_detectado ? "SI" : "NO");
    sd_logger_printf("Valido           : %s\n", resultado_spo2.resultado_valido ? "SI" : "NO");
    sd_logger_printf("IR promedio      : %.2f\n", resultado_spo2.ir_promedio);
    sd_logger_printf("Calidad senal    : %d\n", resultado_spo2.calidad_senal);
    sd_logger_printf("SpO2 estimada    : %.2f %%\n", resultado_spo2.spo2_estimada);
    sd_logger_printf("---------------------------------\n\n");
}

static void ejecutar_estado_acelerometro(void)
{
    TickType_t tiempo_inicio = xTaskGetTickCount();
    TickType_t tiempo_estado = pdMS_TO_TICKS(TIEMPO_ESTADO_MS);

    Acelerometro_t acelerometro;

    ESP_LOGI(TAG_TAREAS, "==============================");
    ESP_LOGI(TAG_TAREAS, "ESTADO: ACELEROMETRO");
    ESP_LOGI(TAG_TAREAS, "==============================");

    sd_logger_printf("\n----- ACELEROMETRO -----\n");

    while ((xTaskGetTickCount() - tiempo_inicio) < tiempo_estado) {
        if (qmi8658_leer_acelerometro(&acelerometro) == ESP_OK) {
            sd_logger_printf("AX: %.3f g | AY: %.3f g | AZ: %.3f g\n",
                             acelerometro.ax,
                             acelerometro.ay,
                             acelerometro.az);
        } else {
            sd_logger_printf("Error leyendo acelerometro\n");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }

    sd_logger_printf("------------------------\n\n");
}

static void ejecutar_estado_giroscopio(void)
{
    TickType_t tiempo_inicio = xTaskGetTickCount();
    TickType_t tiempo_estado = pdMS_TO_TICKS(TIEMPO_ESTADO_MS);

    Giroscopio_t giroscopio;

    ESP_LOGI(TAG_TAREAS, "==============================");
    ESP_LOGI(TAG_TAREAS, "ESTADO: GIROSCOPIO");
    ESP_LOGI(TAG_TAREAS, "==============================");

    sd_logger_printf("\n----- GIROSCOPIO -----\n");

    while ((xTaskGetTickCount() - tiempo_inicio) < tiempo_estado) {
        if (qmi8658_leer_giroscopio(&giroscopio) == ESP_OK) {
            sd_logger_printf("GX: %.3f dps | GY: %.3f dps | GZ: %.3f dps\n",
                             giroscopio.gx,
                             giroscopio.gy,
                             giroscopio.gz);
        } else {
            sd_logger_printf("Error leyendo giroscopio\n");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }

    sd_logger_printf("----------------------\n\n");
}

/* ============================================================
 * API PUBLICA
 * ============================================================ */

void tareas_inicializar(void)
{
    ESP_LOGI(TAG_TAREAS, "Inicializando tareas del sistema...");

    ESP_ERROR_CHECK(max30100_inicializar_i2c());
    ESP_ERROR_CHECK(max30100_inicializar_sensor());

    ESP_ERROR_CHECK(qmi8658_inicializar_i2c());
    ESP_ERROR_CHECK(qmi8658_inicializar_sensor());

    esp_err_t ret_sd = sd_logger_inicializar();
    if (ret_sd != ESP_OK) {
        ESP_LOGW(TAG_TAREAS, "La SD no se pudo inicializar. Se continuara solo por serial.");
    } else {
        sd_logger_printf("SD inicializada correctamente\n");
    }

    contexto_sistema.estado_actual = ESTADO_RITMO;
    contexto_sistema.contador_alertas_amarillas = 0;
    contexto_sistema.contador_alertas_rojas = 0;

    ESP_LOGI(TAG_TAREAS, "Sistema inicializado correctamente");
}

void tareas_ejecutar_maquina_estados(void)
{
    switch (contexto_sistema.estado_actual) {
        case ESTADO_RITMO:
            ejecutar_estado_ritmo();
            break;

        case ESTADO_SPO2:
            ejecutar_estado_spo2();
            break;

        case ESTADO_ACELEROMETRO:
            ejecutar_estado_acelerometro();
            break;

        case ESTADO_GIROSCOPIO:
            ejecutar_estado_giroscopio();
            break;

        default:
            contexto_sistema.estado_actual = ESTADO_RITMO;
            break;
    }

    cambiar_al_siguiente_estado();
}