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

void setupMachine();

#endif