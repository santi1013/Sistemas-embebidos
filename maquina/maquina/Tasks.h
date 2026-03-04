#ifndef TASKS_H
#define TASKS_H

#include "AsyncTaskLib.h"
#include "Sensors.h"

extern AsyncTask taskEntrada;

//INICIO
extern AsyncTask taskEncenderLedG;
extern AsyncTask taskEncenderLedB;
extern AsyncTask taskEncenderLedR;

//Transiciones de tiempo
extern AsyncTask taskT5;
extern AsyncTask taskT3;


#endif