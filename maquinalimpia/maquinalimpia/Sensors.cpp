#include "Sensors.h"
#include "Tasks.h"
#include <AverageValue.h>   // <-- AverageValue (yesbotics)
#include <DHT.h>

#define NUMU 4

AverageValue<float> promedioLuz(NUMU);
AverageValue<float> promedioTem(NUMU);

DHT dht(DHTPIN, DHTTYPE);

static float lecturaLuz = 0.0f;
static float luzPromedio = 0.0f;
int muestraFlama = 0;

static float lecturaTem = 0.0f;
static float temPromedio = 0.0f;

static float lecturaHum = 0.0f;

// Contadores porque AverageValue NO tiene getCount() ni clear()
static uint8_t countLuz = 0;
static uint8_t countTem = 0;

void greenLed(void) {
  ledState = !ledState;
  digitalWrite(GLEDPIN, ledState);
}

void blueLed(void) {
  ledState = !ledState;
  digitalWrite(BLEDPIN, ledState);

  taskEncenderLedB.SetIntervalMillis(ledState ? 500 : 900);
}

void redLed(void) {
  ledState = !ledState;
  digitalWrite(RLEDPIN, ledState);

  taskEncenderLedR.SetIntervalMillis(ledState ? 100 : 300);
}

// Transiciones
void DoneT5(void){ t5 = true; }
void DoneT3(void){ t3 = true; }
void DoneT2(void){ t2 = true; }
void DoneT4(void){ t4 = true; }

void leerFlama(void) {
  muestraFlama = digitalRead(FLAMPIN);

  if (muestraFlama == HIGH) {
    Serial.println("LLAMA DETECTADA");
    eventoActual = FLAMA;
  } else {
    Serial.println("No hay llama");
  }
}

void leerLuz(void) {
  lecturaLuz = analogRead(LDRPIN);
  promedioLuz.push(lecturaLuz);
  if (countLuz < NUMU) countLuz++;
}

void enviarLuz(void) {
  if (countLuz >= NUMU) {
    luzPromedio = promedioLuz.average();   // <-- average() en AverageValue
    Serial.print("Luz promedio: ");
    Serial.println(luzPromedio);

    luz = (luzPromedio < 60.0f);

    countLuz = 0; // reinicio “ventana” (no existe clear())
  }
}

void leerTem(void) {
  float t = dht.readTemperature();
  if (!isnan(t)) {
    lecturaTem = t;
    promedioTem.push(lecturaTem);
    if (countTem < NUMU) countTem++;
  }
}

void enviarTem(void) {
  Serial.print("Temperatura: ");
  Serial.println(lecturaTem);

  // Si quieres usar el promedio (temPromedio), actívalo:
  if (countTem >= NUMU) {
    temPromedio = promedioTem.average();
    Serial.print("Temp promedio: ");
    Serial.println(temPromedio);
    countTem = 0;
  }

  if (lecturaTem > INTERVALO_TEMPERATURA_UNO) tem1 = true;
  if (lecturaTem > INTERVALO_TEMPERATURA_DOS) tem2 = true;
}

void leerHum(void) {
  float h = dht.readHumidity();
  if (!isnan(h)) {
    lecturaHum = h;
  }
}

void enviarHum(void) {
  Serial.print("Humedad: ");
  Serial.println(lecturaHum);
  if (lecturaHum > INTERVALO_HUMEDAD) hum1 = true;   // para entrar a ALERTA
  if (lecturaHum > INTERVALO_HUMEDAD) hum2 = true;   // si luego lo necesitas (similar a tem2)
}

void leerBoton(void) {
  static bool lastStable = HIGH;
  static bool lastRead   = HIGH;
  static unsigned long tDeb = 0;

  bool r = digitalRead(BTNPIN);

  if (r != lastRead) {          // cambio detectado
    lastRead = r;
    tDeb = millis();
  }

  if (millis() - tDeb > 30) {   // debounce 30ms
    if (r != lastStable) {
      lastStable = r;

      // INPUT_PULLUP: presionado = LOW
      if (lastStable == LOW) {
        eventoActual = BOTON;             // evento de pulsación (1 vez)
      }
    }
  }
} 
