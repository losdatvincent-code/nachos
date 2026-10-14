/*! \file interrupt.cc
//  \brief Routines to simulate hardware interrupts.
//
//	The hardware provides a routine (SetStatus) to enable or disable
//	interrupts.
//
//	In order to emulate the hardware, we need to keep track of all
//	interrupts the hardware devices would cause, and when they
//	are supposed to occur.
//
//	This module also keeps track of simulated time.  Time advances
//	only when the following occur:
//		interrupts are re-enabled
//		a user instruction is executed
//		there is nothing in the ready queue
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
//  DO NOT CHANGE -- part of the machine emulation
//

#include "kernel/system.h"
#include "kernel/thread.h"
#include "machine/machine.h"
#include "utility/stats.h"

//! String definition for debugging messages
static char *intLevelNames[] = {(char *) "off", (char *) "on"};
//! String definition for debugging messages
static char *intTypeNames[] = {
    (char *) "timer",        (char *) "disk",         (char *) "console write",
    (char *) "console read", (char *) "ACIA receive", (char *) "ACIA send"};

//----------------------------------------------------------------------
// PendingInterrupt::PendingInterrupt
/*! 	Initialize a hardware device interrupt that is to be scheduled
//	to occur in the near future.
//
//	\param func is the procedure to call when the interrupt occurs
//	\param param is the argument to pass to the procedure
//	\param time is when (in simulated time) the interrupt is to occur
//	\param kind is the hardware device that generated the interrupt
*/
//----------------------------------------------------------------------
PendingInterrupt::PendingInterrupt(VoidFunctionPtr func, int64_t param, Time t,
                                   IntType kind) {
  handler = func;
  arg = param;
  when = t;
  type = kind;
}

//----------------------------------------------------------------------
// Interrupt::Interrupt
/*! 	Constructor. Initialize the simulation of hardware device interrupts.
//
//	Interrupts start disabled, with no interrupts pending, etc.
*/
//----------------------------------------------------------------------
Interrupt::Interrupt() {
  level = INTERRUPTS_OFF;
  pending = new ListTime;
  inHandler = false;
  yieldOnReturn = false;
}

//----------------------------------------------------------------------
// Interrupt::~Interrupt
//! 	De-allocate the data structures needed by the interrupt simulation.
//----------------------------------------------------------------------
Interrupt::~Interrupt() {
  while (!pending->IsEmpty())
    delete (PendingInterrupt *) (pending->Remove());
  delete pending;
}

//----------------------------------------------------------------------
// Interrupt::ChangeLevel
/*! 	Change interrupts to be enabled or disabled, without advancing
//	the simulated time (normally, enabling interrupts advances the time).
//
//	Used internally.
//
//	\param old  the old interrupt status
//	\param now the new interrupt status
*/
//----------------------------------------------------------------------
void
Interrupt::ChangeLevel(IntStatus old, IntStatus now) {
  level = now;
  DEBUG('i', (char *) "\tinterrupts: %s -> %s\n", intLevelNames[old],
        intLevelNames[now]);
}

//----------------------------------------------------------------------
// Interrupt::SetStatus
/*! 	Change interrupts to be enabled or disabled, and if interrupts
//	are being enabled, advance simulated time by calling OneTick().
//
//  \return
//	The old interrupt status.
//
//  \param now the new interrupt status
*/
//----------------------------------------------------------------------
IntStatus
Interrupt::SetStatus(IntStatus now) {
  IntStatus old = level;

  ASSERT((now == INTERRUPTS_OFF) ||
         (inHandler == false));   // interrupt handlers are
                                  // prohibited from enabling
                                  // interrupts

  ChangeLevel(old, now);   // change to new state
  if ((now == INTERRUPTS_ON) && (old == INTERRUPTS_OFF))
    OneTick(SYSTEM_TICK);   // advance simulated time
  return old;
}

//----------------------------------------------------------------------
// Interrupt::OneTick
/*! 	Advance simulated time and check if there are any pending
//	interrupts to be called.
//
//	Two things can cause OneTick to be called:
//	- interrupts are re-enabled
//	- a user instruction is executed
*/
//----------------------------------------------------------------------
void
Interrupt::OneTick(int nbcycles) {
  ASSERT(level == INTERRUPTS_ON);   // interrupts need to be enabled,
                                    // to check for an interrupt handler

  MachineStatus old = gMachine->GetStatus();

  // advance simulated time
  if (gMachine->GetStatus() == SYSTEM_MODE) {
    gCurrentThread->GetProcessOwner()->stat->incrSystemTicks(nbcycles);
  } else {
    gCurrentThread->GetProcessOwner()->stat->incrUserTicks(nbcycles);
  }

  // check any pending interrupts are now ready to fire
  ChangeLevel(INTERRUPTS_ON, INTERRUPTS_OFF);   // first, turn off interrupts
                                                // (interrupt handlers run with
                                                // interrupts disabled)
  while (CheckIfDue(false))                     // check for pending interrupts
    ;
  ChangeLevel(INTERRUPTS_OFF, INTERRUPTS_ON);   // re-enable interrupts
  if (yieldOnReturn) {   // if the timer device handler asked
                         // for a context switch, ok to do it now
    yieldOnReturn = false;
    gMachine->SetStatus(SYSTEM_MODE);   // yield is a kernel routine
    gCurrentThread->Yield();
    gMachine->SetStatus(old);
  }
}

//----------------------------------------------------------------------
// Interrupt::YieldOnReturn
/*! 	Called from within an interrupt handler, to cause a context switch
//	(for example, on a time slice) in the interrupted thread,
//	when the handler returns.
//
//	We can't do the context switch here, because that would switch
//	out the interrupt handler, and we want to switch out the
//	interrupted thread.
*/
//----------------------------------------------------------------------

void
Interrupt::YieldOnReturn() {
  ASSERT(inHandler == true);
  yieldOnReturn = true;
}

//----------------------------------------------------------------------
// Interrupt::Idle
/*! 	Routine called when there is nothing in the ready queue.
//
//	Since something has to be running in order to put a thread
//	on the ready queue, the only thing to do is to advance
//	simulated time until the next scheduled hardware interrupt.
//
//	If there are no pending interrupts, stop.  There's nothing
//	more for us to do.
*/
//----------------------------------------------------------------------
void
Interrupt::Idle() {
  DEBUG('i', (char *) "Machine idling; checking for interrupts.\n");
  gMachine->SetStatus(IDLE_MODE);
  if (CheckIfDue(true)) {       // check for any pending interrupts
    while (CheckIfDue(false))   // check for any other pending
      ;                         // interrupts
    yieldOnReturn = false;      // since there's nothing in the
                                // ready queue, the yield is automatic
    gMachine->SetStatus(SYSTEM_MODE);
    return;   // return in case there's now
              // a runnable thread
  }

  // if there are no pending interrupts, and nothing is on the ready
  // queue, it is time to stop.   If the console or the ACIA is
  // operating, there are *always* pending interrupts, so this code
  // is not reached.  Instead, the halt must be invoked by the user program.

  DEBUG('i', (char *) "Machine idle.  No interrupts to do.\n");
  printf("No threads ready or runnable, and no pending interrupts.\n");
  printf("Assuming the program completed.\n");
  Halt(0);
}

//----------------------------------------------------------------------
// Interrupt::Halt
//! 	Shut down Nachos cleanly, printing out performance statistics.
//      Exits with an error code that corresponds to the execution
//         of the user process
//----------------------------------------------------------------------
void
Interrupt::Halt(int errorcode) {
  printf("Machine halting!\n\n");
  Cleanup();
  Exit(errorcode);
}

//----------------------------------------------------------------------
// Interrupt::Schedule
/*! 	Arrange for the CPU to be interrupted when simulated time
//	reaches "now + when".
//
//	Implementation: just put it on a sorted list.
//
//	NOTE: the Nachos kernel should not call this routine directly.
//	Instead, it is only called by the hardware device simulators.
//
//	\param handler is the procedure to call when the interrupt occurs
//	\param arg is the argument to pass to the procedure
//	\param fromNow is how far in the future (in simulated time) the
//		 interrupt is to occur
//	\param type is the hardware device that generated the interrupt
*/
//----------------------------------------------------------------------
void
Interrupt::Schedule(VoidFunctionPtr handler, int64_t arg, int fromNow,
                    IntType type) {
  Time when;
  when = gStats->getTotalTicks() + fromNow;
  PendingInterrupt *toOccur = new PendingInterrupt(handler, arg, when, type);

  ASSERT(toOccur != NULL);

  DEBUG('i', (char *) "Scheduling interrupt handler %s at time = %llu\n",
        intTypeNames[type], when);
  ASSERT(fromNow > 0);
  pending->SortedInsert(toOccur, when);
}

//----------------------------------------------------------------------
// Interrupt::CheckIfDue
/*! 	Check if an interrupt is scheduled to occur, and if so, fire it off.
//
// \return
//	true, if we fired off any interrupt handlers
//
// \param advanceClock if true, there is nothing in the ready queue,
//		so we should simply advance the clock to when the next
//		pending interrupt would occur (if any).  If the pending
//		interrupt is just the time-slice daemon, however, then
//		we're done!
*/
//----------------------------------------------------------------------
bool
Interrupt::CheckIfDue(bool advanceClock) {
  MachineStatus old = gMachine->GetStatus();
  Time when;

  ASSERT(level == INTERRUPTS_OFF);   // interrupts need to be disabled,
                                     // to invoke an interrupt handler
  if (DebugIsEnabled('i'))
    DumpState();
  PendingInterrupt *toOccur = (PendingInterrupt *) pending->SortedRemove(&when);

  if (toOccur == NULL)   // no pending interrupts
  {
    return false;
  }

  if (advanceClock && when > gStats->getTotalTicks()) {   // advance the clock
    gStats->incrIdleTicks(when - gStats->getTotalTicks());
    gStats->setTotalTicks(when);
    //	delete when;
  } else if (when > gStats->getTotalTicks()) {   // not time yet, put it back
    pending->SortedInsert(toOccur, when);
    return false;
  }

  // Check if there is nothing more to do, and if so, quit
  if ((gMachine->GetStatus() == IDLE_MODE) && (toOccur->type == TIMER_INT) &&
      pending->IsEmpty()) {
    pending->SortedInsert(toOccur, when);
    printf("this is the end \n");
    return false;
  }

  inHandler = true;
  gMachine->SetStatus(SYSTEM_MODE);   // whatever we were doing,
                                       // we are now going to be
                                       // running in the kernel
  (*(toOccur->handler))(toOccur->arg);   // call the interrupt handler
  gMachine->SetStatus(old);             // restore the machine status
  inHandler = false;
  delete toOccur;
  return true;
}

//----------------------------------------------------------------------
// PrintPending
/*! 	Print information about an interrupt that is scheduled to occur.
//	When, where, why, etc.
*/
//----------------------------------------------------------------------

static void
PrintPending(int64_t arg) {
  PendingInterrupt *pend = (PendingInterrupt *) arg;

  printf("Interrupt handler %s, scheduled at time %" PRIu64 "\n",
         intTypeNames[pend->type], pend->when);
}

//----------------------------------------------------------------------
// DumpState
/*! 	Print the complete interrupt state - the status, and all interrupts
//	that are scheduled to occur in the future.
*/
//----------------------------------------------------------------------
void
Interrupt::DumpState() {
  printf("Pending interrupts:\n");
  fflush(stdout);
  pending->Mapcar(PrintPending);
  printf("End of pending interrupts\n");
  fflush(stdout);
}
