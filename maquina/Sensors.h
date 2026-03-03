#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"

// Variables compartidas de sensores
extern char entrada;
extern bool ledState;

// Funciones de los diferentes sensores y procesos complementarios
void leerEntrada();
void toggleLed1();
void toggleLed2();

#endif