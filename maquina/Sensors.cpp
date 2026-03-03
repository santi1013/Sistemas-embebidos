#include "Sensors.h"
#include "Tasks.h"

void leerEntrada() {
  if (Serial.available()) entrada = Serial.read();
}

void toggleLed1() {
  ledState = !ledState;
  digitalWrite(GLED, ledState);
}

void toggleLed2() {
  ledState = !ledState;
  digitalWrite(GLED, ledState);
}