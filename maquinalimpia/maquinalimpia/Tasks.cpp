#include "Tasks.h"


AsyncTask taskEncenderLedG(LED_G_PERIOD, true, []() { greenLed(); });
AsyncTask taskEncenderLedB(LED_B_PERIOD, true, []() { blueLed(); });
AsyncTask taskEncenderLedR(LED_R_PERIOD, true, []() { redLed(); });

AsyncTask taskT5(T5_PERIOD, true, []() { DoneT5(); });
AsyncTask taskT3(T3_PERIOD, true, []() { DoneT3(); });
AsyncTask taskT2(T2_PERIOD, true, []() { DoneT2(); });
AsyncTask taskT4(T4_PERIOD, true, []() { DoneT4(); });


AsyncTask taskFlama(FLAMA_PERIOD, true, []() { leerFlama(); });

AsyncTask taskLeerLuz(LUZ_READ_PERIOD, true, []() { leerLuz(); });
AsyncTask taskEnviarLuz(LUZ_SEND_PERIOD, true, []() { enviarLuz(); });

AsyncTask taskLeerTem(TEMP_READ_PERIOD, true, []() { leerTem(); });
AsyncTask taskEnviarTem(TEMP_SEND_PERIOD, true, []() { enviarTem(); });

AsyncTask taskLeerHum(HUM_READ_PERIOD, true, []() { leerHum(); });
AsyncTask taskEnviarHum(HUM_SEND_PERIOD, true, []() { enviarHum(); });

AsyncTask taskBoton(BTN_PERIOD, true, []() { leerBoton(); });

