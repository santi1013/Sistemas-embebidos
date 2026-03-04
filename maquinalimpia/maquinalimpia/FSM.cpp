#include "FSM.h"

StateMachine machine(6, 11);

// --- Callbacks de estados ---
void onInicio() {
  ledState = false;
  taskEncenderLedG.Start();
  taskBoton.Start();
  Serial.println("Entrada a INICIO Start() tareas correspondientes");
}
void offInicio() {
  taskEncenderLedG.Stop();
  ledState = false;
  digitalWrite(GLEDPIN, LOW);
  taskBoton.Stop();
  eventoActual = DEFECTO;
  Serial.println("Salida de INICIO Stop() tareas correspondientes");
}

void onMontem() {
  t4 = false;
  t2 = false;
  tem1 = false;
  taskT4.Start();
  taskT2.Start();
  taskFlama.Start();
  taskLeerTem.Start();
  taskEnviarTem.Start();
  Serial.println("Entrada a MONTEM Start() tareas correspondientes");
}
void offMontem() {
  t4 = false;
  t2 = false;
  taskT4.Stop();
  taskT2.Stop();
  taskLeerTem.Stop();
  taskEnviarTem.Stop();
  taskFlama.Stop();
  eventoActual = DEFECTO;
  Serial.println("Salida de MONTEM Stop() tareas correspondientes");
}

void onMonhum() {
  t3 = false;
  taskT3.Start();
  taskLeerHum.Start();
  taskEnviarHum.Start();
  Serial.println("Entrada a MONHUM: Monitoreo de Humedad iniciado");
}
void offMonhum() {
  taskT3.Stop();
  taskLeerHum.Stop();
  taskEnviarHum.Stop();
  t3 = false;
  tem1 = false;
  hum1 = false;
  hum2 = false;
  Serial.println("Salida de MONHUM: Deteniendo tareas de humedad");
}

void onMonluz() {
  t5 = false;
  taskT5.Start();
  taskLeerLuz.Start();
  taskEnviarLuz.Start();
  Serial.println("Entrada a MONLUZ: Monitoreo de Fotocelda iniciado");
}
void offMonluz() {
  taskT5.Stop();
  t5 = false;
  taskLeerLuz.Stop();
  taskEnviarLuz.Stop();
  luz = false;
  Serial.println("Salida de MONLUZ: Deteniendo monitoreo de luz");
}

void onAlerta() {
  t3 = false;
  ledState = false;
  conteoAlerta = conteoAlerta + 1;
  taskT3.Start();
  taskEncenderLedB.Start();
  taskLeerTem.Start();
  taskEnviarTem.Start();
  Serial.println("Entrada a ALERTA: ¡Condición anómala detectada!");
}
void offAlerta() {
  ledState = false;
  taskEncenderLedB.Stop();
  taskT3.Stop();
  taskLeerTem.Stop();
  taskEnviarTem.Stop();
  t3 = false;
  tem2 = false;
  Serial.println("Salida de ALERTA: Limpiando estado de alerta");
}

void onAlarma() {
  ledState = false;
  taskEncenderLedR.Start();
  digitalWrite(BLEDPIN, LOW);
  taskBoton.Start();
  Serial.println("Entrada a ALARMA: ¡PELIGRO DETECTADO!");
}
void offAlarma() {
  ledState = false;
  conteoAlerta = 0;
  taskEncenderLedR.Stop();
  taskBoton.Start();
  digitalWrite(RLEDPIN, LOW);
  eventoActual = DEFECTO;
  Serial.println("Salida de ALARMA: Desactivando actuadores de emergencia");
}
// --- Setup de la máquina ---
void setupMachine() {
  machine.AddTransition(INICIO, MONTEM, []() { return eventoActual == BOTON; });

  machine.AddTransition(MONTEM, MONHUM, []() { return t4; });
  machine.AddTransition(MONHUM, MONTEM, []() { return t3; });
  machine.AddTransition(MONTEM, MONLUZ, []() { return eventoActual == FLAMA && t2; });
  machine.AddTransition(MONLUZ, MONTEM, []() { return t5; });
  machine.AddTransition(MONTEM, ALERTA, []() { return tem1; });

  machine.AddTransition(ALERTA, MONTEM, []() { return t3; });
  machine.AddTransition(MONLUZ, ALERTA, []() { return luz; });
  machine.AddTransition(MONHUM, ALERTA, []() { return hum1; });
  machine.AddTransition(ALERTA, ALARMA, []() { return conteoAlerta >= 3 && tem2; });

  machine.AddTransition(ALARMA, INICIO, []() { return eventoActual == BOTON; });

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