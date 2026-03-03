#ifndef TASKS_H
#define TASKS_H

#include "AsyncTaskLib.h"
#include "Sensors.h"

extern AsyncTask taskEntrada;

//tareas del sensor de luz
extern AsyncTask taskLeer;
extern AsyncTask taskPromediar;
extern AsyncTask taskEnviar;
//tareas del sensor de flama
extern AsyncTask taskLeer;



#endif
