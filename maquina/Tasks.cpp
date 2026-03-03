#include "Tasks.h"

AsyncTask taskEntrada(20,  true,  []() { leerEntrada(); });

//del sensor de luz
AsyncTask taskLeer(1800, true, []() { leerLuz(); });
AsyncTask taskPromediar(1, false, []() { promLuz(); });
AsyncTask taskEnviar(1, false, []() { enviarLuz(); });

//del sensor flama 
AsyncTask taskLeer(periodoLectura, true, []() { leerFlama(); });

