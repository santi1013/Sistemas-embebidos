#ifndef FSM_H
#define FSM_H

#include "StateMachineLib.h"
#include "Sensors.h"
#include "Tasks.h"

enum State {
  INICIO = 0,
  MONTEM = 1,
  MONHUM = 2,
  MONLUZ = 3,
  ALERTA = 4,
  ALARMA = 5
};

extern StateMachine machine;

/**
 * @brief Configura la máquina de estados del sistema.
 *
 * Registra las transiciones entre estados y asigna los callbacks
 * que se ejecutan al entrar y salir de cada estado.
 */
void setupMachine();

#endif