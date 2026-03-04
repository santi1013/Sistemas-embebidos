#include "FSM.h"

StateMachine machine(6, 11);

// --- Callbacks de estados ---
void onInicio() {
  ledState = false;
  taskEntrada.Start();
  taskEncenderLedG.Start();
  Serial.println("Entrada a INICIO Start() tareas correspondientes");
}
void offInicio() {
  taskEntrada.Stop();
  taskEncenderLedG.Stop();
  ledState = false;
  digitalWrite(GLEDPIN, LOW);
  Serial.println("Salida de INICIO Stop() tareas correspondientes");
}

void onMontem() {
  t5 = false;
  taskEntrada.Start();
  taskT5.Start();
  Serial.println("Entrada a MONTEM Start() tareas correspondientes");
}
void offMontem() {
  taskEntrada.Stop();
  taskT5.Stop();
  t5 = false;
  Serial.println("Salida de MONTEM Stop() tareas correspondientes");
}

void onMonhum() {
  t3 = false;
  taskEntrada.Start();
  taskT3.Start();
  Serial.println("Entrada a MONHUM: Monitoreo de Humedad iniciado");
}
void offMonhum() {
  taskEntrada.Stop();
  taskT3.Stop();
  t3 = false;
  Serial.println("Salida de MONHUM: Deteniendo tareas de humedad");
}

void onMonluz() {
  taskEntrada.Start();
  Serial.println("Entrada a MONLUZ: Monitoreo de Fotocelda iniciado");
}
void offMonluz() {
  taskEntrada.Stop();
  Serial.println("Salida de MONLUZ: Deteniendo monitoreo de luz");
}

void onAlerta() {
  ledState = false;
  conteoAlerta = conteoAlerta + 1;
  taskEntrada.Start();
  taskEncenderLedB.Start();
  Serial.println("Entrada a ALERTA: ¡Condición anómala detectada!");
}
void offAlerta() {
  ledState = false;
  taskEntrada.Stop();
  taskEncenderLedB.Stop();
  Serial.println("Salida de ALERTA: Limpiando estado de alerta");
}

void onAlarma() {
  ledState = false;
  taskEntrada.Start();
  taskEncenderLedR.Start();
  digitalWrite(BLEDPIN, LOW);
  Serial.println("Entrada a ALARMA: ¡PELIGRO DETECTADO!");
}
void offAlarma() {
  ledState = false;
  conteoAlerta = 0;
  taskEntrada.Stop();
  taskEncenderLedR.Stop();
  digitalWrite(RLEDPIN, LOW);
  Serial.println("Salida de ALARMA: Desactivando actuadores de emergencia");
}
// --- Setup de la máquina ---
void setupMachine() {
  machine.AddTransition(INICIO, MONTEM, []() { return entrada == '1'; });

  machine.AddTransition(MONTEM, MONHUM, []() { return t5 == true; });
  machine.AddTransition(MONHUM, MONTEM, []() { return t3 == true; });
  machine.AddTransition(MONTEM, MONLUZ, []() { return entrada == '4'; });
  machine.AddTransition(MONLUZ, MONTEM, []() { return entrada == '5'; });
  machine.AddTransition(MONTEM, ALERTA, []() { return entrada == '6'; });

  machine.AddTransition(ALERTA, MONTEM, []() { return entrada == '7'; });
  machine.AddTransition(MONLUZ, ALERTA, []() { return entrada == '8'; });
  machine.AddTransition(MONHUM, ALERTA, []() { return entrada == '9'; });
  machine.AddTransition(ALERTA, ALARMA, []() { return conteoAlerta == 3; });

  machine.AddTransition(ALARMA, INICIO, []() { return entrada == 'y'; });

  machine.SetOnEntering(INICIO, []() { onInicio(); });
  machine.SetOnEntering(MONTEM, []() { onMontem(); });
  machine.SetOnEntering(MONHUM, []() { onMonhum(); });
  machine.SetOnEntering(MONLUZ, []() { onMonluz(); });
  machine.SetOnEntering(ALERTA, []() { onAlerta(); });
  machine.SetOnEntering(ALARMA, []() { onAlarma(); });

  machine.SetOnLeaving(INICIO, []() { offInicio(); });
  machine.SetOnLeaving(MONTEM, []() { offMontem(); });
  machine.SetOnLeaving(MONHUM, []() { offMonhum(); });
  machine.SetOnLeaving(MONLUZ, []() { offMonluz(); });
  machine.SetOnLeaving(ALERTA, []() { offAlerta(); });
  machine.SetOnLeaving(ALARMA, []() { offAlarma(); });
}