
# Nodo ESP32 + MPU6050 + microSD

## Descripción

Este proyecto implementa un nodo basado en **ESP32** desarrollado en **ESP-IDF** dentro de **VS Code**, cuyo objetivo es:

- leer datos del sensor **MPU6050** por **I2C**,
- calcular los ángulos **pitch** y **roll**,
- almacenar los registros en un archivo dentro de una **microSD**,
- y trabajar con una arquitectura modular usando **FreeRTOS**.

Actualmente el nodo funciona de forma **local**, es decir, **sin comunicación ESP-NOW** y sin intercambio de datos con otros nodos.

---

## Funcionalidad actual

El sistema realiza las siguientes tareas:

1. Inicializa las colas de FreeRTOS.
2. Inicializa el sensor **MPU6050**.
3. Inicializa y monta la **microSD** por SPI.
4. Ejecuta una prueba directa de escritura sobre la SD.
5. Crea las tareas del sistema:
   - lectura del sensor,
   - formateo de datos,
   - escritura en archivo.
6. Guarda registros continuos en la tarjeta microSD.

---

## Arquitectura del sistema

El flujo general del sistema es el siguiente:

**MPU6050 -> cola de datos crudos -> tarea de formateo -> cola de registros -> tarea de escritura SD -> archivo de log**

### Tareas principales

- **sensor_task**
  - Lee periódicamente el MPU6050.
  - Envía las muestras crudas a `g_raw_queue`.

- **format_local_task**
  - Recibe muestras desde `g_raw_queue`.
  - Calcula `pitch` y `roll`.
  - Construye una línea de texto con los datos.
  - Envía el registro a `g_log_queue`.

- **sd_writer_task**
  - Inicializa el logger de archivo.
  - Recibe registros desde `g_log_queue`.
  - Escribe cada línea en la microSD.

---

## Estructura del proyecto

```text
main/
│
├── main.c
├── config.h
├── data_types.h
├── task_manager.c
├── task_manager.h
├── mpu6050_drv.c
├── mpu6050_drv.h
├── sdcard_drv.c
├── sdcard_drv.h
├── file_logger.c
├── file_logger.h
├── logger.c
└── logger.h
