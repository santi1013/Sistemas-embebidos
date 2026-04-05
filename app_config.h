#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include "driver/i2c.h"

/* ============================================================
 * CONFIGURACIÓN DE PINES
 * ============================================================ */

// MAX30100 (ritmo cardíaco)
#define MAX30100_I2C_SDA_GPIO           15
#define MAX30100_I2C_SCL_GPIO           16

// QMI8658 (IMU)
#define QMI8658_I2C_SDA_GPIO            6
#define QMI8658_I2C_SCL_GPIO            7

// LED y botón
#define LED_GPIO                        11
#define BUTTON_GPIO                     10

/* ============================================================
 * CONFIGURACIÓN I2C GENERAL
 * ============================================================ */

#define I2C_MASTER_FREQ_HZ              100000
#define I2C_TIMEOUT_MS                  1000

// Direcciones
#define MAX30100_I2C_ADDRESS            0x57
#define QMI8658_I2C_ADDRESS             0x6B   // (verifica tu módulo)

/* ============================================================
 * ALIAS POR SENSOR
 * ============================================================ */

// MAX30100
#define I2C_PUERTO_MAX30100             I2C_NUM_0
#define I2C_GPIO_SDA_MAX30100           MAX30100_I2C_SDA_GPIO
#define I2C_GPIO_SCL_MAX30100           MAX30100_I2C_SCL_GPIO
#define I2C_FRECUENCIA_MAX30100_HZ      I2C_MASTER_FREQ_HZ
#define MAX30100_DIRECCION_I2C          MAX30100_I2C_ADDRESS

// QMI8658
#define I2C_PUERTO_QMI8658              I2C_NUM_1
#define I2C_GPIO_SDA_QMI8658            QMI8658_I2C_SDA_GPIO
#define I2C_GPIO_SCL_QMI8658            QMI8658_I2C_SCL_GPIO
#define I2C_FRECUENCIA_QMI8658_HZ       I2C_MASTER_FREQ_HZ
#define QMI8658_DIRECCION_I2C           QMI8658_I2C_ADDRESS

/* ============================================================
 * TIEMPOS DE LA MÁQUINA DE ESTADOS
 * ============================================================ */

#define TIEMPO_ESTADO_MS                2000   // 2 segundos

/* ============================================================
 * CONFIGURACIÓN MAX30100
 * ============================================================ */

#define MAX30100_SAMPLE_RATE_HZ         50

#define MUESTRAS_MAX30100_NORMAL        200
#define MUESTRAS_MAX30100_ALERTA        200

#define DELAY_MUESTRAS_MAX30100_MS      10

/* ============================================================
 * CONFIGURACIÓN RITMO CARDÍACO
 * ============================================================ */

#define RITMO_MIN_LPM                   55.0f
#define RITMO_MAX_LPM                   75.0f

/* ============================================================
 * CONFIGURACIÓN ALERTAS
 * ============================================================ */

#define MAX_ALERTAS_AMARILLAS           5
#define MAX_ALERTAS_ROJAS               3

/* ============================================================
 * MICROSD POR SPI
 * ============================================================ */
#define SD_MOSI_GPIO                    18
#define SD_MISO_GPIO                    17
#define SD_SCK_GPIO                     33
#define SD_CS_GPIO                      21

#define SD_MOUNT_POINT                  "/Documents"
#define SD_LOG_FILE_PATH                "/Documents/Sensores.txt"

#endif