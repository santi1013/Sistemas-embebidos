#include "Sensors.h"
#include "Tasks.h"

//Variables INICIO

void leerEntrada() {
  if (Serial.available()) entrada = Serial.read();
}

void greenLed() {
  ledState = !ledState;
  digitalWrite(GLEDPIN, ledState);
}
void blueLed() {
  ledState = !ledState;
  digitalWrite(BLEDPIN, ledState);

  taskEncenderLedB.SetIntervalMillis(ledState ? 500 : 900);
}
void redLed() {
  ledState = !ledState;
  digitalWrite(RLEDPIN, ledState);

  taskEncenderLedB.SetIntervalMillis(ledState ? 100 : 300);
}

//Transiciones
void DoneT5(){
  t5 = true;
}
void DoneT3(){
  t3 = true;
}