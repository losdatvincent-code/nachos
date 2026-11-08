/*! \file synch.cc
//  \brief Routines for synchronizing threads.
//
//      Three kinds of synchronization routines are defined here:
//      semaphores, locks and condition variables.
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation. We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts. While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes.
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


#include "kernel/synch.h"
#include "kernel/msgerror.h"
#include "kernel/scheduler.h"
#include "kernel/system.h"
#include <cstddef>

//----------------------------------------------------------------------
// Semaphore::Semaphore
/*! 	Initializes a semaphore, so that it can be used for synchronization.
//
// \param debugName is an arbitrary name, useful for debugging only.
// \param initialValue is the initial value of the semaphore.
*/
//----------------------------------------------------------------------
Semaphore::Semaphore(char *debugName, uint32_t initialCount) {
  semaphore_name = new char[strlen(debugName) + 1];
  strcpy(semaphore_name, debugName);
  count = initialCount;
  wait_queue = new ListThread;
  type = SEMAPHORE_TYPE;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
/*! 	De-allocates a semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
*/
//----------------------------------------------------------------------
Semaphore::~Semaphore() {
  type = INVALID_TYPE;
  if (!wait_queue->IsEmpty()) {
    DEBUG('s',
          (char *) "Destructor of semaphore \"%s\", queue is not empty!!\n",
          semaphore_name);
    Thread *t = (Thread *) wait_queue->Remove();
    DEBUG('s', (char *) "Queue contents %s\n", t->GetName());
    wait_queue->Append((void *) t);
  }
  ASSERT(wait_queue->IsEmpty());
  delete[] semaphore_name;
  delete wait_queue;
}

//----------------------------------------------------------------------
// Semaphore::P
/*!
// Implementation of P (counter decrementation+blocking if required).
// Checking the
// value and decrementing must be done atomically, so we
// need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
*/
//----------------------------------------------------------------------
void
Semaphore::P() {
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Semaphore::P is not implemented yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  // version du cours ou le compteur peut devenir negatif :
  // si count vaut -n, alors n threads sont bloques dans wait_queue.

  // on coupe les interruptions pour que P() soit atomique : aucun autre
  // thread ne doit pouvoir passer entre le test du compteur et la mise
  // en attente.
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  count--;
  if (count < 0) {
    // plus de place : le thread se met dans la file du semaphore et
    // s'endort. Sleep() donne le processeur a un autre thread pret, ou
    // attend une interruption s'il n'y en a aucun (par exemple la fin
    // d'une lecture disque). on ressort de Sleep() quand un V() nous a
    // remis dans la file des threads prets.
    DEBUG('s', (char *) "%s se bloque sur le semaphore %s\n",
          gCurrentThread->GetName(), semaphore_name);
    wait_queue->Append((void *) gCurrentThread);
    gCurrentThread->Sleep();
  }

  // on remet les interruptions comme on les a trouvees
  gMachine->interrupt->SetStatus(oldLevel);
#endif
}

//----------------------------------------------------------------------
// Semaphore::V
/*! 	Implementation of V (counter incrementation + waking up a waiting thread
//  if required).
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that interrupts
//	are disabled when it is called.
*/
//----------------------------------------------------------------------
void
Semaphore::V() {
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Semaphore::V is not implemented yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  count++;
  if (count <= 0) {
    // avant l'incrementation le compteur etait negatif, donc au moins un
    // thread attend : on reveille le premier de la file en le remettant
    // dans la file des threads prets.
    Thread *t = (Thread *) wait_queue->Remove();
    gScheduler->ReadyToRun(t);
  }

  // remarque : V() ne bloque jamais et ne change jamais de thread, car il
  // est aussi appele par les routines d'interruption des drivers (console,
  // disque), ou on n'a pas le droit de changer de thread.
  gMachine->interrupt->SetStatus(oldLevel);
#endif
}

//----------------------------------------------------------------------
// Lock::Lock
/*! 	Initialize a Lock, so that it can be used for synchronization.
//      The lock is initialy free
//  \param "debugName" is an arbitrary name, useful for debugging.
*/
//----------------------------------------------------------------------
Lock::Lock(char *debugName) {
  lock_name = new char[strlen(debugName) + 1];
  strcpy(lock_name, debugName);
  wait_queue = new ListThread;
  is_free = true;
  owner = NULL;
  type = LOCK_TYPE;
#ifdef ETUDIANTS_TP
  nb_acquire = 0;
#endif
}

//----------------------------------------------------------------------
// Lock::~Lock
/*! 	De-allocate lock, when no longer needed. Assumes that no thread
//      is waiting on the lock.
*/
//----------------------------------------------------------------------
Lock::~Lock() {
  type = INVALID_TYPE;
  ASSERT(wait_queue->IsEmpty());
  delete[] lock_name;
  delete wait_queue;
}

//----------------------------------------------------------------------
// Lock::Acquire
/*! 	Wait until the lock become free.  Checking the
//	state of the lock (free or busy) and modify it must be done
//	atomically, so we need to disable interrupts before checking
//	the value of is_free.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
*/
//----------------------------------------------------------------------
void
Lock::Acquire() {
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Lock::Acquire is not implemented yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  if (is_free) {
    // cas 1 : le verrou est libre, on le prend.
    is_free = false;
    owner = gCurrentThread;
    nb_acquire = 1;
  } else if (owner == gCurrentThread) {
    // cas 2 : on a deja le verrou (comme demande dans le sujet, on ne se
    // bloque pas, sinon on attendrait pour toujours que l'on se libere
    // soi-meme). on compte juste un Acquire de plus.
    nb_acquire++;
  } else {
    // cas 3 : le verrou est pris par un autre thread, on attend.
    // quand on sera reveille, Release() nous aura deja donne le verrou
    // (owner = nous, voir Lock::Release) : il n'y a rien d'autre a faire.
    DEBUG('s', (char *) "%s se bloque sur le verrou %s\n",
          gCurrentThread->GetName(), lock_name);
    wait_queue->Append((void *) gCurrentThread);
    gCurrentThread->Sleep();
  }

  gMachine->interrupt->SetStatus(oldLevel);
#endif
}

//----------------------------------------------------------------------
// Lock::Release
/*! 	Wake up a waiter if necessary, or release it if no thread is waiting.
//	As with Acquire, this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
*/
//----------------------------------------------------------------------
void
Lock::Release() {
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Lock::Release is not implemented yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  // seul le proprietaire a le droit de relacher le verrou. l'appel
  // systeme LockRelease le verifie avant de nous appeler, donc si ce
  // test echoue c'est une erreur dans le noyau.
  ASSERT(owner == gCurrentThread);

  nb_acquire--;
  if (nb_acquire == 0) {
    // c'est le dernier Release (autant de Release que d'Acquire) : le
    // verrou doit etre libere.
    Thread *t = (Thread *) wait_queue->Remove();
    if (t == NULL) {
      // personne n'attend : le verrou devient libre.
      is_free = true;
      owner = NULL;
    } else {
      // quelqu'un attend : on lui donne directement le verrou. le verrou
      // reste pris, il change juste de proprietaire, donc aucun autre
      // thread ne peut le prendre avant lui (ordre d'arrivee respecte).
      owner = t;
      nb_acquire = 1;
      gScheduler->ReadyToRun(t);
    }
  }

  gMachine->interrupt->SetStatus(oldLevel);
#endif
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
/*! To check if current thread hold the lock
 */
//----------------------------------------------------------------------
bool
Lock::isHeldByCurrentThread() {
  return (gCurrentThread == owner);
}

//----------------------------------------------------------------------
// Condition::Condition
/*! 	Initializes a Condition, so that it can be used for synchronization.
//
//    \param  "debugName" is an arbitrary name, useful for debugging.
*/
//----------------------------------------------------------------------
Condition::Condition(char *debugName) {
  condition_name = new char[strlen(debugName) + 1];
  strcpy(condition_name, debugName);
  wait_queue = new ListThread;
  type = CONDITION_TYPE;
}

//----------------------------------------------------------------------
// Condition::~Condition
/*! 	De-allocate condition, when no longer needed.
//      Assumes that nobody is waiting on the condition.
*/
//----------------------------------------------------------------------
Condition::~Condition() {
  type = INVALID_TYPE;
  ASSERT(wait_queue->IsEmpty());
  delete[] condition_name;
  delete wait_queue;
}

//----------------------------------------------------------------------
// Condition::Wait
/*! Block the calling thread (put it in the wait queue).
//  This operation must be atomic, so we need to disable interrupts.
*/
//----------------------------------------------------------------------
void
Condition::Wait() {
  printf("**** Warning: method Condition::Wait is not implemented yet\n");
  exit(ERROR);
}

//----------------------------------------------------------------------
// Condition::Signal
/*! Wake up the first thread of the wait queue (if any).
// This operation must be atomic, so we need to disable interrupts.
*/
//----------------------------------------------------------------------
void
Condition::Signal() {
  printf("**** Warning: method Condition::Signal is not implemented yet\n");
  exit(ERROR);
}

//----------------------------------------------------------------------
// Condition::Broadcast
/*! Wake up all threads waiting in the waitqueue of the condition
// This operation must be atomic, so we need to disable interrupts.
*/
//----------------------------------------------------------------------
void
Condition::Broadcast() {
  printf("**** Warning: method Condition::Broadcast is not implemented yet\n");
  exit(ERROR);
}
