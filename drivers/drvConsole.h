/*! \file drvConsole.h
    \brief Data structures to export a synchronous interface to the console
           device.

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

#ifndef SYNCHCONS_H
#define SYNCHCONS_H

class DriverConsole;

#include "kernel/synch.h"
#include "kernel/system.h"
#include "machine/console.h"
#include "utility/utility.h"

/*! \brief Defines a "synchronous" console abstraction.
//
// As with other I/O devices, the console is an asynchronous device.
// This class provides methods in order to preserve mutual exclusion
// on the console device. Two operations can be called : the first one
// writes a string to the console and the second one reads a string from
// the console. They return only when the read or write operation is
// completed.
*/
class DriverConsole {
public:
  DriverConsole();    // Constructor. Initialize the console driver
  ~DriverConsole();   // Destructor. Data de-allocation
  void PutString(char *buffer, int nbcar);
  // Write a buffer on the console
  void GetString(char *buffer, int nbcar);
  // Read characters from the console

  void GetAChar();   // Send a char to the console device
  void PutAChar();   // Receive e char from the console

private:
  Lock *mutexget;         //!< Lock on read operations
  Lock *mutexput;         //!< Lock on write operations
  Semaphore *get, *put;   //!< Semaphores to wait for interrupts
};

void ConsoleGet();
void ConsolePut();

#endif   // SYNCHCONS_H
