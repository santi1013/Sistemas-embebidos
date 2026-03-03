#ifndef FSM_H
#define FSM_H

#include "StateMachineLib.h"
#include "Sensors.h"
#include "Tasks.h"

enum State {
  INICIO = 0,
  TEMPERATURA = 1,
  HUMEDAD = 2,
  LUZ = 3,
  ALERTA =4,
  ALARMA =5
};

extern StateMachine machine;

void setupMachine();

#endif