#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Pinout.h"
#include <DHT.h>

enum Event{
  DEFECTO = 0,
  BOTON,
  FLAMA
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
extern bool luz;
extern bool tem1;
extern bool tem2;
extern bool hum1;
extern bool hum2;
extern DHT dht;

// Funciones de los diferentes sensores y procesos complementarios
void greenLed(void);
void blueLed(void);
void redLed(void);

//Transiciones de tiempo
void DoneT5(void);
void DoneT3(void);
void DoneT2(void);
void DoneT4(void);

void leerFlama(void);

//Funciones del sensor d eluz
void leerLuz(void);
void enviarLuz(void);

void leerTem(void);
void enviarTem(void);

void leerHum(void);
void enviarHum(void);

void leerBoton(void);


#endif