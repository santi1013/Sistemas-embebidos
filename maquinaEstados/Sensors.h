#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"
#include <DHT.h>

enum Event{
  DEFECTO = 0,
  BOTON,
  FLAMA,
  TEMPERATURA,
  HUMEDAD,
  LUZ
};

extern Event eventoActual;

// Variables compartidas de sensores
extern bool ledState;
extern int conteoAlerta;
extern int muestraFlama;
extern bool t5;
extern bool t3;
extern bool t2;
extern bool t4;

// Funciones de los diferentes sensores y procesos complementarios

/**
 * @brief Alterna el estado del LED verde.
 *
 * Invierte la variable ledState y actualiza el pin GLEDPIN para generar
 * el efecto de parpadeo del LED verde.
 */
void greenLed(void);

/**
 * @brief Alterna el estado del LED azul.
 *
 * Invierte la variable ledState y actualiza el pin BLEDPIN. Además ajusta
 * el intervalo de la tarea del LED azul para crear un parpadeo asimétrico.
 */
void blueLed(void);

/**
 * @brief Alterna el estado del LED rojo.
 *
 * Invierte la variable ledState y actualiza el pin RLEDPIN. Además ajusta
 * el intervalo de la tarea del LED rojo para definir el patrón de parpadeo.
 */
void redLed(void);

// Transiciones de tiempo

/**
 * @brief Señaliza el fin del temporizador T5.
 *
 * Activa la bandera t5 para habilitar transiciones basadas en tiempo
 * dentro de la máquina de estados.
 */
void DoneT5(void);

/**
 * @brief Señaliza el fin del temporizador T3.
 *
 * Activa la bandera t3 para habilitar transiciones basadas en tiempo
 * dentro de la máquina de estados.
 */
void DoneT3(void);

/**
 * @brief Señaliza el fin del temporizador T2.
 *
 * Activa la bandera t2 para habilitar transiciones basadas en tiempo
 * dentro de la máquina de estados.
 */
void DoneT2(void);

/**
 * @brief Señaliza el fin del temporizador T4.
 *
 * Activa la bandera t4 para habilitar transiciones basadas en tiempo
 * dentro de la máquina de estados.
 */
void DoneT4(void);

/**
 * @brief Lee el sensor de flama y genera el evento FLAMA.
 *
 * Realiza lectura digital del pin FLAMPIN. Si se detecta flama, asigna
 * el evento FLAMA para ser utilizado por la máquina de estados.
 */
void leerFlama(void);

// Funciones del sensor de luz

/**
 * @brief Toma una muestra del sensor de luz (LDR).
 *
 * Lee el valor analógico del pin LDRPIN y lo almacena para el cálculo
 * del promedio de luz.
 */
void leerLuz(void);

/**
 * @brief (No utilizado en la versión actual) Envía o procesa lectura de luz.
 *
 * Declaración reservada para funciones de envío/procesamiento de luz
 * si se requiere una separación adicional de responsabilidades.
 */
void enviarLuz(void);

/**
 * @brief Toma una muestra de temperatura desde el sensor DHT.
 *
 * Lee la temperatura del sensor DHT y almacena la lectura para el cálculo
 * del promedio de temperatura.
 */
void leerTem(void);

/**
 * @brief Toma una muestra de humedad desde el sensor DHT.
 *
 * Lee la humedad relativa del sensor DHT y almacena la lectura para el cálculo
 * del promedio de humedad.
 */
void leerHum(void);

/**
 * @brief Calcula promedios y define el evento del sistema.
 *
 * Procesa las muestras acumuladas para obtener promedios de temperatura,
 * humedad y luz, los imprime por Serial y asigna el eventoActual en función
 * de los umbrales configurados.
 */
void enviarDatos(void);

/**
 * @brief Lee el estado del botón y genera el evento BOTON.
 *
 * Implementa la lectura del botón conectado al pin BTNPIN con anti-rebote
 * y asigna el evento BOTON cuando se detecta una pulsación válida.
 */
void leerBoton(void);


#endif
