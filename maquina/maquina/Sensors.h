#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"
#include <DHT.h>
#include "AsyncTaskLib.h"

#define dhtipo DHT11
#define numu 4

// Variables compartidas de sensores
extern char entrada;
extern bool ledState;
extern bool boton;


// Funciones de sensores/actuadores
void leerEntrada();
void toggleLed1();
void toggleLed2();
void rgbRed();
void rgbGreen();
void rgbBlue();
void readButton();
void leerTemperatura();
void promTemperatura();
void enviarTemperatura();



#endif