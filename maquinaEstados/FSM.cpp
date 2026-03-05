#include "FSM.h"

StateMachine machine(6, 11);

// --- Callbacks de estados ---

/**
 * @brief Callback de entrada al estado INICIO.
 *
 * Activa el parpadeo del LED verde y habilita la tarea de lectura del botón.
 * Este estado representa la condición inicial del sistema.
 */
void onInicio() {
  ledState = false;
  taskEncenderLedG.Start();
  taskBoton.Start();
  Serial.println("Entrada a INICIO Start() tareas correspondientes");
}

/**
 * @brief Callback de salida del estado INICIO.
 *
 * Detiene las tareas asociadas al estado inicial, apaga el LED verde
 * y limpia el evento actual para evitar transiciones no deseadas.
 */
void offInicio() {
  taskEncenderLedG.Stop();
  ledState = false;
  digitalWrite(GLEDPIN, LOW);
  taskBoton.Stop();
  eventoActual = DEFECTO;
  Serial.println("Salida de INICIO Stop() tareas correspondientes");
}

/**
 * @brief Callback de entrada al estado MONTEM.
 *
 * Inicia las tareas necesarias para el monitoreo de temperatura,
 * incluyendo temporizadores, lectura del sensor DHT y detección
 * de flama.
 */
void onMontem() {
  t4 = false;
  t2 = false;
  tem1 = false;
  taskT4.Start();
  taskT2.Start();
  taskFlama.Start();
  taskLeerTem.Start();
  taskEnviar.Start();
  Serial.println("Entrada a MONTEM Start() tareas correspondientes");
}

/**
 * @brief Callback de salida del estado MONTEM.
 *
 * Detiene las tareas relacionadas con el monitoreo de temperatura
 * y restablece variables de control para evitar condiciones residuales.
 */
void offMontem() {
  t4 = false;
  t2 = false;
  taskT4.Stop();
  taskT2.Stop();
  taskLeerTem.Stop();
  taskEnviar.Stop();
  taskFlama.Stop();
  eventoActual = DEFECTO;
  Serial.println("Salida de MONTEM Stop() tareas correspondientes");
}

/**
 * @brief Callback de entrada al estado MONHUM.
 *
 * Activa el monitoreo del sensor de humedad y el temporizador
 * asociado para controlar el tiempo de permanencia en este estado.
 */
void onMonhum() {
  t3 = false;
  taskT3.Start();
  taskLeerHum.Start();
  taskEnviar.Start();
  Serial.println("Entrada a MONHUM: Monitoreo de Humedad iniciado");
}

/**
 * @brief Callback de salida del estado MONHUM.
 *
 * Detiene las tareas de monitoreo de humedad y reinicia
 * las variables asociadas a este sensor.
 */
void offMonhum() {
  taskT3.Stop();
  taskLeerHum.Stop();
  taskEnviar.Stop();
  t3 = false;
  tem1 = false;
  hum1 = false;
  hum2 = false;
  Serial.println("Salida de MONHUM: Deteniendo tareas de humedad");
}

/**
 * @brief Callback de entrada al estado MONLUZ.
 *
 * Inicia el monitoreo del sensor de luz y activa el temporizador
 * encargado de controlar el tiempo en este estado.
 */
void onMonluz() {
  t5 = false;
  taskT5.Start();
  taskLeerLuz.Start();
  taskEnviar.Start();
  Serial.println("Entrada a MONLUZ: Monitoreo de Fotocelda iniciado");
}

/**
 * @brief Callback de salida del estado MONLUZ.
 *
 * Detiene las tareas relacionadas con el sensor de luz y
 * reinicia las variables utilizadas para el monitoreo.
 */
void offMonluz() {
  taskT5.Stop();
  t5 = false;
  taskLeerLuz.Stop();
  taskEnviar.Stop();
  luz = false;
  Serial.println("Salida de MONLUZ: Deteniendo monitoreo de luz");
}

/**
 * @brief Callback de entrada al estado ALERTA.
 *
 * Incrementa el contador de alertas, activa el parpadeo del LED azul
 * y continúa monitoreando la temperatura para determinar si se debe
 * escalar al estado de alarma.
 */
void onAlerta() {
  t3 = false;
  ledState = false;
  conteoAlerta = conteoAlerta + 1;
  taskT3.Start();
  taskEncenderLedB.Start();
  taskLeerTem.Start();
  taskEnviar.Start();
  Serial.println("Entrada a ALERTA: ¡Condición anómala detectada!");
}

/**
 * @brief Callback de salida del estado ALERTA.
 *
 * Detiene el parpadeo del LED azul y reinicia las variables
 * utilizadas durante la condición de alerta.
 */
void offAlerta() {
  ledState = false;
  taskEncenderLedB.Stop();
  taskT3.Stop();
  taskLeerTem.Stop();
  taskEnviar.Stop();
  t3 = false;
  tem2 = false;
  Serial.println("Salida de ALERTA: Limpiando estado de alerta");
}

/**
 * @brief Callback de entrada al estado ALARMA.
 *
 * Activa el parpadeo del LED rojo para indicar una condición crítica
 * y habilita nuevamente el botón para permitir reiniciar el sistema.
 */
void onAlarma() {
  ledState = false;
  taskEncenderLedR.Start();
  digitalWrite(BLEDPIN, LOW);
  taskBoton.Start();
  Serial.println("Entrada a ALARMA: ¡PELIGRO DETECTADO!");
}

/**
 * @brief Callback de salida del estado ALARMA.
 *
 * Detiene los actuadores de emergencia, reinicia el contador de alertas
 * y limpia el evento actual antes de regresar al estado inicial.
 */
void offAlarma() {
  ledState = false;
  conteoAlerta = 0;
  taskEncenderLedR.Stop();
  taskBoton.Start();
  digitalWrite(RLEDPIN, LOW);
  eventoActual = DEFECTO;
  Serial.println("Salida de ALARMA: Desactivando actuadores de emergencia");
}

/**
 * @brief Configura la máquina de estados del sistema.
 *
 * Define todas las transiciones entre estados y registra los
 * callbacks que se ejecutan al entrar o salir de cada estado.
 */
void setupMachine() {
  machine.AddTransition(INICIO, MONTEM, []() { return eventoActual == BOTON; });

  machine.AddTransition(MONTEM, MONHUM, []() { return t4; });
  machine.AddTransition(MONHUM, MONTEM, []() { return t3; });
  machine.AddTransition(MONTEM, MONLUZ, []() { return eventoActual == FLAMA && t2; });
  machine.AddTransition(MONLUZ, MONTEM, []() { return t5; });
  machine.AddTransition(MONTEM, ALERTA, []() { return eventoActual == TEMPERATURA; });

  machine.AddTransition(ALERTA, MONTEM, []() { return t3; });
  machine.AddTransition(MONLUZ, ALERTA, []() { return eventoActual == LUZ; });
  machine.AddTransition(MONHUM, ALERTA, []() { return eventoActual == HUMEDAD; });
  machine.AddTransition(ALERTA, ALARMA, []() { return conteoAlerta >= 3 && eventoActual == TEMPERATURA; });

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