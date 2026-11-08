/*! \file thread.cc
//  \brief Routines to manage threads.
//
//   There are four main operations:
//	- Constructor : create an inactive thread
//      - Start : bind the thread to a process, and prepare it to be
//               dispatched on the CPU
//	- Finish : called when a thread finishes, to clean up
//	- Yield : relinquish control over the CPU to another ready thread
//	- Sleep : relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly
//		put back on the ready queue.
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

#include "kernel/thread.h"
#include "kernel/msgerror.h"
#include "kernel/scheduler.h"
#include "kernel/synch.h"

#define UNSIGNED_LONG_AT_ADDR(addr) (*((unsigned long int *) (addr)))

#define STACK_FENCEPOST                                                        \
  0xdeadbeef   // this is put at the end of the
               // simulator stack, for detecting
               // stack overflows

//----------------------------------------------------------------------
// Thread::Thread
/*! 	Constructor. Initialize an empty thread (just a name)
//
//	\param threadName is an arbitrary string, used for debugging.
*/
//----------------------------------------------------------------------
Thread::Thread(char *threadName) {
  thread_name = new char[strlen(threadName) + 1];
  strcpy(thread_name, threadName);
  type = THREAD_TYPE;

  // No process owner yet
  process = NULL;
}

//----------------------------------------------------------------------
// Thread::~Thread
/*! 	Destructor. De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      When the last thread of a process has finished, its
//      process can be deallocated.
*/
//----------------------------------------------------------------------

Thread::~Thread() {
  DEBUG('t', (char *) "Deleting thread \"%s\"\n", thread_name);
  type = INVALID_TYPE;

  // CheckOverflow();

  // Delete the simulator stack In case this==gCurrentThread, it
  // means we are currently deleting the last executing thread in
  // the system at system shutdown time. It this situation, we do not
  // free the stack since we are still using it
  if (this != gCurrentThread)
    DeallocBoundedArray(simulator_context.stackBottom,
                        simulator_context.stackSize);

  // NB: the thread stack itself is not freed, we do not attempt to
  // reuse the address space dedicated to stack The corresponding
  // physical memory will be deallocated when the process will be
  // deleted

  // Protect from other accesses to the process object
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  // Signals to the process that we terminated
  process->numThreads--;

  // If I'm the last thread of the process, delete it
  if (process->numThreads == 0) {
    delete process;
  }

  gMachine->interrupt->SetStatus(oldLevel);

  delete[] thread_name;
}

//----------------------------------------------------------------------
// Thread::Start
/*!  Attach a thread to a process context (essentially an address
//   space), and prepare it to be dispatched on the CPU
//
// \return NO_ERROR on success, an error code on error
*/
//----------------------------------------------------------------------
int
Thread::Start(Process *owner, int64_t func, int64_t arg) {
  ASSERT(process == NULL);
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Thread::Start is not implemented yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  // 1. le thread appartient au processus owner. on lui alloue sa pile
  //    utilisateur (pile RISC-V) dans l'espace d'adressage du processus.
  //    StackAllocate renvoie l'adresse du haut de cette pile.
  process = owner;
  stack_pointer = process->addrspace->StackAllocate();

  // 2. pile du simulateur : c'est sur cette pile (cote machine hote)
  //    que s'execute le code de Nachos quand ce thread a le processeur.
  int8_t *simulator_stack = AllocBoundedArray(SIMULATOR_CTX_SIZE);

  // 3. contexte du simulateur : la premiere fois que l'on passera sur
  //    ce thread, c'est la fonction StartThreadExecution qui sera lancee
  //    (elle demarre l'execution des instructions RISC-V).
  InitSimulatorContext(simulator_stack, SIMULATOR_CTX_SIZE);

  // 4. contexte RISC-V : pc = adresse de la fonction a executer,
  //    sp = haut de la pile utilisateur, a0 = argument de la fonction.
  InitThreadContext(func, stack_pointer, arg);

  // 5. on enregistre le thread dans le noyau. ces listes sont partagees
  //    avec les autres threads, donc on coupe les interruptions.
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);
  process->numThreads++;           // un thread de plus dans le processus
  gAlive->Append((void *) this);   // liste des threads vivants (pour Join)
  gScheduler->ReadyToRun(this);    // file des threads prets
  gMachine->interrupt->SetStatus(oldLevel);

  return NO_ERROR;
#endif
}

//----------------------------------------------------------------------
// Thread::InitThreadContext
/*!	Set the initial values for the thread contact
//
//      \param initialPCREG initial value for the PC register
//      \param initialSP initial value for the SP register
//      \param arg argument to pass to the user thread
*/
//----------------------------------------------------------------------
void
Thread::InitThreadContext(int64_t initialPCREG, int64_t initialSP,
                          int64_t arg) {
  int i;

  for (i = 0; i < NUM_INT_REGS; i++)
    thread_context.int_registers[i] = 0;

  // Initial program counter -- must be location of "Start"
  thread_context.pc = initialPCREG;

  // Arguments
  thread_context.int_registers[10] = arg;

  // Set the stack register
  thread_context.int_registers[STACK_REG] = initialSP;
}

//----------------------------------------------------------------------
// StartThreadExecution, ThreadPrint
/*!	Dummy function because C++ does not allow a pointer to a member
//	function.  So in order to do this, we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the
//	member function.
*/
//----------------------------------------------------------------------
void
ThreadPrint(long arg) {
  Thread *t = (Thread *) arg;
  printf("%s", t->GetName());
}

void
StartThreadExecution(void) {
  printf("****  Starting thread\n");
  gMachine->interrupt->SetStatus(INTERRUPTS_ON);
  gMachine->Run();
  // Should not return there ...
  ASSERT(0);
}

//----------------------------------------------------------------------
// Thread::InitSimulatorContext
/*!
//
//  Sets-up the simulator context : fills it with the appropriate
//  values such that the low-level context switch executes function
//  StartThreadExecution
// 	\param base_stack_addr is the lowest address of the kernel stack
//
//----------------------------------------------------------------------
*/
void
Thread::InitSimulatorContext(int8_t *base_stack_addr,
                             unsigned long int stack_size) {
  DEBUG('t', (char *) "Init simulator context \"%s\" with stack=%p\n", thread_name,
        base_stack_addr);

  ASSERT(base_stack_addr != NULL);

  // Fill in buf with the current context
  // and then fill busf such that StartThreadExecution
  // will be called when a setcontext will be made on buf
  // NB: the gcc implementation of makecontext
  //     interprets ss_sp as the stack BASE and not stack BOTTOM
  //     (may not be portable to other architectures/compilers)
  ASSERT(getcontext(&(simulator_context.buf)) == 0);
  simulator_context.buf.uc_stack.ss_sp = base_stack_addr;
  simulator_context.buf.uc_stack.ss_size = stack_size;
  simulator_context.buf.uc_stack.ss_flags = 0;
  simulator_context.buf.uc_link = NULL;
  makecontext(&simulator_context.buf, StartThreadExecution, 0);

  // Setup kernel stack parameters for low-level context switch
  simulator_context.stackBottom = base_stack_addr;
  simulator_context.stackSize = stack_size;

  // Mark the bottom of the stack in order to detect stack overflows
  UNSIGNED_LONG_AT_ADDR(simulator_context.stackBottom) = STACK_FENCEPOST;
}

//----------------------------------------------------------------------
// Thread::Join
/*!
//      Sleep the thread until another thread finishes.
//	\param Idthread thread to wait for
//----------------------------------------------------------------------
*/
void
Thread::Join(Thread *Idthread) {
  while (gAlive->Search(Idthread))
    Yield();
}

//----------------------------------------------------------------------
// Thread::CheckOverflow
/*! 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
*/
//----------------------------------------------------------------------

void
Thread::CheckOverflow() {
  ASSERT(UNSIGNED_LONG_AT_ADDR(simulator_context.stackBottom) ==
         STACK_FENCEPOST);
}

//----------------------------------------------------------------------
// Thread::Finish
/*! 	Called by static function threadStart when a thread has finished
//      its job (see userlib/libnachos.c).
//
// 	NOTE: we don't immediately de-allocate the thread data structure
//	or the execution stack, because we're still running in the thread
//	and we're still on the stack!  Instead, we set
"gThreadToBeDestroyed",
//	so that Scheduler::SwitchTo() will call the destructor, once we're
//	running in the context of a different thread.
//
// 	NOTE: we disable interrupts, so that we don't get a time slice
//	between setting gThreadToBeDestroyed and going to sleep.
*/
//----------------------------------------------------------------------
void
Thread::Finish() {

  DEBUG('t', (char *) "Finishing thread \"%s\"\n", GetName());

#ifndef ETUDIANTS_TP
  printf("**** Warning: method Thread::Finish is not fully implemented yet\n");
#endif
#ifdef ETUDIANTS_TP
  // on coupe les interruptions (Sleep() l'exige). on ne les remet pas :
  // ce thread ne reprendra jamais la main.
  gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  // on retire le thread de la liste des threads vivants : Join() attend
  // tant que le thread attendu est dans gAlive.
  // (on teste d'abord avec Search, car RemoveItem boucle a l'infini si
  // l'element n'est pas dans la liste.)
  if (gAlive->Search((void *) this))
    gAlive->RemoveItem((void *) this);

  // on retire aussi le thread de la table des objets : ainsi un appel
  // systeme fait plus tard avec son identifiant (Join par exemple)
  // obtiendra NULL, au lieu d'un pointeur vers un objet detruit.
  gObjectAddrs->RemoveObject((void *) this);

  // on ne peut pas faire "delete this" maintenant, car on s'execute
  // encore sur la pile de ce thread. on note juste qu'il faut le
  // detruire : c'est le prochain thread elu qui le fera (fin de
  // Scheduler::SwitchTo, ou StartThreadExecution).
  gThreadToBeDestroyed = this;
#endif

  // Go to sleep
  Sleep();   // invokes SWITCH
}

//----------------------------------------------------------------------
// Thread::Yield
/*! 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled.
*/
//----------------------------------------------------------------------
void
Thread::Yield() {
  Thread *nextThread;
  IntStatus oldLevel = gMachine->interrupt->SetStatus(INTERRUPTS_OFF);

  ASSERT(this == gCurrentThread);

  DEBUG('t', (char *) "Yielding thread \"%s\"\n", GetName());

  nextThread = gScheduler->FindNextToRun();
  if (nextThread != NULL) {
    gScheduler->ReadyToRun(this);
    gScheduler->SwitchTo(nextThread);
  }
  (void) gMachine->interrupt->SetStatus(oldLevel);
}

//----------------------------------------------------------------------
// Thread::Sleep
/*! 	Relinquish the CPU, because the current thread is blocked
//	waiting on a synchronization variable (Semaphore, Lock, or Condition).
//	Eventually, some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
*/
//----------------------------------------------------------------------
void
Thread::Sleep() {
  Thread *nextThread;

  ASSERT(this == gCurrentThread);
  ASSERT(gMachine->interrupt->GetStatus() == INTERRUPTS_OFF);

  DEBUG('t', (char *) "Sleeping thread \"%s\"\n", GetName());

  // In case, there is nobody else to execute, we wait for an
  // interrupt In case there is no interrupt to come in the future,
  // Nachos exists
  //
  // Note that during this phase, variable gCurrentThread is still
  // set to the thread which is put to sleep, which is weird and
  // would need to be fixed
  while ((nextThread = gScheduler->FindNextToRun()) == NULL) {
    DEBUG('t', (char *) "Nobody to run => idle\n");
    gMachine->interrupt->Idle();   // no one to run, wait for an interrupt
  }

  // Once we have another thread to execute, perform the context switch
  gScheduler->SwitchTo(nextThread);
}

//----------------------------------------------------------------------
// Thread::SaveProcessorState
/*!	Save the CPU state of a user program on a context switch
 */
//----------------------------------------------------------------------
void
Thread::SaveProcessorState() {
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Thread::SaveProcessorState is not implemented "
         "yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  // on recopie les registres du processeur RISC-V emule dans le contexte
  // du thread : registres entiers, registres flottants et pc.
  // (le pc pointe deja sur l'instruction qui suit le ecall, car il est
  // incremente au moment ou l'instruction est lue.)
  for (int i = 0; i < NUM_INT_REGS; i++)
    thread_context.int_registers[i] = gMachine->ReadIntRegister(i);
  for (int i = 0; i < NUM_FP_REGS; i++)
    thread_context.float_registers[i] = gMachine->ReadFPRegister(i);
  thread_context.pc = gMachine->pc;
#endif
}

//----------------------------------------------------------------------
// Thread::RestoreProcessorState
/*!	Restore the CPU state of a user program on a context switch.
 */
//----------------------------------------------------------------------

void
Thread::RestoreProcessorState() {
#ifndef ETUDIANTS_TP
  printf("**** Warning: method Thread::RestoreProcessorState is not "
         "implemented yet\n");
  exit(ERROR);
#endif
#ifdef ETUDIANTS_TP
  // l'inverse de SaveProcessorState : on remet dans le processeur les
  // registres sauvegardes du thread.
  // (changer aussi la table des pages de la MMU, pour avoir plusieurs
  // processus, fait partie du TP 2.)
  for (int i = 0; i < NUM_INT_REGS; i++)
    gMachine->WriteIntRegister(i, thread_context.int_registers[i]);
  for (int i = 0; i < NUM_FP_REGS; i++)
    gMachine->WriteFPRegister(i, thread_context.float_registers[i]);
  gMachine->pc = thread_context.pc;
#endif
}

//----------------------------------------------------------------------
// Thread::SaveSimulatorState
/*!	Save the simulator state.
 */
//----------------------------------------------------------------------
void
Thread::SaveSimulatorState() {
  getcontext(&(simulator_context.buf));
}

//----------------------------------------------------------------------
// Thread::RestoreSimulatorState
/*!	Restore simulator state.
 */
//----------------------------------------------------------------------
void
Thread::RestoreSimulatorState() {
  setcontext(&(simulator_context.buf));
}
