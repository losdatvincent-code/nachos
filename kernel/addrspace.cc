/*! \file  addrspace.cc
//  \brief Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//         1. Generate an ELF (Executable and Linkable Format) binary
//            using a MIPS cross-compiler (see how to do this in
//            test/Makefile)
//         2. Load the ELF file into the Nachos file system
//            (see documentation of configuration file nachos.cfg)
//         3. Execute it. You can do this
//            - when stating Nachos (see Nachos.cfg)
//            - by calling the appropriate system call in another
//              program (Exec)
//            - by typing the program name in the Nachos shell
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

#include "kernel/addrspace.h"
#include "filesys/filehdr.h"
#include "filesys/filesys.h"
#include "filesys/openfile.h"
#include "kernel/elf.h"
#include "kernel/msgerror.h"
#include "kernel/synch.h"
#include "kernel/system.h"
#include "utility/stats.h"
#include "vm/physMem.h"

//----------------------------------------------------------------------
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
 //   \param err: error code 0 if OK, ERROR otherwise
 */
//----------------------------------------------------------------------
AddrSpace::AddrSpace(OpenFile *elfFile, Process *p, uint64_t *err) {
  char eident[16];
  *err = 0;
  translationTable = NULL;
  freePageId = 0;
  process = p;
  char is32Bits = 0;

  /* Empty user address space requested ? */
  if (elfFile == NULL) {
    // Allocate translation table now
    translationTable = new TranslationTable();
    return;
  }

  // Read the 16 first bytes of the Header to check if the
  // file is 32 or 64 bits
  elfFile->ReadAt((char *) &eident, 16, 0);
  if (eident[EI_CLASS] == ELFCLASS32)
    is32Bits = 1;
  else if (eident[EI_CLASS] == ELFCLASS64)
    is32Bits = 0;
  else {
    printf("Error, the ELF File was not recognized as a 32 or 64 bits file, "
           "exiting.\n");
    exit(ERROR);
  }

  // Read elf header and check file format
  ElfFile elff(elfFile, is32Bits, err);
  if (*err != NO_ERROR) {
    printf("Error, wrong file format for ELF file, exiting.\n");
    exit(ERROR);
  }

  printf("\n****  Loading file %s :\n", elfFile->GetName());

  // Create an empty translation table
  translationTable = new TranslationTable();

  // Compute the highest virtual address to init the translation table
  uint64_t mem_topaddr = 0;
  for (int i = 0; i < elff.getShNum(); i++) {
    // Ignore empty sections
    if (elff.getShSize(i) <= 0)
      continue;
    uint64_t section_topaddr = elff.getShAddr(i) + elff.getShSize(i);
    if ((elff.getShFlags(i) & SHF_ALLOC) && (section_topaddr > mem_topaddr))
      mem_topaddr = section_topaddr;
  }

  // Allocate space in virtual memory
  int base_addr = this->Alloc(divRoundUp(mem_topaddr, gConfig->PageSize));
  // Make sure this region really starts at virtual address 0
  ASSERT(base_addr == 0);

  DEBUG('a', (char *) "Allocated virtual area [0x0,0x%x[ for program\n",
        mem_topaddr);

  // Loading of all sections
  for (int i = 0; i < elff.getShNum(); i++) {
    // Retrieve the section name
    const char *section_name = elff.getShName(i);

    DEBUG('a', (char *) "Section %d : size=0x%x name=\"%s\"\n", i,
          elff.getShSize(i), section_name);

    // Ignore empty sections
    if (elff.getShSize(i) <= 0)
      continue;

    // Look if this section has to be loaded (SHF_ALLOC flag)
    if (!(elff.getShFlags(i) & SHF_ALLOC))
      continue;

    printf("\t- Section %s : file offset 0x%x, size 0x%x, addr 0x%x, %s%s\n",
           section_name, (unsigned) elff.getShOffset(i),
           (unsigned) elff.getShSize(i), (unsigned) elff.getShAddr(i),
           (elff.getShFlags(i) & SHF_WRITE) ? "R/W" : "R",
           (elff.getShFlags(i) & SHF_EXECINSTR) ? "/X" : "");

    // Make sure section is aligned on page boundary
    ASSERT((elff.getShAddr(i) % gConfig->PageSize) == 0);


    // Initializes the page table entries and loads the section
    // in memory (demand paging will be implemented later on)
    for (unsigned int pgdisk = 0,
                      virt_page = elff.getShAddr(i) / gConfig->PageSize;
         pgdisk < divRoundUp(elff.getShSize(i), gConfig->PageSize);
         pgdisk++, virt_page++) {

      /* Without demand paging */

      // Set up default values for the page table entry
      translationTable->clearBitSwap(virt_page);
      translationTable->setBitReadAllowed(virt_page);

      if (elff.getShFlags(i) & SHF_WRITE)
        translationTable->setBitWriteAllowed(virt_page);
      else
        translationTable->clearBitWriteAllowed(virt_page);
      translationTable->clearBitIo(virt_page);

      // Get a page in physical memory, halt of there is not sufficient space
      int pp = gPhysicalMemManager->FindFreePage();
      if (pp == INVALID_PAGE) {
        printf("Not enough free space to load program %s\n",
               elfFile->GetName());
        gMachine->interrupt->Halt(ERROR);
      }
      gPhysicalMemManager->tpr[pp].virtualPage = virt_page;
      gPhysicalMemManager->tpr[pp].owner = this;
      gPhysicalMemManager->tpr[pp].locked = true;
      translationTable->setPhysicalPage(virt_page, pp);

      // The SHT_NOBITS flag indicates if the section has an image
      // in the executable file (text or data section) or not
      // (bss section)
      if (elff.getShType(i) != SHT_NOBITS) {
        // The section has an image in the executable file
        // Read it from the disk
        elfFile->ReadAt(
            (char *) &(gMachine->mainMemory[translationTable->getPhysicalPage(
                                                 virt_page) *
                                             gConfig->PageSize]),
            gConfig->PageSize, elff.getShOffset(i) + pgdisk * gConfig->PageSize);

      } else {
        // The section does not have an image in the executable
        // Fill it with zeroes
        memset(&(gMachine
                     ->mainMemory[translationTable->getPhysicalPage(virt_page) *
                                  gConfig->PageSize]),
               0, gConfig->PageSize);
      }

      // The page has been loded in physical memory but
      // later-on will be saved in the swap disk. We have to indicate this
      // in the translation table
      translationTable->setAddrDisk(virt_page, INVALID_SECTOR);

      // The entry is valid
      translationTable->setBitValid(virt_page);

      /* End of code without demand paging */
    }
  }

  // Get program start address
  CodeStartAddress = (int32_t) elff.getEntry();
  printf("\t- Program start address : 0x%lx\n\n",
         (unsigned long) CodeStartAddress);

  // Init the number of memory mapped files to zero
  nb_mapped_files = 0;
}

//----------------------------------------------------------------------
/**   Deallocates an address space and in particular frees
 *   all memory it uses (RAM and swap area).
 */
//----------------------------------------------------------------------
AddrSpace::~AddrSpace() {
  int i;

  if (translationTable != NULL) {

    // For every virtual page
    for (i = 0; i < freePageId; i++) {

      // If it is in physical memory, free the physical page
      if (translationTable->getBitValid(i)) {
        gPhysicalMemManager->FreePhysicalPage(
            translationTable->getPhysicalPage(i));
      }
      // If it is in the swap disk, free the corresponding disk sector
      if (translationTable->getBitSwap(i)) {
        int addrDisk = translationTable->getAddrDisk(i);
        if (addrDisk >= 0) {
          gSwapManager->ReleasePageSwap(translationTable->getAddrDisk(i));
        }
      }
    }
    delete translationTable;
  }
}

//----------------------------------------------------------------------
/**	Allocates a new stack of size gConfig->UserStackSize
 *
 *      Allocation is done by calling Alloc, a very simple
 *      allocation procedure of virtual memory areas.
 *
 *      \return stack pointer (at the end of the allocated stack)
 */
//----------------------------------------------------------------------
uint64_t
AddrSpace::StackAllocate(void) {
  // Optional : leave an anmapped blank space below the stack to
  // detect stack overflows
#define STACK_BLANK_LEN 4   // in pages
  int blankaddr = this->Alloc(STACK_BLANK_LEN);
  DEBUG('a',
        (char *) "Allocated unmapped virtual area [0x%x,0x%x[ for stack "
                 "overflow detection\n",
        blankaddr * gConfig->PageSize,
        (blankaddr + STACK_BLANK_LEN) * gConfig->PageSize);

  // The new stack parameters
  int stackBasePage, numPages;
  numPages = divRoundUp(gConfig->UserStackSize, gConfig->PageSize);

  // Allocate virtual space for the new stack
  stackBasePage = this->Alloc(numPages);
  ASSERT(stackBasePage >= 0);
  // Print address range for stack even in non debug mode to help debugging
  // in case of stack overflow
  printf("****  Stack: allocated virtual area [0x%x,0x%x[ for thread\n",
         stackBasePage * gConfig->PageSize,
         (stackBasePage + numPages) * gConfig->PageSize);
  DEBUG('a', (char *) "Allocated virtual area [0x%x,0x%x[ for stack\n",
        stackBasePage * gConfig->PageSize,
        (stackBasePage + numPages) * gConfig->PageSize);

  for (int i = stackBasePage; i < (stackBasePage + numPages); i++) {
    /* Without demand paging */
    // Allocate a new physical page for the stack, halt if not page available
    int pp = gPhysicalMemManager->FindFreePage();
    if (pp == INVALID_PAGE) {
      printf("Not enough free space to load stack\n");
      gMachine->interrupt->Halt(ERROR);
    }
    gPhysicalMemManager->tpr[pp].virtualPage = i;
    gPhysicalMemManager->tpr[pp].owner = this;
    gPhysicalMemManager->tpr[pp].locked = true;
    translationTable->setPhysicalPage(i, pp);

    // Fill the page with zeroes
    memset(&(gMachine->mainMemory[translationTable->getPhysicalPage(i) *
                                   gConfig->PageSize]),
           0x0, gConfig->PageSize);
    translationTable->setAddrDisk(i, INVALID_SECTOR);
    translationTable->setBitValid(i);
    translationTable->clearBitSwap(i);
    translationTable->setBitReadAllowed(i);
    translationTable->setBitWriteAllowed(i);
    translationTable->clearBitIo(i);
    /* End of code without demand paging */
  }

  int stackpointer =
      (stackBasePage + numPages) * gConfig->PageSize - 4 * sizeof(int);
  return stackpointer;
}

//----------------------------------------------------------------------
/**  Allocate numPages virtual pages in the current address space
//
//    \param numPages the number of contiguous virtual pages to allocate
//    \return the virtual page number of the beginning of the allocated
//      area, or INVALID_PAGE when not enough virtual space is available
*/
//----------------------------------------------------------------------
int
AddrSpace::Alloc(int numPages) {
  int result = freePageId;

  DEBUG('a', (char *) "Virtual space alloc request for %d pages\n", numPages);

  // Check if the translation table is big enough for the allocation
  // to succeed
  if (freePageId + numPages >= translationTable->getMaxNumPages())
    return INVALID_PAGE;

  // Do the allocation.
  // The allocation procedure is VERY SIMPLE. It just remembers
  // the number of the lastly allocated virtual page and increments it
  // when new pages are allocated. No de-allocation mechanisms is
  // implemented.
  freePageId += numPages;
  return result;
}

//----------------------------------------------------------------------
/** Map an open file in memory
 *
 * \param f: pointer to open file descriptor
 * \param size: size to be mapped in bytes (rounded up to next page boundary)
 * \return the virtual address at which the file is mapped
 */
// ----------------------------------------------------------------------
uint64_t
AddrSpace::Mmap(OpenFile *f, int size) {
  printf("**** Warning: method AddrSpace::Mmap is not implemented yet\n");
  exit(ERROR);
}

//----------------------------------------------------------------------
/*! Search if the address is in a memory-mapped file
 *
 * \param addr: virtual address to be searched for
 * \return address of file descriptor if found, NULL otherwise
 */
//----------------------------------------------------------------------
OpenFile *
AddrSpace::findMappedFile(int64_t addr) {
  printf("**** Warning: method AddrSpace::findMappedFile is not implemented "
         "yet\n");
  exit(ERROR);
}
