#include <Arduino.h>
#include "AsyncTaskLib.h"
#include "DHT.h"

// ===================== CONFIGURACIÓN =====================
#define DHTPIN     4
#define DHTTYPE    DHT11

#define LDR_PIN    34   // HW-486: usar AO a un pin ADC (ESP32)

// RGB (4 pines): R, G, B a GPIO
#define PIN_R      19
#define PIN_G      18
#define PIN_B      5

// Si tu RGB es ÁNODO COMÚN pon esto en 1 (LOW enciende). Si es CÁTODO COMÚN pon 0 (HIGH enciende).
#define RGB_ANODO_COMUN 1

// Umbrales alarma
const float TEMP_UMBRAL_C = 25.0;  // activa si T > 25
const int   LUZ_UMBRAL_PCT = 60;   // activa si Luz < 60 (%)

// ===================== OBJETOS Y VARIABLES =====================
DHT dht(DHTPIN, DHTTYPE);

volatile float temperaturaC = NAN;
volatile float humedadPct   = NAN;
volatile int   luzRaw       = 0;   // 0..4095
volatile int   luzPct       = 0;   // 0..100
volatile bool  alarmaActiva = false;

// ---- Acumuladores para promedios en ventana de 7s ----
double sumTemp = 0.0;
double sumHum  = 0.0;
long   sumLuz  = 0;

unsigned long countTemp = 0;
unsigned long countHum  = 0;
unsigned long countLuz  = 0;

// ===================== FUNCIONES AUXILIARES (RGB) =====================
inline void rgbWrite(bool rOn, bool gOn, bool bOn) {
#if RGB_ANODO_COMUN
  // ÁNODO COMÚN: LOW enciende, HIGH apaga
  digitalWrite(PIN_R, rOn ? LOW : HIGH);
  digitalWrite(PIN_G, gOn ? LOW : HIGH);
  digitalWrite(PIN_B, bOn ? LOW : HIGH);
#else
  // CÁTODO COMÚN: HIGH enciende, LOW apaga
  digitalWrite(PIN_R, rOn ? HIGH : LOW);
  digitalWrite(PIN_G, gOn ? HIGH : LOW);
  digitalWrite(PIN_B, bOn ? HIGH : LOW);
#endif
}

inline void rgbOff()   { rgbWrite(true, true, true); }
inline void rgbRedOn() { rgbWrite(true,  false, false); }

// ===================== PROTOTIPOS =====================
void leerTemperatura();
void leerLuz();
void leerHumedad();
void imprimirSerial();

void evaluarAlarma();
void activarAlarma();
void desactivarAlarma();

void alarmaOn();
void alarmaOff();

void resetPromedios();

// ===================== TAREAS (5) =====================
// T1: temperatura cada 1.5 s
AsyncTask tTemp(1500, true, leerTemperatura);

// T2: luz cada 1.0 s
AsyncTask tLuz(1000, true, leerLuz);

// T3: humedad cada 1.8 s
AsyncTask tHum(1800, true, leerHumedad);

// T4: imprimir cada 7.0 s
AsyncTask tPrint(7000, true, imprimirSerial);

// T5: alarma visual (ROJO del RGB): 500ms ON / 900ms OFF
AsyncTask tAlarmaOn (500, alarmaOn);
AsyncTask tAlarmaOff(900, alarmaOff);

// ===================== IMPLEMENTACIÓN =====================
void leerTemperatura() {
  float t = dht.readTemperature(); // °C
  if (!isnan(t)) {
    temperaturaC = t;

    // acumular para promedio de 7s
    sumTemp += t;
    countTemp++;
  }
  evaluarAlarma();
}

void leerHumedad() {
  float h = dht.readHumidity(); // %
  if (!isnan(h)) {
    humedadPct = h;

    // acumular para promedio de 7s
    sumHum += h;
    countHum++;
  }
  evaluarAlarma();
}

void leerLuz() {
  // IMPORTANTE: HW-486 usar AO al pin ADC
  int raw = analogRead(LDR_PIN);
  luzRaw = raw;

  // % simple (0..100)
  int pct = (raw * 100) / 4095;

  // Si notas que con MÁS luz te baja el número, invierte:
  // pct = 100 - pct;

  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  luzPct = pct;

  // acumular para promedio de 7s
  sumLuz += pct;
  countLuz++;

  evaluarAlarma();
}

void imprimirSerial() {
  // calcular promedios del intervalo (últimos 7s)
  bool okTemp = (countTemp > 0);
  bool okHum  = (countHum > 0);
  bool okLuz  = (countLuz > 0);

  float promTemp = okTemp ? (float)(sumTemp / (double)countTemp) : NAN;
  float promHum  = okHum  ? (float)(sumHum  / (double)countHum)  : NAN;
  float promLuz  = okLuz  ? (float)((double)sumLuz / (double)countLuz) : NAN;

  Serial.print(millis());
  Serial.print(" ms | PROM 7s -> T=");

  if (isnan(promTemp)) Serial.print("N/A");
  else Serial.print(promTemp, 1);

  Serial.print(" C | H=");
  if (isnan(promHum)) Serial.print("N/A");
  else Serial.print(promHum, 1);

  Serial.print(" % | Luz=");
  if (isnan(promLuz)) Serial.print("N/A");
  else Serial.print(promLuz, 0);

  Serial.print("% | Alarma=");
  Serial.println(alarmaActiva ? "ON" : "OFF");

  // reiniciar acumuladores para la siguiente ventana de 7s
  resetPromedios();
}

void resetPromedios() {
  sumTemp = 0.0; sumHum = 0.0; sumLuz = 0;
  countTemp = 0; countHum = 0; countLuz = 0;
}

// ----- Alarma (condición + blink sin delay) -----
void evaluarAlarma() {
  // Si no hay lectura válida de T, apagamos por seguridad
  if (isnan(temperaturaC)) {
    if (alarmaActiva) desactivarAlarma();
    return;
  }

  bool condicion = (temperaturaC > TEMP_UMBRAL_C) && (luzPct < LUZ_UMBRAL_PCT);

  if (condicion && !alarmaActiva) activarAlarma();
  else if (!condicion && alarmaActiva) desactivarAlarma();
}

void activarAlarma() {
  alarmaActiva = true;
  rgbOff();          // estado inicial
  tAlarmaOn.Start(); // empieza ON (OFF se encadena en loop)
}

void desactivarAlarma() {
  alarmaActiva = false;
  tAlarmaOn.Stop();
  tAlarmaOff.Stop();
  rgbOff();
}

void alarmaOn() {
  // Enciende SOLO rojo (canal R en D19/GPIO19)
  rgbRedOn();
}

void alarmaOff() {
  rgbOff();
}

// ===================== SETUP / LOOP =====================
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  rgbOff();

  dht.begin();

  resetPromedios();

  // Iniciar tareas 1-4
  tTemp.Start();
  tLuz.Start();
  tHum.Start();
  tPrint.Start();

  // T5 se activa solo si cumple condición
}

void loop() {
  // Tareas “simultáneas”
  tTemp.Update();
  tLuz.Update();
  tHum.Update();
  tPrint.Update();

  // Alarma con BlinkWithoutDelay alternado (estilo ejemplo)
  if (alarmaActiva) {
    tAlarmaOn.Update(tAlarmaOff);
    tAlarmaOff.Update(tAlarmaOn);
  }
}