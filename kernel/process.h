/*! \file  process.h
    \brief A process keeps track of the resources used by a running
           Nachos program

        The resources considered are the memory (addrspace.h), the
        processor (thread.h), the open files, the statistics, ...
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

#ifndef PROCESS_H
#define PROCESS_H

#include "filesys/openfile.h"
#include "kernel/addrspace.h"
#include "kernel/copyright.h"
#include "kernel/synch.h"
#include "utility/stats.h"

class AddrSpace;
class Thread;
class Semaphore;

/*! \brief Defines the data structures to keep track of the execution
 environment of a user program */
class Process {
public:
  /*!
   * Create an address space, initializing it with the program stored
   * in the file "executable", without any thread in it.
   */
  Process(char *filename, uint64_t *err);

  /*! Process destructor */
  ~Process();

  /*! Pointer on the executable file (may be NULL) */
  OpenFile *elfFile;

  AddrSpace *addrspace; /*!< Pointer to the address space */

  int numThreads; /*!< Number of threads attached
                    to this process */

  ProcessStat *stat; /*!< Statistics concerning this
                       process */

  char *getName() { return (name); } /*!< Returns the process name */

private:
  char *name;
};

#endif   // PROCESS_H
