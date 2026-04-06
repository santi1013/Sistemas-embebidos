#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief Constantes de configuración global del proyecto.
 *
 * Este archivo centraliza la configuración del nodo basado en ESP32 para la
 * lectura del MPU6050, el acceso a la microSD y la administración de tareas
 * y colas en FreeRTOS.
 *
 * Agrupar estos parámetros en un solo archivo facilita el mantenimiento del
 * proyecto y permite modificar pines, periodos, prioridades y rutas de
 * almacenamiento sin cambiar la lógica interna de cada módulo.
 */

/* ============================================================================
 * CONFIGURACION I2C - MPU6050
 * ============================================================================
 */
/**
 * @brief Parámetros de comunicación I2C para el MPU6050.
 *
 * Este bloque define el puerto I2C maestro del ESP32, los pines SDA y SCL,
 * la frecuencia del bus y el tiempo máximo de espera en las transacciones.
 * También establece la dirección I2C del sensor MPU6050.
 *
 * Estas constantes son utilizadas por el driver del sensor para inicializar
 * la interfaz I2C y realizar lecturas de aceleración, velocidad angular y
 * temperatura.
 */
#define I2C_MASTER_NUM             I2C_NUM_0
#define I2C_MASTER_SCL_IO          22
#define I2C_MASTER_SDA_IO          21
#define I2C_MASTER_FREQ_HZ         100000
#define I2C_MASTER_TIMEOUT_MS      1000
#define MPU6050_I2C_ADDR           0x68


/* ============================================================================
 * CONFIGURACION SPI - MICRO SD
 * ============================================================================
 */
/**
 * @brief Parámetros de conexión SPI para la microSD.
 *
 * Este bloque define los pines empleados por el ESP32 para comunicarse con el
 * módulo lector de microSD mediante SPI: MOSI, MISO, SCLK y CS.
 *
 * Estas definiciones son utilizadas por el driver de la SD durante la 
 * inicialización del bus SPI y el montaje del sistema de archivos.
 */
#define SD_SPI_MOSI   23
#define SD_SPI_MISO   19
#define SD_SPI_SCLK   18
#define SD_SPI_CS     4


/* ============================================================================
 * CONFIGURACION DE FREE RTOS
 * ============================================================================
 */
/**
 * @brief Parámetros de temporización, colas, pilas y prioridades de tareas.
 *
 * Este bloque reúne la configuración base del sistema multitarea. Incluye:
 * - el periodo de muestreo del sensor,
 * - el periodo de tareas de estado,
 * - la longitud de las colas de comunicación,
 * - el tamaño de pila de cada tarea,
 * - y su prioridad de ejecución.
 *
 * Estos valores permiten coordinar el flujo de adquisición, procesamiento
 * y almacenamiento de datos dentro de FreeRTOS.
 */
#define SENSOR_SAMPLE_PERIOD_MS    500
#define STATUS_TASK_PERIOD_MS      2000

#define RAW_QUEUE_LENGTH           16
#define LOG_QUEUE_LENGTH           16

#define SENSOR_TASK_STACK          4096
#define FORMAT_TASK_STACK          4096
#define SD_TASK_STACK              4096
#define STATUS_TASK_STACK          3072

#define SENSOR_TASK_PRIORITY       4
#define FORMAT_TASK_PRIORITY       3
#define SD_TASK_PRIORITY           3
#define STATUS_TASK_PRIORITY       1


/* ============================================================================
 * CONFIGURACION DEL ARCHIVO DE LOG
 * ============================================================================
 */
/**
 * @brief Ruta del archivo de texto donde se almacenan los datos del MPU6050.
 *
 * Esta constante define el archivo principal de salida dentro de la microSD
 * montada en el sistema. Allí se guardan los registros formateados generados
 * por el nodo.
 */
#define SD_TEST_FILE_PATH          "/sdcard/sd_test.txt"
#define LOG_FILE_PATH              "/sdcard/mpu_log.txt"

#endif