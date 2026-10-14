/*! \file drvConsole.cc
//  \brief Routines to synchronously access the console
//
//      The console is an asynchronous device (requests return
//      immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the console providing a synchronous interface
//      (requests wait until the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the console can only
//	handle one read operation and one write operation at a time,
//      use two locks to enforce mutual exclusion.
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

#include "drivers/drvConsole.h"
#include "machine/interrupt.h"

//----------------------------------------------------------------------
// static void ConsoleGet
/*! 	Console write interrupt handler. Needs this to be a C routine,
//      because C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------
void
ConsoleGet() {
  gConsoleDriver->GetAChar();
}

//----------------------------------------------------------------------
// static void ConsolePut
/*! 	Console read interrupt handler.  Needs this to be a C routine,
//      because C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------
void
ConsolePut() {
  gConsoleDriver->PutAChar();
}

//-----------------------------------------------------------------
// DriverConsole::DriverConsole
/*!     Constructor.
//      Initialize the console driver (lock and semaphore creation)
*/
//-----------------------------------------------------------------
DriverConsole::DriverConsole() {
  get = new Semaphore((char *) "get", 0);
  put = new Semaphore((char *) "put", 0);
  mutexget = new Lock((char *) "mutex get");
  mutexput = new Lock((char *) "mutex put");
}

//-----------------------------------------------------------------
// DriverConsole::~DriverConsole
/*!     Destructor.
//      De-allocate data structures needed by the console driver
//      (semaphores, locks).
*/
//-----------------------------------------------------------------
DriverConsole::~DriverConsole() {
  delete mutexget;
  delete mutexput;
  delete get;
  delete put;
}

//-----------------------------------------------------------------
// DriverConsole::PutAChar
/*!     Operate a V on the "write" semaphore to signal a char is written.
//      The method is called by the interrupt handler ConsolePut.
*/
//-----------------------------------------------------------------
void
DriverConsole::PutAChar() {

  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);
  put->V();
  (void) gMachine->interrupt->SetStatus(oldLevel);
}

//-----------------------------------------------------------------
// DriverConsole::PutString
/*!     Send a string to the console device using a lock to insure
//      mutual exclusion. The method returns when all characters
//      has been sent.
//
//      \param buffer contains the data to send
//      \param nbcar is the number of chars to send
*/
//-----------------------------------------------------------------
void
DriverConsole::PutString(char *buffer, int nbcar) {

  mutexput->Acquire();

  for (int i = 0; i < nbcar; i++) {
    gCurrentThread->GetProcessOwner()->stat->incrNumCharWritten();
    gMachine->console->PutChar(buffer[i]);
    put->P();
  }

  mutexput->Release();
}

//-----------------------------------------------------------------
// DriverConsole::GetAChar
/*!     Operate a V on the "read" semaphore to signal a char is read.
//      The method is called by the interrupt handler ConsoleGet.
*/
//-----------------------------------------------------------------
void
DriverConsole::GetAChar() {

  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);
  get->V();
  (void) gMachine->interrupt->SetStatus(oldLevel);
}

//-----------------------------------------------------------------
// DriverConsole::GetString
/*!     Receive a string from the console device using a lock to
//      prevent from concurrent accesses. The method returns when
//      all characters has been received.
//
//      \param buffer is the structure to fill
/       \param size is the number max of char to be received
*/
//-----------------------------------------------------------------
void
DriverConsole::GetString(char *buffer, int nbcar) {

  char c = 0;
  int i;

  mutexget->Acquire();
  gMachine->console->EnableInterrupt();

  for (i = 0; ((i < nbcar) && (c != '\n')); i++) {
    gCurrentThread->GetProcessOwner()->stat->incrNumCharRead();
    get->P();
    c = gMachine->console->GetChar();
    buffer[i] = c;
  }
  buffer[i] = 0;

  gMachine->console->DisableInterrupt();
  mutexget->Release();
}
