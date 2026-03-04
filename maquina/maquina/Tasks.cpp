#include "Tasks.h"

AsyncTask taskEntrada(20,  true,  []() { leerEntrada(); });

//LEDs
AsyncTask taskEncenderLedG(200,  true,  []() { greenLed(); });
AsyncTask taskEncenderLedB(500,  true,  []() { blueLed(); });
AsyncTask taskEncenderLedR(100,  true,  []() { redLed(); });

//Transiciones de tiempo
AsyncTask taskT5(5000, true, []() { DoneT5(); });
AsyncTask taskT3(3000, true, []() { DoneT3(); });
