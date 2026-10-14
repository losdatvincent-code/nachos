//-----------------------------------------------------------------------
/*! \file scheduler.h
    \brief Defines the Nachos scheduler

   The class defines the scheduler/dispatcher abstraction --
   the data structures and operations needed to keep track of which
   thread is running, and which threads are ready but not running.

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
//-----------------------------------------------------------------------

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "kernel/copyright.h"
#include "utility/list.h"

class Thread;

class Scheduler {
public:
  //! Constructor. Initializes list of ready threads.
  Scheduler();

  //! Destructor. De-allocates the ready list.
  ~Scheduler();

  //! Inserts a thread in the ready list
  void ReadyToRun(Thread *thread);

  //! Dequeue first thread of the ready list, if any, and return thread.
  Thread *FindNextToRun();

  //! Causes a context switch to nextThread
  void SwitchTo(Thread *nextThread);

  //! Print contents of ready list.
  void Print();

protected:
  //! Queue of threads that are ready to run, but not running.
  ListThread *readyList;
};

#endif   // SCHEDULER_H
