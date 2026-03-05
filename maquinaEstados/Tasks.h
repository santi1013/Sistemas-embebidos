#ifndef TASKS_H
#define TASKS_H

#include "AsyncTaskLib.h"
#include "Sensors.h"

#define FLAMA_PERIOD 500
#define TASK_ENTRADA_PERIOD 20

#define LED_G_PERIOD 200
#define LED_B_PERIOD 500
#define LED_R_PERIOD 100

#define T5_PERIOD 5000
#define T3_PERIOD 3000
#define T2_PERIOD 2000
#define T4_PERIOD 4000

#define LUZ_READ_PERIOD 500

#define TEMP_READ_PERIOD 1500

#define HUM_READ_PERIOD 1500

#define SEND_PERIOD 300

#define BTN_PERIOD 20


/// Tarea encargada del parpadeo del LED verde en el estado inicial.
extern AsyncTask taskEncenderLedG;

/// Tarea encargada del parpadeo del LED azul cuando el sistema está en estado de alerta.
extern AsyncTask taskEncenderLedB;

/// Tarea encargada del parpadeo del LED rojo cuando el sistema está en estado de alarma.
extern AsyncTask taskEncenderLedR;

/// Temporizador T5 utilizado para generar transiciones de estado basadas en tiempo.
extern AsyncTask taskT5;

/// Temporizador T3 utilizado para controlar tiempos de permanencia en ciertos estados.
extern AsyncTask taskT3;

/// Temporizador T2 utilizado para habilitar ciertas transiciones dentro de la máquina de estados.
extern AsyncTask taskT2;

/// Temporizador T4 utilizado para generar eventos temporales en el monitoreo del sistema.
extern AsyncTask taskT4;

/// Tarea encargada de leer periódicamente el sensor de flama.
extern AsyncTask taskFlama;

/// Tarea encargada de leer muestras del sensor de luz (LDR).
extern AsyncTask taskLeerLuz;

/// Tarea encargada de leer muestras del sensor de temperatura del DHT.
extern AsyncTask taskLeerTem;

/// Tarea encargada de leer muestras del sensor de humedad del DHT.
extern AsyncTask taskLeerHum;

/// Tarea encargada de procesar los datos de sensores, calcular promedios
/// y generar eventos para la máquina de estados.
extern AsyncTask taskEnviar;

/// Tarea encargada de leer el estado del botón y generar el evento correspondiente.
extern AsyncTask taskBoton;

#endif