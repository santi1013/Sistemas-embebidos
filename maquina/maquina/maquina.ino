#include "Pinout.h"
#include "Sensors.h"
#include "Tasks.h"
#include "FSM.h"

// Definición de variables globales (una sola vez aquí)
char entrada = ' ';
bool ledState = false;
int conteoAlerta = 0;
bool t5 = false;
bool t3 = false;

void setup() {
  pinMode(GLEDPIN, OUTPUT);
  pinMode(BLEDPIN, OUTPUT);
  pinMode(RLEDPIN, OUTPUT);
  Serial.begin(9600);
  setupMachine();
  machine.SetState(INICIO, false, true);
}

void loop() {
  machine.Update();
  taskEntrada.Update();
  taskEncenderLedG.Update();
  taskEncenderLedB.Update();
  taskEncenderLedR.Update();
  taskT5.Update();
  taskT3.Update();
}