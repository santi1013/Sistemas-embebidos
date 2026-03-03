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
void rgbRed() {
  digitalWrite(PIN_R, HIGH);
  digitalWrite(PIN_G, LOW);
  digitalWrite(PIN_B, LOW)
}

void rgbGreen() {
  isOn = !isOn;
  digitalWrite(PIN_R, LOW);
  digitalWrite(PIN_G, isOn ? HIGH : LOW);
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
//funciones del sensor de luz
void leerLuz(void){

  float lectura = analogRead(ldrPin);

  promedio.push(lectura);   // Agregamos lectura al buffer

  if(promedio.isFull()){    // Cuando ya tiene las 4 muestras
    taskPromediar.Start();
  }
}


void enviarLuz(void){
  luzPromedio = promedio.mean();  // Calcula promedio
  taskEnviar.Start();

  Serial.println("Nivel de luz (ADC): ");
  Serial.println(luzPromedio);

  Serial.println("Numero de muestras: ");
  Serial.println(numu);

  Serial.println("Tiempo (ms): ");
  Serial.println(millis());

  Serial.println("-------------------------");

  promedio.clear();   // Limpia el buffer para nuevas muestras
}
//del sensor de flama
void leerFlama(void) {
  int estado = digitalRead(flamePin);
  
  if (estado == LOW) {
    flameState = true;
    Serial.println("LLAMA DETECTADA");
  } else {
    flameState = false;
  }
}



