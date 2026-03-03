#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"

// Variables compartidas de sensores
extern char entrada;
extern bool ledState;
extern bool boton;
// Funciones de los diferentes sensores y procesos complementarios
void leerEntrada();
void toggleLed1();
void toggleLed2();
// Funciones de el ldr
void leerLuz(void);
void promLuz(void);
void enviarLuz(void);
//funcion del sensor de flama 
void leerFlama(void);
void rgbRed();
void rgbGreen();
void rgbBlue();
void readButton();

#endif


