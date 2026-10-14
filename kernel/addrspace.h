/** \file  addrspace.h
    \brief Data structures to keep track of the memory resources
           of executing user programs (address spaces).

           Don't look at this code in the first assignment.
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

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "filesys/openfile.h"
#include "kernel/copyright.h"
#include "machine/machine.h"
#include "utility/list.h"

// Forward references
class Thread;
class Semaphore;
class OpenFile;
class Process;

#define MAX_MAPPED_FILES 10
//! Information describing a memory-mapped file
typedef struct {
  int32_t first_address;
  int size;   // size in bytes
  OpenFile *file;
} s_mapped_file;
typedef s_mapped_file t_mapped_files[MAX_MAPPED_FILES];

/**
 @brief Defines the data structures to keep track of memory resources of
 executing user programs (address spaces).

 The constructor of this class takes as parameter an ELF executable
 file and loads it in the RAM of the (simulated) MIPS processor. This
 part will be modified in the virtual memory assignment to implement
 demand paging (on-demand loading of code/data).

*/
class AddrSpace {
public:
  /** 	Create an address space to run a user program.
   //	Load the program from a file "elfFile", and set everything
   //	up so that we can start executing user instructions.
   //
   //      Executables are in ELF (Executable and Linkable Format) (see elf32.h)
   //      and can be generated using a standard MIPS cross-compiler
   //      (here gcc).
   //
   //      For now, the code and data are entirely loaded into memory and
   //      the stacks are preallocated (NB: memory here stands for the
   //      memory of the simulated MIPS machine). Code/Data loading will
   //      be changed in the virtual memory assignment.
   //
   //      Don't look at this code right now. You may get lost. You will
   //      have plenty of time to do so in the virtual memory assignment
   //
   //	\param elfFile is the file containing the object code
   //             to load into memory, or NULL when the address space
   //             should be empty
   //   \param process: process to be executed
   //   \param err: error code 0 if OK, -1 otherwise
   */
  AddrSpace(OpenFile *elfFile, Process *p, uint64_t *err);

  /**   Deallocates an address space and in particular frees
   *   all memory it uses (RAM and swap area).
   */
  ~AddrSpace();

  /**	Allocates a new stack of size cfg->UserStackSize
   *
   *      Allocation is done by calling Alloc, a very simple
   *      allocation procedure of virtual memory areas.
   *
   *      \return stack pointer (at the end of the allocated stack)
   */
  uint64_t StackAllocate();

  /** Returns the address of the first instruction to execute in the process
    found in the ELF file */
  int32_t getCodeStartAddress32() { return (int32_t) CodeStartAddress; }

  int64_t getCodeStartAddress64() { return (int64_t) CodeStartAddress; }

  /*! Translation table. This table will be discovered in the virtual
    memory assignement, and is used to know where virtual pages are
    allocated in RAM. */
  TranslationTable *translationTable;

  /*! Map an open file in memory
   *
   * \param f: pointer to open file descriptor
   * \param size: size to be mapped (rounded up to next page boundary)
   */
  uint64_t Mmap(OpenFile *f, int size);

  /*! Search if the address is in a memory-mapped file
   *
   * \param addr: virtual address to be searched for
   * \return address of file descriptor if found, NULL otherwise
   */
  OpenFile *findMappedFile(int64_t addr);

private:
  //* Code start address, found in the ELF file
  int64_t CodeStartAddress;

  /**  Allocate numPages virtual pages in the current address space
   //
   //    \param numPages the number of contiguous virtual pages to allocate
   //    \return the virtual page number of the beginning of the allocated
   //      area, or -1 when not enough virtual space is available
   */
  int Alloc(int numPages);

  /** Number of the next virtual page to be allocated.
    Virtual addresses allocated in a very simple manner : an
    allocation will simply increment this address by
    the size of the allocated object (there are no complex
    malloc/free functions implemented yet) */
  int freePageId;

  /*! (Heavyweight) process using this address space */
  Process *process;

  /*! List of memory-mapped files */
  int nb_mapped_files;
  t_mapped_files mapped_files;
};

#endif   // ADDRSPACE_H
