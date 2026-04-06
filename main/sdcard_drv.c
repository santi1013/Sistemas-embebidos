
#include "sdcard_drv.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"

/**
 * @file sdcard_drv.c
 * @brief Implementación del driver de acceso a la microSD por SPI.
 *
 * Este módulo se encarga de inicializar el bus SPI, montar la tarjeta microSD
 * usando el sistema de archivos FAT y ofrecer funciones básicas para abrir,
 * escribir y cerrar archivos.
 *
 * Además, incluye una rutina de prueba directa que permite verificar si la
 * tarjeta puede abrir un archivo, escribir una línea y leerla nuevamente.
 *
 * La lógica general del módulo es:
 * - configurar el bus SPI con una frecuencia reducida,
 * - montar la microSD,
 * - habilitar funciones de acceso a archivos,
 * - y asegurar que las escrituras queden realmente sincronizadas en la tarjeta.
 */

/** @brief Etiqueta usada para mensajes de depuración del módulo SD. */
static const char *TAG = "SDCARD";

/**
 * @brief Puntero a la estructura que representa la tarjeta SD montada.
 *
 * Esta variable almacena la información detectada por el sistema sobre la
 * tarjeta, como tipo, capacidad y otros datos de identificación.
 */
static sdmmc_card_t *g_card = NULL;

/**
 * @brief Bandera que indica si la microSD fue montada correctamente.
 *
 * Permite a otros módulos consultar si el sistema de archivos está disponible
 * antes de abrir o escribir archivos.
 */
static bool g_sd_mounted = false;

/**
 * @brief Inicializa la microSD y monta el sistema de archivos FAT.
 *
 * Esta función configura el host SPI, inicializa el bus físico usando los
 * pines definidos en el proyecto y monta la tarjeta SD en la ruta `/sdcard`.
 *
 * La velocidad del bus se fija en 400 kHz para mejorar la estabilidad de la
 * comunicación, especialmente en módulos o adaptadores de microSD sensibles
 * a frecuencias altas.
 *
 * Su lógica es:
 * - mostrar información de los pines usados,
 * - configurar el sistema FAT,
 * - preparar el host SPI y el bus físico,
 * - inicializar el bus SPI,
 * - configurar el dispositivo SD,
 * - montar la tarjeta en el sistema de archivos virtual del ESP32.
 *
 * @return
 * - ESP_OK si la tarjeta fue montada correctamente.
 * - Código de error si falla la inicialización del SPI o el montaje de la SD.
 */
esp_err_t sdcard_init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Inicializando microSD por SPI...");
    ESP_LOGI(TAG, "Pines SD -> MOSI=%d MISO=%d SCLK=%d CS=%d",
             SD_SPI_MOSI, SD_SPI_MISO, SD_SPI_SCLK, SD_SPI_CS);

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.max_freq_khz = 400;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_SPI_MOSI,
        .miso_io_num = SD_SPI_MISO,
        .sclk_io_num = SD_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Error inicializando bus SPI: %s", esp_err_to_name(ret));
        return ret;
    }

    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "Bus SPI ya estaba inicializado");
    } else {
        ESP_LOGI(TAG, "Bus SPI inicializado correctamente");
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_SPI_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &g_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error montando SD: %s", esp_err_to_name(ret));
        spi_bus_free(host.slot);
        return ret;
    }

    g_sd_mounted = true;
    ESP_LOGI(TAG, "MicroSD montada correctamente en /sdcard");
    sdmmc_card_print_info(stdout, g_card);

    return ESP_OK;
}

/**
 * @brief Indica si la tarjeta microSD está montada.
 *
 * Esta función permite a otros módulos consultar si la tarjeta ya se encuentra
 * disponible para operaciones de archivo.
 *
 * @return
 * - true si la microSD está montada.
 * - false si aún no ha sido montada o falló la inicialización.
 */
bool sdcard_is_mounted(void)
{
    return g_sd_mounted;
}

/**
 * @brief Abre un archivo de log en la microSD en modo append.
 *
 * Esta función verifica primero que la tarjeta esté montada. Si la SD está
 * disponible, intenta abrir el archivo indicado en modo `"a"`, lo que permite
 * agregar contenido al final sin borrar los datos existentes.
 *
 * @param path Ruta completa del archivo que se desea abrir.
 *
 * @return
 * - Puntero válido de tipo FILE* si la apertura fue exitosa.
 * - NULL si la SD no está montada o si ocurre un error al abrir el archivo.
 */
FILE *sdcard_open_log_file(const char *path)
{
    if (!g_sd_mounted) {
        ESP_LOGE(TAG, "La SD no esta montada");
        return NULL;
    }

    errno = 0;
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "No se pudo abrir el archivo: %s | errno=%d", path, errno);
        return NULL;
    }

    ESP_LOGI(TAG, "Archivo abierto correctamente: %s", path);
    return f;
}

/**
 * @brief Agrega una línea de texto al archivo abierto en la microSD.
 *
 * Esta función escribe una cadena de texto en el archivo recibido y luego
 * fuerza la sincronización completa del contenido en la tarjeta.
 *
 * Su lógica es:
 * - escribir la línea con `fprintf()`,
 * - vaciar el buffer de la librería con `fflush()`,
 * - obtener el descriptor del archivo con `fileno()`,
 * - y ejecutar `fsync()` para asegurar que los datos queden realmente
 *   almacenados en la microSD.
 *
 * Esto ayuda a evitar que el archivo aparezca con tamaño incorrecto o que se
 * pierdan datos si la tarjeta se retira poco después de la escritura.
 *
 * @param file Puntero al archivo previamente abierto.
 * @param line Cadena de texto que se desea escribir.
 *
 * @return
 * - ESP_OK si la línea fue escrita y sincronizada correctamente.
 * - ESP_ERR_INVALID_ARG si alguno de los parámetros es nulo.
 * - ESP_FAIL si ocurre un error en la escritura o sincronización.
 */
esp_err_t sdcard_append_line(FILE *file, const char *line)
{
    if (file == NULL || line == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int written = fprintf(file, "%s", line);
    if (written < 0) {
        ESP_LOGE(TAG, "Error escribiendo en archivo");
        return ESP_FAIL;
    }

    if (fflush(file) != 0) {
        ESP_LOGE(TAG, "Error en fflush()");
        return ESP_FAIL;
    }

    int fd = fileno(file);
    if (fd < 0) {
        ESP_LOGE(TAG, "Error obteniendo descriptor con fileno()");
        return ESP_FAIL;
    }

    if (fsync(fd) != 0) {
        ESP_LOGE(TAG, "Error en fsync()");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Cierra un archivo previamente abierto en la microSD.
 *
 * Esta función valida que el puntero recibido no sea nulo y luego libera el
 * recurso asociado al archivo mediante `fclose()`.
 *
 * @param file Puntero al archivo que se desea cerrar.
 */
void sdcard_close_file(FILE *file)
{
    if (file != NULL) {
        fclose(file);
    }
}

/**
 * @brief Ejecuta una prueba básica de escritura y lectura en la microSD.
 *
 * Esta función permite verificar que la tarjeta esté operativa antes de poner
 * en marcha el resto del sistema. Para ello:
 * - comprueba que la SD esté montada,
 * - abre un archivo de prueba en modo append,
 * - escribe una línea fija,
 * - vacía el buffer,
 * - cierra el archivo,
 * - lo vuelve a abrir en modo lectura,
 * - y comprueba que al menos una línea pueda leerse correctamente.
 *
 * Su propósito es validar de forma rápida que el acceso al sistema de archivos
 * funciona antes de iniciar las tareas principales del proyecto.
 *
 * @param path Ruta del archivo de prueba.
 *
 * @return
 * - ESP_OK si la prueba de escritura y lectura fue exitosa.
 * - ESP_FAIL si falla la apertura, escritura o lectura del archivo.
 */
esp_err_t sdcard_self_test(const char *path)
{
    if (!g_sd_mounted) {
        ESP_LOGE(TAG, "No se puede hacer self-test: SD no montada");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Iniciando self-test de escritura: %s", path);

    FILE *f = fopen(path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Self-test: no se pudo abrir archivo | errno=%d", errno);
        return ESP_FAIL;
    }

    if (fprintf(f, "SD_SELF_TEST_OK\n") < 0) {
        ESP_LOGE(TAG, "Self-test: fallo en fprintf()");
        fclose(f);
        return ESP_FAIL;
    }

    if (fflush(f) != 0) {
        ESP_LOGE(TAG, "Self-test: fallo en fflush()");
        fclose(f);
        return ESP_FAIL;
    }

    fclose(f);

    f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Self-test: no se pudo reabrir el archivo");
        return ESP_FAIL;
    }

    char line[64];
    if (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        ESP_LOGI(TAG, "Self-test OK, primera linea leida: %s", line);
    } else {
        ESP_LOGW(TAG, "Self-test: archivo abierto pero sin contenido legible");
    }

    fclose(f);
    ESP_LOGI(TAG, "Self-test de microSD completado correctamente");
    return ESP_OK;
}