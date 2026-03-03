#include "Tasks.h"

AsyncTask taskEntrada(20,  true,  []() { leerEntrada(); });

//del sensor de luz
AsyncTask taskLeer(1800, true, []() { leerLuz(); });
AsyncTask taskEnviar(1, false, []() { enviarLuz(); });

//del sensor flama 
AsyncTask taskLeer(periodoLectura, true, []() { leerFlama(); });

AsyncTask taskGreen  (200, true,  []() { rgbGreen();    });
AsyncTask taskBoton  (10, true,   []() { readButton();  });


