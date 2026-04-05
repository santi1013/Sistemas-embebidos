#include "max30100.h"
#include "app_config.h"
#include "app_types.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG_MAX30100 = "MAX30100";

/* =========================
 * REGISTROS DEL MAX30100
 * ========================= */
#define MAX30100_REG_INT_STATUS         0x00
#define MAX30100_REG_INT_ENABLE         0x01
#define MAX30100_REG_FIFO_WR_PTR        0x02
#define MAX30100_REG_OVF_COUNTER        0x03
#define MAX30100_REG_FIFO_RD_PTR        0x04
#define MAX30100_REG_FIFO_DATA          0x05
#define MAX30100_REG_MODE_CONFIG        0x06
#define MAX30100_REG_SPO2_CONFIG        0x07
#define MAX30100_REG_LED_CONFIG         0x09
#define MAX30100_REG_PART_ID            0xFF

#define MAX30100_MODE_SPO2              0x03
#define MAX30100_RESET_BIT              0x40
#define MAX30100_SPO2_HI_RES_EN         0x40
#define MAX30100_SR_100HZ               0x04
#define MAX30100_LED_PW_1600US          0x03
#define MAX30100_LED_CURR_27MA          0x0B

#define MAX30100_MAX_PICOS              20
#define MAX30100_MIN_MUESTRAS_VALIDAS   20

/* =========================
 * FUNCIONES PRIVADAS
 * ========================= */
static float calcular_desviacion_estandar(const float *arreglo, int n, float media)
{
    double suma_cuadrados = 0.0;
    for (int i = 0; i < n; i++) {
        double diferencia = arreglo[i] - media;
        suma_cuadrados += diferencia * diferencia;
    }
    return (float)sqrt(suma_cuadrados / n);
}

static float calcular_rms(const float *arreglo, int n)
{
    double suma = 0.0;
    for (int i = 0; i < n; i++) {
        suma += (double)arreglo[i] * (double)arreglo[i];
    }
    return (float)sqrt(suma / n);
}

static void filtro_pasabajos_iir(const float *entrada, float *salida, int n, float alpha)
{
    if (n <= 0) {
        return;
    }

    salida[0] = entrada[0];
    for (int i = 1; i < n; i++) {
        salida[i] = alpha * salida[i - 1] + (1.0f - alpha) * entrada[i];
    }
}

static void eliminar_componente_dc(const float *entrada, float *componente_ac, int n, float alpha_dc)
{
    if (n <= 0) {
        return;
    }

    float estimacion_dc = entrada[0];
    for (int i = 0; i < n; i++) {
        estimacion_dc = alpha_dc * estimacion_dc + (1.0f - alpha_dc) * entrada[i];
        componente_ac[i] = entrada[i] - estimacion_dc;
    }
}

static int detectar_picos_y_calcular_bpm(const float *senal_ac, int n, float fs)
{
    if (senal_ac == NULL || n < 10 || fs <= 0.0f) {
        return -1;
    }

    float media = 0.0f;
    for (int i = 0; i < n; i++) {
        media += senal_ac[i];
    }
    media /= (float)n;

    float desviacion_estandar = calcular_desviacion_estandar(senal_ac, n, media);

    if (desviacion_estandar < 0.2f) {
        return -1;
    }

    float umbral_adaptativo = media + 0.15f * desviacion_estandar;
    int periodo_refractario_muestras = (int)(0.25f * fs);

    int indices_picos[MAX30100_MAX_PICOS];
    int total_picos_detectados = 0;
    int indice_ultimo_pico = -1000;

    for (int i = 2; i < n - 2; i++) {
        bool es_maximo_local =
            senal_ac[i] > umbral_adaptativo &&
            senal_ac[i] > senal_ac[i - 1] &&
            senal_ac[i] > senal_ac[i - 2] &&
            senal_ac[i] >= senal_ac[i + 1] &&
            senal_ac[i] >= senal_ac[i + 2];

        if (es_maximo_local && (i - indice_ultimo_pico) >= periodo_refractario_muestras) {
            if (total_picos_detectados < MAX30100_MAX_PICOS) {
                indices_picos[total_picos_detectados++] = i;
            }
            indice_ultimo_pico = i;
        }
    }

    ESP_LOGD(TAG_MAX30100, "SD=%.3f | Umbral=%.3f | Picos=%d",
             desviacion_estandar, umbral_adaptativo, total_picos_detectados);

    if (total_picos_detectados < 2) {
        return -1;
    }

    float suma_intervalos = 0.0f;
    int intervalos_validos = 0;

    for (int i = 1; i < total_picos_detectados; i++) {
        int delta_muestras = indices_picos[i] - indices_picos[i - 1];
        float delta_segundos = (float)delta_muestras / fs;

        if (delta_segundos > 0.0f) {
            suma_intervalos += delta_segundos;
            intervalos_validos++;
        }
    }

    if (intervalos_validos == 0) {
        return -1;
    }

    float intervalo_promedio_segundos = suma_intervalos / (float)intervalos_validos;
    float bpm = 60.0f / intervalo_promedio_segundos;

    if (bpm < 40.0f || bpm > 180.0f) {
        return -1;
    }

    return (int)(bpm + 0.5f);
}

static int estimar_calidad_senal_max30100(float ir_promedio,
                                          float ir_ac_rms,
                                          float rojo_promedio,
                                          float rojo_ac_rms,
                                          float ir_pico_a_pico)
{
    int calidad = 0;

    if (ir_promedio > 12000.0f)  calidad += 25;
    if (rojo_promedio > 12000.0f) calidad += 15;
    if (ir_ac_rms > 80.0f)       calidad += 20;
    if (rojo_ac_rms > 50.0f)     calidad += 10;
    if (ir_pico_a_pico > 300.0f) calidad += 30;

    if (calidad > 100) calidad = 100;
    if (calidad < 0)   calidad = 0;

    return calidad;
}

static float calcular_spo2(float ir_promedio, float ir_ac_rms,
                           float rojo_promedio, float rojo_ac_rms)
{
    if (ir_promedio <= 1.0f || rojo_promedio <= 1.0f ||
        ir_ac_rms <= 5.0f   || rojo_ac_rms <= 5.0f) {
        return -1.0f;
    }

    float ratio_r = (rojo_ac_rms / rojo_promedio) / (ir_ac_rms / ir_promedio);

    float spo2 = 104.0f - 17.0f * ratio_r;

    if (spo2 > 100.0f) spo2 = 100.0f;
    if (spo2 < 80.0f)  spo2 = 80.0f;

    return spo2;
}

/* =========================
 * I2C PRIVADO
 * ========================= */
static esp_err_t max30100_escribir_registro(uint8_t registro, uint8_t dato)
{
    uint8_t buffer_tx[2] = { registro, dato };

    return i2c_master_write_to_device(
        I2C_PUERTO_MAX30100,
        MAX30100_DIRECCION_I2C,
        buffer_tx,
        sizeof(buffer_tx),
        pdMS_TO_TICKS(I2C_TIMEOUT_MS)
    );
}

static esp_err_t max30100_leer_registro(uint8_t registro, uint8_t *datos, size_t longitud)
{
    return i2c_master_write_read_device(
        I2C_PUERTO_MAX30100,
        MAX30100_DIRECCION_I2C,
        &registro,
        1,
        datos,
        longitud,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS)
    );
}

/* =========================
 * API PÚBLICA
 * ========================= */
esp_err_t max30100_inicializar_i2c(void)
{
    i2c_config_t configuracion_i2c_max30100 = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_GPIO_SDA_MAX30100,
        .scl_io_num = I2C_GPIO_SCL_MAX30100,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FRECUENCIA_MAX30100_HZ,
        .clk_flags = 0
    };

    esp_err_t estado = i2c_param_config(I2C_PUERTO_MAX30100, &configuracion_i2c_max30100);
    if (estado != ESP_OK) {
        ESP_LOGE(TAG_MAX30100, "Error config I2C MAX30100: %s", esp_err_to_name(estado));
        return estado;
    }

    estado = i2c_driver_install(I2C_PUERTO_MAX30100, I2C_MODE_MASTER, 0, 0, 0);
    if (estado != ESP_OK && estado != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG_MAX30100, "Error instalando driver I2C MAX30100: %s", esp_err_to_name(estado));
        return estado;
    }

    ESP_LOGI(TAG_MAX30100, "I2C MAX30100 inicializado");
    return ESP_OK;
}

esp_err_t max30100_inicializar_sensor(void)
{
    uint8_t part_id = 0;
    esp_err_t estado = max30100_leer_registro(MAX30100_REG_PART_ID, &part_id, 1);
    if (estado != ESP_OK) {
        ESP_LOGE(TAG_MAX30100, "MAX30100 no responde");
        return estado;
    }

    ESP_LOGI(TAG_MAX30100, "PART_ID = 0x%02X", part_id);

    estado = max30100_escribir_registro(MAX30100_REG_MODE_CONFIG, MAX30100_RESET_BIT);
    if (estado != ESP_OK) {
        return estado;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_FIFO_WR_PTR, 0x00));
    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_OVF_COUNTER, 0x00));
    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_FIFO_RD_PTR, 0x00));

    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_MODE_CONFIG, MAX30100_MODE_SPO2));

    uint8_t config_spo2 = MAX30100_SPO2_HI_RES_EN |
                          (MAX30100_SR_100HZ << 2) |
                          MAX30100_LED_PW_1600US;
    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_SPO2_CONFIG, config_spo2));

    uint8_t config_leds = (MAX30100_LED_CURR_27MA << 4) | MAX30100_LED_CURR_27MA;
    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_LED_CONFIG, config_leds));

    ESP_ERROR_CHECK(max30100_escribir_registro(MAX30100_REG_INT_ENABLE, 0x00));

    ESP_LOGI(TAG_MAX30100, "MAX30100 configurado");
    return ESP_OK;
}

esp_err_t max30100_leer_datos_crudos(MuestraMax30100_t *muestra)
{
    if (muestra == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t datos_fifo[4];
    esp_err_t estado = max30100_leer_registro(MAX30100_REG_FIFO_DATA, datos_fifo, 4);
    if (estado != ESP_OK) {
        return estado;
    }

    muestra->ir  = ((uint16_t)datos_fifo[0] << 8) | datos_fifo[1];
    muestra->red = ((uint16_t)datos_fifo[2] << 8) | datos_fifo[3];

    return ESP_OK;
}

void max30100_monitorear_signos_vitales(bool modo_alerta, ResultadoRitmo_t *resultado)
{
    if (resultado == NULL) {
        ESP_LOGE(TAG_MAX30100, "Puntero resultado es NULL");
        return;
    }

    int cantidad_muestras = modo_alerta ? MUESTRAS_MAX30100_ALERTA : MUESTRAS_MAX30100_NORMAL;

    MuestraMax30100_t *muestras_crudas = malloc(cantidad_muestras * sizeof(MuestraMax30100_t));
    float *senal_ir_cruda = malloc(cantidad_muestras * sizeof(float));
    float *senal_roja_cruda = malloc(cantidad_muestras * sizeof(float));
    float *senal_ir_filtrada = malloc(cantidad_muestras * sizeof(float));
    float *senal_roja_filtrada = malloc(cantidad_muestras * sizeof(float));
    float *componente_ac_ir = malloc(cantidad_muestras * sizeof(float));
    float *componente_ac_roja = malloc(cantidad_muestras * sizeof(float));

    if (!muestras_crudas || !senal_ir_cruda || !senal_roja_cruda ||
        !senal_ir_filtrada || !senal_roja_filtrada ||
        !componente_ac_ir || !componente_ac_roja) {
        ESP_LOGE(TAG_MAX30100, "Sin memoria para buffers");
        resultado->bpm_calculado = -1;
        resultado->spo2_estimada = -1.0f;
        resultado->contacto_detectado = false;
        resultado->resultado_valido = false;
        resultado->calidad_senal = 0;
        resultado->ir_promedio = 0.0f;
        strcpy(resultado->estado_texto, "Sin memoria");
        goto cleanup;
    }

    ESP_LOGI(TAG_MAX30100, "Iniciando monitoreo (%d muestras)...", cantidad_muestras);

    int cantidad_muestras_validas = 0;
    for (int i = 0; i < cantidad_muestras; i++) {
        if (max30100_leer_datos_crudos(&muestras_crudas[i]) == ESP_OK) {
            cantidad_muestras_validas++;
        } else {
            muestras_crudas[i].ir = 0;
            muestras_crudas[i].red = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(DELAY_MUESTRAS_MAX30100_MS));
    }

    max30100_escribir_registro(MAX30100_REG_FIFO_WR_PTR, 0x00);
    max30100_escribir_registro(MAX30100_REG_OVF_COUNTER, 0x00);
    max30100_escribir_registro(MAX30100_REG_FIFO_RD_PTR, 0x00);
    vTaskDelay(pdMS_TO_TICKS(50));

    if (cantidad_muestras_validas < MAX30100_MIN_MUESTRAS_VALIDAS) {
        resultado->bpm_calculado = -1;
        resultado->spo2_estimada = -1.0f;
        resultado->contacto_detectado = false;
        resultado->resultado_valido = false;
        resultado->calidad_senal = 0;
        resultado->ir_promedio = 0.0f;
        strcpy(resultado->estado_texto, "Error de lectura I2C");
        goto cleanup;
    }

    double acumulado_ir = 0.0;
    double acumulado_rojo = 0.0;

    for (int i = 0; i < cantidad_muestras; i++) {
        senal_ir_cruda[i] = (float)muestras_crudas[i].ir;
        senal_roja_cruda[i] = (float)muestras_crudas[i].red;
        acumulado_ir += muestras_crudas[i].ir;
        acumulado_rojo += muestras_crudas[i].red;
    }

    resultado->ir_promedio = (float)(acumulado_ir / cantidad_muestras);
    float promedio_rojo = (float)(acumulado_rojo / cantidad_muestras);

    resultado->contacto_detectado = (resultado->ir_promedio > 10000.0f);

    if (!resultado->contacto_detectado) {
        resultado->bpm_calculado = -1;
        resultado->spo2_estimada = -1.0f;
        resultado->resultado_valido = false;
        resultado->calidad_senal = 0;
        strcpy(resultado->estado_texto, "Sin contacto");
        goto cleanup;
    }

    filtro_pasabajos_iir(senal_ir_cruda, senal_ir_filtrada, cantidad_muestras, 0.70f);
    filtro_pasabajos_iir(senal_roja_cruda, senal_roja_filtrada, cantidad_muestras, 0.70f);

    eliminar_componente_dc(senal_ir_filtrada, componente_ac_ir, cantidad_muestras, 0.90f);
    eliminar_componente_dc(senal_roja_filtrada, componente_ac_roja, cantidad_muestras, 0.90f);

    float ir_rms_ac = calcular_rms(componente_ac_ir, cantidad_muestras);
    float rojo_rms_ac = calcular_rms(componente_ac_roja, cantidad_muestras);

    float ir_ac_maximo = componente_ac_ir[0];
    float ir_ac_minimo = componente_ac_ir[0];
    for (int i = 1; i < cantidad_muestras; i++) {
        if (componente_ac_ir[i] > ir_ac_maximo) ir_ac_maximo = componente_ac_ir[i];
        if (componente_ac_ir[i] < ir_ac_minimo) ir_ac_minimo = componente_ac_ir[i];
    }
    float ir_amplitud_pico_a_pico = ir_ac_maximo - ir_ac_minimo;

    resultado->calidad_senal = estimar_calidad_senal_max30100(
        resultado->ir_promedio,
        ir_rms_ac,
        promedio_rojo,
        rojo_rms_ac,
        ir_amplitud_pico_a_pico
    );

    if (resultado->calidad_senal < 35) {
        resultado->bpm_calculado = -1;
        resultado->spo2_estimada = -1.0f;
        resultado->resultado_valido = false;
        strcpy(resultado->estado_texto, "Señal débil");
        goto cleanup;
    }

    resultado->bpm_calculado = detectar_picos_y_calcular_bpm(
        componente_ac_ir,
        cantidad_muestras,
        (float)MAX30100_SAMPLE_RATE_HZ
    );

    resultado->spo2_estimada = calcular_spo2(
        resultado->ir_promedio,
        ir_rms_ac,
        promedio_rojo,
        rojo_rms_ac
    );

    if (resultado->bpm_calculado < 0) {
        resultado->resultado_valido = false;
        strcpy(resultado->estado_texto, "Pulso inestable");
    } else if (resultado->spo2_estimada < 0.0f) {
        resultado->resultado_valido = false;
        strcpy(resultado->estado_texto, "SpO2 inestable");
    } else {
        resultado->resultado_valido = true;
        strcpy(resultado->estado_texto, "Lectura válida");
    }

    ESP_LOGI(TAG_MAX30100, "BPM: %d | SpO2: %.1f%% | Calidad: %d/100",
             resultado->bpm_calculado, resultado->spo2_estimada, resultado->calidad_senal);
    ESP_LOGD(TAG_MAX30100, "IR prom: %.2f | IR AC RMS: %.2f | RED AC RMS: %.2f | P-P: %.2f",
             resultado->ir_promedio, ir_rms_ac, rojo_rms_ac, ir_amplitud_pico_a_pico);

cleanup:
    free(muestras_crudas);
    free(senal_ir_cruda);
    free(senal_roja_cruda);
    free(senal_ir_filtrada);
    free(senal_roja_filtrada);
    free(componente_ac_ir);
    free(componente_ac_roja);
}