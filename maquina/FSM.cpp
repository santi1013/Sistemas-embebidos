#include "FSM.h"

StateMachine machine(6, 11);

// --- Callbacks de estados ---
void onInicio() {
  taskEntrada.Start();
  taskGreen.Start();
  taskBoton.Start();
  Serial.println("Entrada a INICIO Start() tareas correspondientes");
}
void offInicio() {
  taskEntrada.Stop();
  Serial.println("Salida de INICIO Stop() tareas correspondientes");
}

void onMontem() {
  taskEntrada.Start();
  dht.begin();
  taskLeer.Start();
  taskPromediar.Start();
  taskEnviar.Start();
  Serial.println("Entrada a MONTEM Start() tareas correspondientes");
}
void offMontem() {
  taskEntrada.Stop();
  Serial.println("Salida de MONTEM Stop() tareas correspondientes");
}

void onMonhum() {
  taskEntrada.Start();
  Serial.println("Entrada a MONHUM: Monitoreo de Humedad iniciado");
}
void offMonhum() {
  taskEntrada.Stop();
  Serial.println("Salida de MONHUM: Deteniendo tareas de humedad");
}

void onMonluz() {
  taskEntrada.Start();
  taskLeer.Start();
  taskEnviar.Start();
  Serial.println("Entrada a MONLUZ: Monitoreo de Fotocelda iniciado");
}
void offMonluz() {
  taskEntrada.Stop();
  taskLeer.Stop();
  Serial.println("Salida de MONLUZ: Deteniendo monitoreo de luz");
}

void onAlerta() {
  taskEntrada.Start();
  Serial.println("Entrada a ALERTA: ¡Condición anómala detectada!");
}
void offAlerta() {
  taskEntrada.Stop();
  Serial.println("Salida de ALERTA: Limpiando estado de alerta");
}

void onAlarma() {
  taskEntrada.Start();
  Serial.println("Entrada a ALARMA: ¡PELIGRO DETECTADO!");
}
void offAlarma() {
  taskEntrada.Stop();
  Serial.println("Salida de ALARMA: Desactivando actuadores de emergencia");
}

//del sensor luz


// --- Setup de la máquina ---
void setupMachine() {
  machine.AddTransition(INICIO, MONTEM, []() { return entrada == '1'; });

  machine.AddTransition(MONTEM, MONHUM, []() { return entrada == '2'; });
  machine.AddTransition(MONHUM, MONTEM, []() { return entrada == '3'; });
  machine.AddTransition(MONTEM, MONLUZ, []() { return flameState == true; });
  machine.AddTransition(MONLUZ, MONTEM, []() { return entrada == '5'; });
  machine.AddTransition(MONTEM, ALERTA, []() { return entrada == '6'; });

  machine.AddTransition(ALERTA, MONTEM, []() { return entrada == '7'; });
  machine.AddTransition(MONLUZ, ALERTA, []() { return entrada == '8'; });
  machine.AddTransition(MONHUM, ALERTA, []() { return entrada == '9'; });
  machine.AddTransition(ALERTA, ALARMA, []() { return entrada == 'x'; });

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







