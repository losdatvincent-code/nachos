/*! \file drvACIA.h
  \brief Data structures for the ACIA device driver
   interrupt_send and interrupt_receive are used in the intrrupt mode only.

        The ACIA is an asynchronous device (requests return
        immediately, and an interrupt happens later on).
        This is a layer on top of the ACIA.
        Two working modes are to be implemented in assignment 2:
        a Busy Waiting mode and an Interrupt mode. The Busy Waiting
        mode implements a synchronous IO whereas IOs are asynchronous
        IOs are implemented in the Interrupt mode (see the Nachos
        roadmap for further details).
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

#ifndef _ACIA_HDL
#define _ACIA_HDL

/* Includes */

#include "kernel/synch.h"   // for the acces to the synchronisation's tools

#define BUFFER_SIZE 256   // size of emission and reception buffers

/*! The class DriverACIA defines the handler of the ACIA. It is the
system's interface between the user programs and the ACIA (simulated)
hardware.*/

/*! \brief Defines an ACIA (Asynchronous Communication Interface Adapter)
device driver */
class DriverACIA {

private:
  char send_buffer[BUFFER_SIZE];      //!< system buffer for emission
  char receive_buffer[BUFFER_SIZE];   //!< system buffer for reception
  Semaphore *send_sema;   //!< semaphore used to synchronize emission requests
  Semaphore
      *receive_sema;   //!< semaphore used to synchronize reception requests

  int ind_send;   //!< index in the emission buffer
  int ind_rec;    //!< index in the reception buffer

public:
  //! Constructor. Driver initialization.
  DriverACIA();

  //! Send a message through the ACIA
  int TtySend(char *buff);

  //! Receive a message using the ACIA
  int TtyReceive(char *buff, int lg);

  //! Emission interrupt handler. Used in the ACIA Interrupt mode only
  void InterruptSend();

  //! Reception interrupt handler. Used in the ACIA Interrupt mode only
  void InterruptReceive();
};
#endif   // _ACIA_HDL
