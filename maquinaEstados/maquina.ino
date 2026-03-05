#include "Pinout.h"
#include "Sensors.h"
#include "Tasks.h"
#include "FSM.h"

// Definición de variables globales (una sola vez aquí)
bool ledState = false;
int conteoAlerta = 0;
bool t5 = false;
bool t3 = false;
bool t2 = false;
bool t4 = false;
bool luz = false;
bool tem1 = false;
bool tem2 = false;
bool hum1 = false;
bool hum2 = false;
Event eventoActual = DEFECTO;

/**
 * @brief Configura el hardware y prepara el sistema para iniciar la ejecución.
 *
 * Inicializa pines de salida/entrada (LED RGB, sensor de flama y botón),
 * configura el ADC del ESP32, arranca el sensor DHT, habilita la comunicación
 * serial y configura la máquina de estados estableciendo el estado inicial.
 */
void setup() {
  pinMode(GLEDPIN, OUTPUT);
  pinMode(BLEDPIN, OUTPUT);
  pinMode(RLEDPIN, OUTPUT);
  pinMode(FLAMPIN, INPUT);  // Activo en LOW
  pinMode(BTNPIN, INPUT_PULLUP);   // S del HW-483
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  dht.begin();
  Serial.begin(9600);
  setupMachine();
  machine.SetState(INICIO, false, true);
}

/**
 * @brief Ciclo principal de ejecución del sistema.
 *
 * Actualiza la máquina de estados y ejecuta el planificador cooperativo
 * basado en tareas. Cada llamada a Update() permite que las tareas
 * asociadas (LEDs, temporizadores, sensores y botón) se ejecuten cuando
 * corresponda según sus intervalos configurados.
 */
void loop() {
  machine.Update();
  taskEncenderLedG.Update();
  taskEncenderLedB.Update();
  taskEncenderLedR.Update();
  taskT5.Update();
  taskT3.Update();
  taskT2.Update();
  taskT4.Update();
  taskFlama.Update();
  taskLeerLuz.Update();
  taskLeerTem.Update();
  taskEnviar.Update();
  taskLeerHum.Update();
  taskBoton.Update();
}