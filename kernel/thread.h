/*! \file thread.h
    \brief Data structures for managing threads

    A thread represents sequential execution of code within a program.
    So the state of a thread includes the program counter,
    the processor registers, and the execution stack.

    Note that because we allocate a fixed size stack for each
    thread, it is possible to overflow the stack -- for instance,
    by recursing to too deep a level.

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

class Thread;

#ifndef THREAD_H
#define THREAD_H

#include "kernel/copyright.h"
#include "kernel/process.h"
#include "kernel/system.h"
#include "machine/machine.h"
#include "utility/stats.h"
#include "utility/utility.h"
#include <ucontext.h>

// Size of the simulator's context 
#define SIMULATOR_CTX_SIZE (32 * 1024)   // in Bytes

// External function, dummy routine whose sole job is to call Thread::Print.
extern void ThreadPrint(long arg);

class Semaphore;
class Process;

/*! \brief Defines the context of the Nachos simulator
 */
typedef struct {
  ucontext_t buf;
  int8_t *stackBottom;
  int stackSize;
} simulatorContextT;

/*! \brief Defines the thread context (MIPS virtual machine)
 */
typedef struct {
  //! Integer CPU register state (value of
  //  MIPS registers)
  int64_t int_registers[NUM_INT_REGS];

  //! Floating point general purpose registers
  int64_t float_registers[NUM_FP_REGS];

  //! Program counter
  int64_t pc;
} threadContextT;

/*! \brief Data structures for managing threads
 *
 */
class Thread {
public:
  //! Build an empty thread
  Thread(char *debugName);

  //! Deallocate a Thread.
  ~Thread();

  //! Start a thread, attaching it to a process (return NoError on success)
  int Start(Process *owner, int64_t func, int64_t arg);

  //! Wait for another thread to finish its execution
  void Join(Thread *Idthread);

  //! Relinquish the CPU if any other thread is runnable.
  void Yield();

  //! Put the thread to sleep and relinquish the processor
  void Sleep();

  //! Finish the execution of the thread, and prepare its deallocation
  void Finish();

  //! Check if a thread has overflowed its stack.
  void CheckOverflow();

  //! Sets-up the thread simulator context : fills it with the appropriate
  //  values such that the low-level context switch executes function
  //  StartThreadExecution.
  void InitSimulatorContext(int8_t *stack_addr, unsigned long int stack_size);

  //! Initialize CPU registers,
  //  before jumping to user code
  void InitThreadContext(int64_t initialPCREG, int64_t initialSP, int64_t arg);

  //! Save the processor registers.
  void SaveProcessorState();

  //! Restore the processor registers.
  void RestoreProcessorState();

  //! Save the state of the Nachos simulator.
  void SaveSimulatorState();

  //! Restore Nachos simulator state.
  void RestoreSimulatorState();

  char *GetName() { return (thread_name); }
  Process *GetProcessOwner() { return process; }

protected:
  //! Thread name (for debugging)
  char *thread_name;

  //! Main resource container the thread is running in.
  Process *process;

  //! MIPS simulator context
  simulatorContextT simulator_context;

  //! Thread context
  threadContextT thread_context;

public:
  //! signature to make sure the thread is in the correct state
  ObjectType type;

  uint64_t stack_pointer;
};

#endif   // THREAD_H
