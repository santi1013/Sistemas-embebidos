#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"
#include "AsyncTaskLib.h"

#define dhtipo DHT11

#define numu 4

Average<float> promedio(numu);
float luzPromedio = 0.0f;
// Variables compartidas de sensores
extern char entrada;
extern bool ledState;
extern bool boton;
extern bool isOn;
extern bool flamaState;
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

//Sensor dht11
void leerTemperatura(void);
void promTemperatura(void);
void enviarTemperatura(void);

#endif







