#include "Sensors.h"
#include "Tasks.h"
#include <AverageValue.h>   // <-- AverageValue (yesbotics)
#include <DHT.h>

#define NUMU 4

AverageValue<float> promedioLuz(NUMU);
AverageValue<float> promedioTem(NUMU);
AverageValue<float> promedioHum(NUMU);

DHT dht(DHTPIN, DHTTYPE);

static float lecturaLuz = 0.0f;
static float luzPromedio = 0.0f;
int muestraFlama = 0;

static float lecturaTem = 0.0f;
static float temPromedio = 0.0f;

static float lecturaHum = 0.0f;
static float humPromedio = 0.0f;

// Contadores porque AverageValue NO tiene getCount() ni clear()
static uint8_t countLuz = 0;
static uint8_t countHum = 0;
static uint8_t countTem = 0;

/**
 * @brief Alterna el estado del LED verde.
 *
 * Cambia el valor de la variable compartida ledState e invierte el estado
 * del pin asociado al LED verde para generar parpadeo.
 */
void greenLed(void) {
  ledState = !ledState;
  digitalWrite(GLEDPIN, ledState);
}

/**
 * @brief Alterna el estado del LED azul.
 *
 * Cambia el estado del LED azul y ajusta dinámicamente el intervalo
 * de la tarea asociada para lograr un parpadeo asimétrico (500 ms encendido,
 * 900 ms apagado).
 */
void blueLed(void) {
  ledState = !ledState;
  digitalWrite(BLEDPIN, ledState);

  taskEncenderLedB.SetIntervalMillis(ledState ? 500 : 900);
}

/**
 * @brief Alterna el estado del LED rojo.
 *
 * Cambia el estado del LED rojo y ajusta el intervalo de la tarea asociada
 * para definir el patrón de parpadeo del estado de alarma.
 */
void redLed(void) {
  ledState = !ledState;
  digitalWrite(RLEDPIN, ledState);

  taskEncenderLedR.SetIntervalMillis(ledState ? 100 : 300);
}

// Transiciones

/**
 * @brief Señaliza la finalización del temporizador T5.
 *
 * Activa la bandera t5 para permitir transiciones de la máquina de estados
 * basadas en tiempo.
 */
void DoneT5(void){ t5 = true; }

/**
 * @brief Señaliza la finalización del temporizador T3.
 *
 * Activa la bandera t3 para permitir transiciones de la máquina de estados
 * basadas en tiempo.
 */
void DoneT3(void){ t3 = true; }

/**
 * @brief Señaliza la finalización del temporizador T2.
 *
 * Activa la bandera t2 para permitir transiciones de la máquina de estados
 * basadas en tiempo.
 */
void DoneT2(void){ t2 = true; }

/**
 * @brief Señaliza la finalización del temporizador T4.
 *
 * Activa la bandera t4 para permitir transiciones de la máquina de estados
 * basadas en tiempo.
 */
void DoneT4(void){ t4 = true; }

/**
 * @brief Lee el sensor de flama y genera el evento correspondiente.
 *
 * Realiza una lectura digital del sensor conectado al pin FLAMPIN. Si se
 * detecta flama (HIGH), se genera el evento FLAMA para la máquina de estados.
 */
void leerFlama(void) {
  muestraFlama = digitalRead(FLAMPIN);

  if (muestraFlama == HIGH) {
    Serial.println("LLAMA DETECTADA");
    eventoActual = FLAMA;
  } else {
    Serial.println("No hay llama");
  }
}

/**
 * @brief Toma una muestra del sensor de luz.
 *
 * Lee el ADC del pin LDRPIN, almacena la muestra en el buffer de promedio
 * y actualiza el contador de muestras para el cálculo posterior del promedio.
 */
void leerLuz(void) {
  lecturaLuz = analogRead(LDRPIN);
  promedioLuz.push(lecturaLuz);
  if (countLuz < NUMU) countLuz++;
}

/**
 * @brief Toma una muestra de temperatura desde el DHT.
 *
 * Lee la temperatura del sensor DHT. Si la lectura es válida, almacena la
 * muestra en el buffer de promedio y actualiza el contador de muestras.
 */
void leerTem(void) {
  float t = dht.readTemperature();
  if (!isnan(t)) {
    lecturaTem = t;
    promedioTem.push(lecturaTem);
    if (countTem < NUMU) countTem++;
  }
}

/**
 * @brief Toma una muestra de humedad desde el DHT.
 *
 * Lee la humedad relativa del sensor DHT. Si la lectura es válida, almacena la
 * muestra en el buffer de promedio y actualiza el contador de muestras.
 */
void leerHum(void) {
  float h = dht.readHumidity();
  if (!isnan(h)) {
    lecturaHum = h;
    promedioHum.push(lecturaHum);
    if (countHum < NUMU) countHum++;
  }
}

/**
 * @brief Calcula promedios de sensores y determina el evento del sistema.
 *
 * Cuando se cuenta con suficientes muestras, calcula los promedios de
 * temperatura, humedad y luz. Luego imprime los valores por Serial y asigna
 * el eventoActual según los umbrales configurados (TEMPERATURA, HUMEDAD o LUZ).
 */
void enviarDatos(void) {

  // Calcular promedios solo si hay muestras
  if (countTem >= NUMU) {
    temPromedio = promedioTem.average();
    countTem = 0;
  }

   if (countHum >= NUMU) {
    humPromedio = promedioHum.average();
    countHum = 0;
  }

  if (countLuz >= NUMU) {
    luzPromedio = promedioLuz.average();
    countLuz = 0;
  }

  // Mostrar valores
  Serial.println("----- DATOS PROMEDIO -----");

  Serial.print("Temperatura promedio: ");
  Serial.println(temPromedio);

  Serial.print("Humedad: ");
  Serial.println(humPromedio);

  Serial.print("Luz promedio: ");
  Serial.println(luzPromedio);

  // Evento por defecto
  eventoActual = DEFECTO;

  // Generar eventos según condiciones
  if (temPromedio > 25.5f) {
    eventoActual = TEMPERATURA;
  }

  else if (humPromedio > 60.0f) {
    eventoActual = HUMEDAD;
  }

  else if (luzPromedio < 60.0f) {
    eventoActual = LUZ;
  }
}

/**
 * @brief Lee el botón con anti-rebote por tiempo y genera el evento BOTON.
 *
 * Realiza lectura del pin del botón (BTNPIN) y aplica un debounce de 30 ms.
 * Cuando detecta una pulsación estable (INPUT_PULLUP: presionado = LOW),
 * asigna el evento BOTON para ser consumido por la máquina de estados.
 */
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
