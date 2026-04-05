#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat!");
}

void setup() {
  Serial.begin(115200);

  // Inicializar I2C con los pines de tu ESP32-S3
  Wire.begin(15, 16);

  Serial.println("Inicializando MAX30100...");

  // Inicializar MAX30100
  if (!pox.begin()) {
    Serial.println("MAX30100 FAILED");
    while (true);
  } else {
    Serial.println("MAX30100 SUCCESS");
  }

  // Corriente del LED IR
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Callback para detectar latido
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    Serial.print("Heart rate: ");
    Serial.print(hr);
    Serial.print(" bpm / SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");

    tsLastReport = millis();
  }
}