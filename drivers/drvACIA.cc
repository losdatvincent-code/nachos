/* \file drvACIA.cc
   \brief Routines of the ACIA device driver
//
//      The ACIA is an asynchronous device (requests return
//      immediately, and an interrupt happens later on).
//      This is a layer on top of the ACIA.
//      Two working modes are to be implemented in assignment 2:
//      a Busy Waiting mode and an Interrupt mode. The Busy Waiting
//      mode implements a synchronous IO whereas IOs are asynchronous
//      IOs are implemented in the Interrupt mode (see the Nachos
//      roadmap for further details).
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

/* Includes */
#include "drivers/drvACIA.h"
#include "kernel/msgerror.h"
#include "kernel/synch.h"
#include "kernel/system.h"   // for the ACIA object
#include "machine/ACIA.h"

//-------------------------------------------------------------------------
// DriverACIA::DriverACIA()
/*! Constructor.
  Initialize the ACIA driver.
  In the ACIA Interrupt mode,
  initialize the reception index and semaphores and allow
  reception interrupts.
  In the ACIA Busy Waiting mode, simply inittialize the ACIA
  working mode and create the semaphore.
  */
//-------------------------------------------------------------------------

DriverACIA::DriverACIA() {
  printf("**** Warning: contructor of the ACIA driver not implemented yet\n");
  exit(ERROR);
}

//-------------------------------------------------------------------------
// DriverACIA::TtySend(char* buff)
/*! Routine to send a message through the ACIA (Busy Waiting or Interrupt mode)
 */
//-------------------------------------------------------------------------

int
DriverACIA::TtySend(char *buff) {
  printf(
      "**** Warning: method Tty_Send of the ACIA driver not implemented yet\n");
  exit(ERROR);
  return 0;
}

//-------------------------------------------------------------------------
// DriverACIA::TtyReceive(char* buff,int length)
/*! Routine to reveive a message through the ACIA
//  (Busy Waiting and Interrupt mode).
  */
//-------------------------------------------------------------------------

int
DriverACIA::TtyReceive(char *buff, int lg) {
  printf("**** Warning: method Tty_Receive of the ACIA driver not implemented "
         "yet\n");
  exit(ERROR);
  return 0;
}

//-------------------------------------------------------------------------
// DriverACIA::InterruptSend()
/*! Emission interrupt handler.
  Used in the ACIA Interrupt mode only.
  Detects when it's the end of the message (if so, releases the send_sema
  semaphore), else sends the next character according to index ind_send.
  */
//-------------------------------------------------------------------------

void
DriverACIA::InterruptSend() {
  printf("**** Warning: send interrupt handler not implemented yet\n");
  exit(ERROR);
}

//-------------------------------------------------------------------------
// DriverACIA::Interrupt_receive()
/*! Reception interrupt handler.
  Used in the ACIA Interrupt mode only. Reveices a character through the ACIA.
  Releases the receive_sema semaphore and disables reception
  interrupts when the last character of the message is received
  (character '\0').
  */
//-------------------------------------------------------------------------

void
DriverACIA::InterruptReceive() {
  printf("**** Warning: receive interrupt handler not implemented yet\n");
  exit(ERROR);
}
