#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"

// Variables compartidas de sensores
extern char entrada;
extern bool ledState;
extern int conteoAlerta;
extern bool t5;
extern bool t3;

// Funciones de los diferentes sensores y procesos complementarios
void leerEntrada();
void greenLed();
void blueLed();
void redLed();

//Transiciones de tiempo
void DoneT5();
void DoneT3();

#endif