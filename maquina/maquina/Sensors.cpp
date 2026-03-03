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
  taskBlink2.SetIntervalMillis(ledState ? 300 : 500);
}
void rgbRed() {
  digitalWrite(PIN_R, HIGH);
  digitalWrite(PIN_G, LOW);
  digitalWrite(PIN_B, LOW)
}

void rgbGreen() {
  digitalWrite(PIN_R, LOW);
  digitalWrite(PIN_G, HIGH);
  digitalWrite(PIN_B, LOW);
}

void rgbBlue() {
  digitalWrite(PIN_R, LOW);
  digitalWrite(PIN_G, LOW);
  digitalWrite(PIN_B, HIGH);

}
void readButton() {
  static bool last = HIGH;              // Sin presionar = HIGH (INPUT_PULLUP)
  bool now = digitalRead(BTN_PIN); // Flanco de bajada: HIGH -> LOW (pulsación)
  if (last == HIGH && now == LOW) {
    boton = true;
  }

  last = now;

}
