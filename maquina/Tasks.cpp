#include "Tasks.h"

AsyncTask taskEntrada(20,  true,  []() { leerEntrada(); });

//del sensor de luz
AsyncTask taskLeer(1800, true, []() { leerLuz(); });
AsyncTask taskEnviar(1, false, []() { enviarLuz(); });

//del sensor flama 
AsyncTask taskLeerFlama(periodoLectura, true, []() { leerFlama(); });

AsyncTask taskGreen  (200, true,  []() { rgbGreen();    });
AsyncTask taskBoton  (10, true,   []() { readButton();  });

AsyncTask taskBlue (200, true,  []() { rgbBlue();    });

// del sensor dht11
AsyncTask taskLeertemp(1800, true, []() { leerTemperatura(); });
AsyncTask taskPromediar(1, false, []() { promTemperatura(); });
AsyncTask taskEnviartemp(1, false, []() { enviarTemperatura(); });







