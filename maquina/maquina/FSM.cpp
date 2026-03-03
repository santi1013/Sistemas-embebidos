#include "FSM.h"
#include "Sensors.h"
#include "Tasks.h"
StateMachine machine(6, 2);

// --- Callbacks de estados ---
void onEstado1() {
  Serial.println("Estado 1: 200/200ms");
  ledState = false;
  taskEntrada.Start();
  taskBlink1.Start();
}
void Inicio() {
  serial.println("Iniciando Maquina")
  taskGreen.Start();
  taskBoton.Start();
}
void Temperatura() {
  dht.begin();
  taskLeer.Start();
  taskPromediar.Start();
  taskEnviar.Start();
}

void offEstado1() {
  taskBlink1.Stop();
  digitalWrite(GLED, LOW);
  entrada = ' ';
}

void onEstado2() {
  Serial.println("Estado 2: 300/500ms");
  ledState = false;
  taskBlink2.SetIntervalMillis(300);
  taskEntrada.Start();
  taskBlink2.Start();
}
void offEstado2() {
  taskBlink2.Stop();
  digitalWrite(GLED, LOW);
  entrada = ' ';
}

// --- Setup de la máquina ---
void setupMachine() {
  machine.AddTransition(IINICIO, TEMPERATURA, []() { return boton == true; });
  machine.AddTransition(ESTADO1, ESTADO2, []() { return entrada == '2'; });
  machine.AddTransition(ESTADO2, ESTADO1, []() { return entrada == '1'; });

  machine.SetOnEntering(INICIO, []() { Inicio(); });
  machine.SetOnEntering(TEMPERATURA, []() { Temperatura(); });

  machine.SetOnLeaving(ESTADO1,  []() { offEstado1(); });
  machine.SetOnLeaving(ESTADO2,  []() { offEstado2(); });
}