#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app_config.h"
#include "app_types.h"
#include "max30100.h"
#include "tareas.h"
#include "qmi8658.h"
#include "sd.h"
#include "pantalla.h"

static const char *TAG_TAREAS = "TAREAS";

/* ============================================================
 * CONTEXTO DEL SISTEMA
 * ============================================================ */

static ContextoSistema_t contexto_sistema = {
    .estado_actual = ESTADO_RITMO,
    .contador_alertas_amarillas = 0,
    .contador_alertas_rojas = 0
};

/* ============================================================
 * CLASIFICACION DE MOVIMIENTO
 * ============================================================ */

static estado_movimiento_t clasificar_movimiento(float a_prom,
                                                 float a_max,
                                                 float g_prom,
                                                 float g_max)
{
    // CAIDA: aceleracion y giro muy altos
    if (a_max > 2.2f && g_max > 200.0f) {
        return MOVIMIENTO_CAIDA;
    }

    // QUIETO: casi sin movimiento
    if (fabs(a_prom - 1.0f) < 0.1f && g_prom < 20.0f) {
        return MOVIMIENTO_QUIETO;
    }

    // CAMINANDO: movimiento moderado
    if (a_prom > 1.05f && a_prom < 1.8f && g_prom > 20.0f) {
        return MOVIMIENTO_CAMINANDO;
    }

    return MOVIMIENTO_INDETERMINADO;
}

/* ============================================================
 * CAMBIO DE ESTADO
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

/* ============================================================
 * ESTADO RITMO
 * ============================================================ */

static void ejecutar_estado_ritmo(void)
{
    ResultadoRitmo_t resultado;
    memset(&resultado, 0, sizeof(resultado));

    max30100_monitorear_signos_vitales(false, &resultado);

    sd_logger_printf("\n--- RITMO ---\n");
    sd_logger_printf("BPM: %d | Calidad: %d\n",
                     resultado.bpm_calculado,
                     resultado.calidad_senal);

    // Mostrar en pantalla
    pantalla_mostrar_ritmo(resultado.bpm_calculado,
                           resultado.calidad_senal,
                           resultado.estado_texto);

    // ALERTA ROJA
    if (resultado.resultado_valido) {
        if (resultado.bpm_calculado < RITMO_MIN_LPM ||
            resultado.bpm_calculado > RITMO_MAX_LPM) {

            contexto_sistema.contador_alertas_rojas++;

            pantalla_mostrar_alerta("RITMO FUERA DE RANGO", COLOR_ROJO);

            sd_logger_printf("ALERTA ROJA\n");
        }
    }
}

/* ============================================================
 * ESTADO SPO2
 * ============================================================ */

static void ejecutar_estado_spo2(void)
{
    ResultadoRitmo_t resultado;
    memset(&resultado, 0, sizeof(resultado));

    max30100_monitorear_signos_vitales(false, &resultado);

    sd_logger_printf("\n--- SPO2 ---\n");
    sd_logger_printf("SpO2: %.2f\n", resultado.spo2_estimada);

    pantalla_mostrar_spo2(resultado.spo2_estimada,
                          resultado.estado_texto);
}

/* ============================================================
 * ESTADO MOVIMIENTO (ACELEROMETRO + GIROSCOPIO)
 * ============================================================ */

static void ejecutar_estado_movimiento(void)
{
    TickType_t inicio = xTaskGetTickCount();
    TickType_t duracion = pdMS_TO_TICKS(TIEMPO_ESTADO_MS);

    Acelerometro_t acc;
    Giroscopio_t gyro;

    float suma_a = 0;
    float suma_g = 0;

    float max_a = 0;
    float max_g = 0;

    int muestras = 0;

    while ((xTaskGetTickCount() - inicio) < duracion) {

        if (qmi8658_leer_acelerometro(&acc) == ESP_OK &&
            qmi8658_leer_giroscopio(&gyro) == ESP_OK) {

            float a_mag = sqrtf(acc.ax * acc.ax +
                                acc.ay * acc.ay +
                                acc.az * acc.az);

            float g_mag = sqrtf(gyro.gx * gyro.gx +
                                gyro.gy * gyro.gy +
                                gyro.gz * gyro.gz);

            suma_a += a_mag;
            suma_g += g_mag;

            if (a_mag > max_a) max_a = a_mag;
            if (g_mag > max_g) max_g = g_mag;

            muestras++;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (muestras == 0) return;

    float a_prom = suma_a / muestras;
    float g_prom = suma_g / muestras;

    estado_movimiento_t estado = clasificar_movimiento(a_prom, max_a, g_prom, max_g);

    sd_logger_printf("\n--- MOVIMIENTO ---\n");
    sd_logger_printf("A_prom: %.2f | G_prom: %.2f\n", a_prom, g_prom);

    pantalla_mostrar_movimiento(a_prom, g_prom, estado);

    // ALERTA CAIDA
    if (estado == MOVIMIENTO_CAIDA) {
        pantalla_mostrar_alerta("CAIDA DETECTADA", COLOR_ROJO);
    }
}

/* ============================================================
 * INICIALIZACION
 * ============================================================ */

void tareas_inicializar(void)
{
    ESP_LOGI(TAG_TAREAS, "Inicializando sistema...");

    ESP_ERROR_CHECK(max30100_inicializar_i2c());
    ESP_ERROR_CHECK(max30100_inicializar_sensor());

    ESP_ERROR_CHECK(qmi8658_inicializar_i2c());
    ESP_ERROR_CHECK(qmi8658_inicializar_sensor());

    pantalla_inicializar();
    pantalla_mostrar_inicio();

    esp_err_t ret = sd_logger_inicializar();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_TAREAS, "SD no disponible");
    }

    contexto_sistema.estado_actual = ESTADO_RITMO;
}

/* ============================================================
 * MAQUINA DE ESTADOS
 * ============================================================ */

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
        case ESTADO_GIROSCOPIO:
            ejecutar_estado_movimiento();
            break;

        default:
            contexto_sistema.estado_actual = ESTADO_RITMO;
            break;
    }

    cambiar_al_siguiente_estado();
}