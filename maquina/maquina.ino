#include "Pinout.h"
#include "Sensors.h"
#include "Tasks.h"
#include "FSM.h"

// Definición de variables globales (una sola vez aquí)
char entrada = ' ';
bool ledState = false;

void setup() {
  pinMode(GLED, OUTPUT);
  Serial.begin(9600);
  setupMachine();
  machine.SetState(INICIO, false, true);
}

void loop() {
  machine.Update();
  taskEntrada.Update();
}