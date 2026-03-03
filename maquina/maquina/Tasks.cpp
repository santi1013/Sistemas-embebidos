#include "Tasks.h"

AsyncTask taskEntrada(20,  true,  []() { leerEntrada(); });
AsyncTask taskBlink1 (200, true,  []() { toggleLed1();  });
AsyncTask taskBlink2 (300, true,  []() { toggleLed2();  });
AsyncTask taskGreen  (200, true,  []() { rgbGreen();    });
AsyncTask taskBoton  (10, true,   []() { readButton();  });
AsyncTask taskLeer (1800, true, []() { leerTemperatura(); });
AsyncTask taskPromediar (1, false, []() { promTemperatura(); });
AsyncTask taskEnviar (1, false, []() { enviarTemperatura(); });