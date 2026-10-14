/*! \file synch.h
    \brief Data structures for synchronizing threads.

        Three kinds of synchronization are defined here: semaphores,
        locks, and condition variables. Part or all of them are to be
        implemented as part of the first assignment.

        Note that all the synchronization objects take a "name" as
        part of the initialization.  This is solely for debugging purposes.

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

#ifndef SYNCH_H
#define SYNCH_H

#include "kernel/copyright.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "utility/list.h"

/*! \brief Defines the "semaphore" synchronization tool
//
// The semaphore has only two operations P() and V():
//
//	P() -- decrement, then wait if the value becomes < 0
//
//	V() -- increment, waking up a thread waiting in P() if necessary
//
// Note that the interface does *not* allow a thread to read the value of
// the semaphore directly -- even if you did read the value, the
// only thing you would know is what the value used to be.  You don't
// know what the value is now, because by the time you get the value
// into a register, a context switch might have occurred,
// and some other thread might have called P or V, so the true value might
// now be different.
*/
class Semaphore {
public:
  //! Create and set initial value
  Semaphore(char *debugName, uint32_t initialCounter);

  //! Delete semaphore
  ~Semaphore();

  //! debugging assist
  char *getName() { return semaphore_name; }

  void P();   // these are the only operations on a semaphore
  void V();   // they are both *atomic*

private:
  char *semaphore_name;     //!< useful for debugging
  int count;                //!< semaphore counter
  ListThread *wait_queue;   //!< threads waiting in P() for the value to be > 0

public:
  //! Object type, for validity checks during system calls (must be the first
  //! public field)
  ObjectType type;
};

/*! \brief Defines the "lock" synchronization tool
//
// A lock can be BUSY or FREE.
// There are only two operations allowed on a lock:
//
//	Acquire -- wait until the lock is FREE, then set it to BUSY
//
//	Release -- wake up a thread waiting in Acquire if necessary,
//	           or else set the lock to FREE
//
// In addition, by convention, only the thread that acquired the lock
// may release it.  As with semaphores, you can't read the lock value
// (because the value might change immediately after you read it).
*/
class Lock {
public:
  //! Lock creation
  Lock(char *debugName);

  //! Delete a lock
  ~Lock();

  //! For debugging
  char *getName() { return lock_name; }

  //! Acquire a lock (atomic operation)
  void Acquire();

  //! Release a lock (atomic operation)
  void Release();

  //! true if the current thread holds this lock.  Useful for checking
  //! in Release, and in Condition variable operations below.
  bool isHeldByCurrentThread();

private:
  char *lock_name;          //!< for debugging
  ListThread *wait_queue;   //!< threads waiting to acquire the lock
  bool is_free;             //!< to know if the lock is free
  Thread *owner;            //!< Thread who has acquired the lock

public:
  //! Object type, for validity checks during system calls (must be the first
  //! public field)
  ObjectType type;
};

/*! \class Condition
\brief Defines the "condition variable" synchronization tool
//
// A condition
// variable does not have a value, but threads may be queued, waiting
// on the variable.  These are only operations on a condition variable:
//
//	Wait() -- relinquish the CPU until signaled,
//
//	Signal() -- wake up a thread, if there are any waiting on
//		the condition
//
//	Broadcast() -- wake up all threads waiting on the condition
//
*/
class Condition {
public:
  //! Create a condition and initialize it to "no one waiting"
  Condition(char *debugName);

  //! Deallocate the condition
  ~Condition();

  //! For debugging
  char *getName() { return (condition_name); }

  //! Wait until the condition is signalled
  void Wait();

  //! Wake up one of the thread waiting on the condition
  void Signal();

  //! Wake up all threads waiting on the condition
  void Broadcast();

private:
  char *condition_name;     //!< For debbuging
  ListThread *wait_queue;   //!< Threads asked to wait

public:
  //! Object type, for validity checks during system calls (must be the first
  //! public field)
  ObjectType type;
};

#endif   // SYNCH_H
