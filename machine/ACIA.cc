//-------------------------------------------------------------------------
/*! \file ACIA.cc
// \brief Routines implementing an ACIA device

// Provide reading methodes on ACIA's state registers and on
// input/output data register. Two methods provide the reception/emision
// of a character from/to a remote machine. Methods are also provided to
// set/get the ACIA working mode (Busy Waiting or Interrupt).
//
//  DO NOT CHANGE -- part of the machine emulation
//
 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------
*/
//-------------------------------------------------------------------------

#include "ACIA.h"
#include "ACIA_sysdep.h"
#include "kernel/system.h"
#include "utility/stats.h"

//-------------------------------------------------------------------------
/**
   Initialize the ACIA device.
*/
//-------------------------------------------------------------------------
ACIA::ACIA(Machine *m) {
  // Setup the system dependent part of the simulator
  sysdep = new ACIA_sysdep(this, m);

  // Initialize data registers to '0' (empty char).
  inputRegister = 0;
  outputRegister = 0;

  // Initialize state registers to empty.
  inputStateRegister = EMPTY;
  outputStateRegister = EMPTY;

  // Initiliaze working mode to BUSY_WAITING mode.
  mode = BUSY_WAITING;
};

//-------------------------------------------------------------------------
// ACIA::~ACIA
/*! De-allocate the ACIA data structures.
 */
//-------------------------------------------------------------------------
ACIA::~ACIA() { delete sysdep; };

// Methodes

//-------------------------------------------------------------------------
/**  Change the working mode for the ACIA. The parameter is a
    bitwise OR of flags defining its working mode (BUSY_WAITING,
    REC_INTERRUPT, EM_INTERRUPT).  It must be used to enable/disable
    reception and emission interrupts in the Interrupt mode.

    \param mod: the new mode (a bitwise OR between flags REC_INTERRUPT,
   INTERRUPT and BUSY_WAITING).
*/
//-------------------------------------------------------------------------
void
ACIA::SetWorkingMode(int mod) {
  mode = mod;
};

//-------------------------------------------------------------------------
/** Get the current working mode for the ACIA (BUSY_WAITING,
    REC_INTERRUPT, EM_INTERRUPT).
*/
//-------------------------------------------------------------------------
int
ACIA::GetWorkingMode(void) {
  return mode;
};

//-------------------------------------------------------------------------
/** Get the state of the input register (used in the BUSY_WAITING
    mode).
    \return status of input register (EMPTY/FULL)
 */
//-------------------------------------------------------------------------
RegStatus
ACIA::GetInputStateReg() {
  // Simulate a clock tick in order to make Nachos able to provide interrupts.
  gMachine->interrupt->OneTick(SYSTEM_TICK);
  return inputStateRegister;
};

//-------------------------------------------------------------------------
/** Get the state of the output register (used in the BUSY_WAITING
    mode).
    \return status of output register (EMPTY/FULL)
*/
//-------------------------------------------------------------------------
RegStatus
ACIA::GetOutputStateReg() {
  // Simulate a clock tick in order to make nachos able to provide
  // interruptions.
  gMachine->interrupt->OneTick(SYSTEM_TICK);
  return outputStateRegister;
};

//-------------------------------------------------------------------------
/** Get the input data register value. This method does not include
  * any synchronization. When calling this method one must be sure
  * that there is a character available
  \return Byte received
 */
//-------------------------------------------------------------------------
char
ACIA::GetChar() {
  char temp;
  temp = inputRegister;
  sysdep->Drain();
  return temp;
};

// Emission and reception fonctions for the ACIA

//-------------------------------------------------------------------------
// ACIA::PutChar
/** Write a character into the ACIA output register. This method does
 * not include any synchronization. When calling this method one must
 * be sure that the ACIA is ready to send a character.
 \param c: The character to be sent.
*/
//-------------------------------------------------------------------------
void
ACIA::PutChar(char c) {
  // Fill the output register.
  outputRegister = c;
  // Cause the char forwading.
  sysdep->SendChar();
};
