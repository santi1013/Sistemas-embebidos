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
#define LUZ_SEND_PERIOD 50

#define TEMP_READ_PERIOD 1500
#define TEMP_SEND_PERIOD 300

#define HUM_READ_PERIOD 1500
#define HUM_SEND_PERIOD 300

#define BTN_PERIOD 20


//INICIO
extern AsyncTask taskEncenderLedG;
extern AsyncTask taskEncenderLedB;
extern AsyncTask taskEncenderLedR;

//Transiciones de tiempo
extern AsyncTask taskT5;
extern AsyncTask taskT3;
extern AsyncTask taskT2;
extern AsyncTask taskT4;

extern AsyncTask taskFlama;

extern AsyncTask taskLeerLuz;
extern AsyncTask taskEnviarLuz;

extern AsyncTask taskLeerTem;
extern AsyncTask taskEnviarTem;

extern AsyncTask taskLeerHum;
extern AsyncTask taskEnviarHum;

extern AsyncTask taskBoton;
#endif