/*! \file ACIA_sysdep.h
    \brief Data structure to simulate an Asynchronous Communicating
    Interface Adapter.

    The system dependent ACIA provides emissions and receptions of
    char via a simulated serial link. Il will be implemented by using
    sockets through the net. An emission and a reception can be
    parallelized (full duplex operation).
    All the accesses to the sockets are already defined in the module
    sysdep.h, so they will just have to be renamed.

    DO NOT CHANGE -- part of the machine emulation

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

#ifndef _ACIA_SIM
#define _ACIA_SIM

#include "machine/ACIA.h"
#include "machine/interrupt.h"

// Forward declaration
class ACIA;

/*! \brief This class is used to simulate an Asynchronous Communicating
    Interface Adapter on top of Unix sockets.

    The system dependent ACIA provides emissions and receptions of
    bytes using sockets. An emission and a reception can be done in
    parallel (full duplex).
 */
class ACIA_sysdep {
public:
  /** Initializes a system dependent part of the ACIA.
   * \param interface: the non-system dependent part of the Acia simulation
   * (ACIA) \param machine: the MIPS machine
   */
  ACIA_sysdep(ACIA *interface, Machine *m);

  /** Deallocates it and close the socket. */
  ~ACIA_sysdep();

  /** Check if there is an incoming char.
   * Schedule the interrupt to execute itself again in a while.
   * Check if a char had came through the socket. If there is one,
   * input register's value and state are modified and
   * in Interrupt mode, execute the reception handler.
   * The data reception register of the ACIA object is overwritten
   * in all the cases.
   */
  void InterruptRec();

  /**  Send a char through the socket and drain the output register.  In
   * Interrupt mode, execute the emission handler.
   */
  void InterruptEm();

  /** Schedules an interrupt to simulate
   * the output register dumping.
   */
  void SendChar();

  /** Simulate the input register draining because it must be clear just after
   * a read operation.
   */
  void Drain();

private:
  ACIA *interface;     //!< ACIA
  int sock;            //!< UNIX socket number for incoming/outgoing packets.
  char sockName[32];   //!< File name corresponding to UNIX socket.
};

#endif   // _ACIA_SIM
