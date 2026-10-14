/*! \file  machine.h
    \brief Data structures to simulate the RISC V machine

 User programs are loaded into "mainMemory"; to Nachos-RiscV,
 this looks just like an array of bytes.  Of course, the Nachos-RiscV
 kernel is in memory too -- but as in most machines these days,
 the kernel is loaded into a separate memory region from user
 programs, and accesses to kernel memory are not translated or paged.

 In Nachos-RiscV, user programs are executed one instruction at a time,
 by the RISC V simulator.  Each memory reference is translated, checked
 for errors, etc.

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

#ifndef MACHINE_H
#define MACHINE_H

// Forward definition
class ACIA;

#define MAX(a, b) ((a) > (b) ? a : b)
#define MIN(a, b) ((a) < (b) ? a : b)

#define STACK_REG   2   //!< User's stack pointer
#define RETADDR_REG 1   //!< Holds return address for procedure calls

#define NUM_INT_REGS 32   //!< Number of integer registers
#define NUM_FP_REGS  32   //!< Number of floating point registers

#include <math.h> /* For emulating floating point RISCV instructions */
#include <sstream>
#include <stdint.h>
#include <string>
#include <strings.h>

enum ExceptionType : short {
  NO_EXCEPTION,             //!< Everything ok!
  SYSCALL_EXCEPTION,        //!< A program executed a system call.
  PAGEFAULT_EXCEPTION,      //!< Page fault exception
  READONLY_EXCEPTION,       /*!< Write attempted to
                              page marked "read-only" */
  BUSERROR_EXCEPTION,       /*!< Translation resulted
                              in an invalid physical
                              address (mis-aligned or
                              out-of-bounds) */
  ADDRESSERROR_EXCEPTION,   /*!< Reference that was
                              not mapped in the address
                              space */
  OVERFLOW_EXCEPTION,       //!< Integer overflow in add or sub.
  ILLEGALINSTR_EXCEPTION,   //!< Unimplemented or reserved instr.

  NUM_EXCEPTION_TYPES
};

#include "kernel/copyright.h"
#include "machine/disk.h"
#include "machine/instruction.h"
#include "utility/stats.h"

#include "machine/ACIA.h"
#include "machine/interrupt.h"
#include "machine/mmu.h"
#include "machine/translationtable.h"

// Possible exceptions recognized by the machine

class Console;

/*! Nachos-RiscV can be running kernel code (SYSTEM_MODE), user code
 (USER_MODE), or there can be no runnable thread, because the ready list is
 empty (IDLE_MODE).
*/
enum MachineStatus { IDLE_MODE, SYSTEM_MODE, USER_MODE };

// User program CPU state.  The full set of RISC registers, plus a few
// more because we need to be able to start/stop a user program between
// any two instructions (thus we need to keep track of things like load
// delay slots, etc.)

#define STACK_REG   2   //!< User's stack pointer
#define RETADDR_REG 1   //!< Holds return address for procedure calls

#define NUM_INT_REGS 32   //!< Number of integer registers
#define NUM_FP_REGS  32   //!< Number of floating point registers

// Registers used for syscalls
#define REG_NO_SYSCALL      17
#define REG_RET_SYSCALL     10
#define REG_SYSCALL_PARAM_1 10
#define REG_SYSCALL_PARAM_2 11
#define REG_SYSCALL_PARAM_3 12
#define REG_SYSCALL_PARAM_4 13

/*! \brief Defines the simulated execution hardware
//
// User programs shouldn't be able to tell that they are running on our
// simulator or on the real hardware, except
//	- we only partially support floating point instructions (only
//	  "ordered operations", no FP "likely branches", no fixed point
//	  words
//	- the system call interface to Nachos is not the same as UNIX
//	  (10 system calls in Nachos vs. 200 in UNIX!)
// If we were to implement more of the UNIX system calls, we ought to be
// able to run Nachos on top of Nachos!
//
// The procedures in this class are defined in machine.cc, riscvsim.cc, and
// translate.cc.
*/

class Machine {
public:
  Machine(bool debug);   //!<  Constructor. Initialize the RISCV machine
                         //!<  for running user programs
  ~Machine();            //!<  Destructor. De-allocate the data structures

  // Routines callable by the Nachos kernel
  void Run();   //!< Run a user program

  int64_t
  ReadIntRegister(int num);   //!< Read the contents of an Integer CPU register

  void WriteIntRegister(int num, int64_t value);
  //!< Store a value into an Integer CPU register

  int64_t
  ReadFPRegister(int num);   //!< Read the contents of a floating point register

  void WriteFPRegister(int num, int64_t value);
  //!< store a value into a floating point register

  MachineStatus GetStatus() { return status; }   //!< idle, kernel, user
  void SetStatus(MachineStatus st) { status = st; }

  // Routines internal to the machine simulation -- DO NOT call these

  int OneInstruction(Instruction *instr);
  //!< Run one instruction of a user program.
  //!< Return the execution time of the instr (cycle)

  void RaiseException(ExceptionType which, int badVAddr);
  //!< Trap to the Nachos kernel, because of a
  //!< system call or other exception.

  void Debugger();    //!< Invoke the user program debugger
  void DumpState();   //!< Print the user CPU and memory state

  // Data structures -- all of these are accessible to Nachos kernel code.
  // "public" for convenience.
  //
  // Note that *all* communication between the user program and the kernel
  // are in terms of these data structures.

  int64_t int_registers[NUM_INT_REGS];   //!< CPU Integer registers, for
                                         //!< executing user programs

  int64_t float_registers[NUM_FP_REGS];   //!< Floating point general purpose
                                          //!< registers
                                          // Warning : We actually only support
                                          // SINGLE precision float operations.

  char is32Bits;   //!< is the program executed compiled in 32 or 64 bits

  int64_t pc;   //!< program counter

  int64_t badvaddr_reg;   //!< The failing virtual address on an exception

  int8_t *mainMemory; /*!< Physical memory to store user program,
                        code and data, while executing
                      */

  MMU *mmu;             /*!< Machine memory management unit */
  ACIA *acia;           /*!< ACIA Hardware */
  Interrupt *interrupt; /*!< Interrupt management */
  Disk *disk;           /*!< Raw disk device (hardware) */
  Disk *diskSwap;       /*!< Swap raw disk device (hardware) */
  Console *console;     /*!< Console */

private:
  MachineStatus status;   //!< idle, kernel mode, user mode

  bool singleStep;   /*!< Drop back into the debugger after each
                       simulated instruction
                     */
  Time runUntilTime; /*!< Drop back into the debugger when simulated
                       time reaches this value
                     */
  uint64_t shiftMask[64];

  uint64_t n_inst;
  uint64_t cycle;
};

//! Entry point into Nachos to handle user system calls and exceptions.
// Defined in exception.cc
extern void ExceptionHandler(ExceptionType which, int vaddr);

// Routines for converting Words and Short Words to and from the
// simulated machine's format of little endian.  If the host machine
// is little endian (DEC and Intel), these end up being NOPs.
//
// What is stored in each format:
//	- host byte ordering:
//	   - kernel data structures
//	   - user registers
//	- simulated machine byte ordering:
//	   - contents of main memory
uint64_t LongToHost(uint64_t word);
uint32_t WordToHost(uint32_t word);
uint16_t ShortToHost(uint16_t shortword);
uint64_t LongToMachine(uint64_t word);
uint32_t WordToMachine(uint32_t word);
uint16_t ShortToMachine(uint16_t shortword);

#endif   // MACHINE_H
