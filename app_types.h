#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * ESTADOS DEL SISTEMA
 * ============================================================ */

typedef enum {
    ESTADO_RITMO,
    ESTADO_SPO2,
    ESTADO_ACELEROMETRO,
    ESTADO_GIROSCOPIO
} estado_sistema_t;

/* ============================================================
 * MAX30100
 * ============================================================ */

typedef struct {
    uint16_t ir;
    uint16_t red;
} MuestraMax30100_t;

typedef struct {
    int bpm_calculado;
    float spo2_estimada;
    float ir_promedio;
    bool contacto_detectado;
    bool resultado_valido;
    int calidad_senal;
    char estado_texto[64];
} ResultadoRitmo_t;

/* ============================================================
 * QMI8658 (IMU)
 * ============================================================ */

typedef struct {
    float ax;
    float ay;
    float az;
} Acelerometro_t;

typedef struct {
    float gx;
    float gy;
    float gz;
} Giroscopio_t;

/* ============================================================
 * CONTEXTO DEL SISTEMA
 * ============================================================ */

typedef struct {
    estado_sistema_t estado_actual;
    uint32_t contador_alertas_amarillas;
    uint32_t contador_alertas_rojas;
} ContextoSistema_t;

#endif