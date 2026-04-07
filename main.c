#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

static const char *TAG = "SD_WRITE";

// Pines MicroSD Adapter
#define PIN_NUM_MISO  17
#define PIN_NUM_MOSI  18
#define PIN_NUM_CLK   33
#define PIN_NUM_CS    21

#define MOUNT_POINT   "/sdcard"
#define FILE_NAME     MOUNT_POINT "/datos.txt"
#define REPEAT_COUNT  100  // Cuántas veces escribir "funciona"

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando escritura en MicroSD...");

    // Configuración del bus SPI
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al inicializar el bus SPI: %s", esp_err_to_name(ret));
        return;
    }

    // Configuración del dispositivo SD en SPI
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    // Configuración del sistema de archivos FAT
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_card_t *card;
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Error al montar el sistema de archivos. "
                     "¿La tarjeta SD está formateada en FAT32?");
        } else {
            ESP_LOGE(TAG, "Error al inicializar la tarjeta SD: %s", esp_err_to_name(ret));
        }
        spi_bus_free(host.slot);
        return;
    }

    ESP_LOGI(TAG, "MicroSD montada correctamente.");
    sdmmc_card_print_info(stdout, card);

    // Abrir (o crear) el archivo para escritura
    ESP_LOGI(TAG, "Abriendo archivo: %s", FILE_NAME);
    FILE *f = fopen(FILE_NAME, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Error al abrir el archivo para escritura");
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        spi_bus_free(host.slot);
        return;
    }

    // Escribir "funciona" REPEAT_COUNT veces
    ESP_LOGI(TAG, "Escribiendo %d líneas en el archivo...", REPEAT_COUNT);
    for (int i = 1; i <= REPEAT_COUNT; i++) {
        fprintf(f, "funciona %d\n", i);
    }

    fclose(f);
    ESP_LOGI(TAG, "Archivo escrito y cerrado correctamente.");

    // Verificar leyendo las primeras líneas
    ESP_LOGI(TAG, "Verificando contenido del archivo...");
    f = fopen(FILE_NAME, "r");
    if (f != NULL) {
        char line[64];
        int lines_shown = 0;
        while (fgets(line, sizeof(line), f) && lines_shown < 5) {
            // Quitar salto de línea al final para el log
            line[strcspn(line, "\n")] = '\0';
            ESP_LOGI(TAG, "  >> %s", line);
            lines_shown++;
        }
        ESP_LOGI(TAG, "  ... (y más líneas hasta %d)", REPEAT_COUNT);
        fclose(f);
    }

    // Desmontar la tarjeta SD
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    spi_bus_free(host.slot);
    ESP_LOGI(TAG, "Listo. Tarjeta SD desmontada de forma segura.");
}
