/*! \file mmu.cc
//  \brief Routines to translate virtual addresses to physical addresses
//
//	Software sets up a table of legal translations.  We look up
//	in the table on every memory reference to find the true physical
//	memory location.
//
// One single types of translation are supported here, a linear page table.
// The virtual page # is used as an index
// into the table, to find the physical page #.
//
// NB: one could also use a TLB in replacement or addition of
// the linear page table (not integrated in the current version
// of the source code).
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

//
*/
// DO NOT CHANGE -- part of the machine emulation
//

#include "kernel/addrspace.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "machine/machine.h"
#include "vm/pagefaultmanager.h"
#include "vm/physMem.h"

//----------------------------------------------------------------------
// MMU::MMU()
/*! Construction. Empty for now
 */
//----------------------------------------------------------------------
MMU::MMU() { translationTable = NULL; }

//----------------------------------------------------------------------
// MMU::~MMU()
/*! Destructor. Empty for now
 */
//----------------------------------------------------------------------
MMU::~MMU() { translationTable = NULL; }

//----------------------------------------------------------------------
// MMU::ReadMem
/*!     Read "size" (1, 2, 4, 8) bytes of virtual memory at "addr" into
//	the location pointed to by "value".
//
//
//	\param addr the virtual address to read from
//	\param size the number of bytes to read (1, 2, 4, 8)
//	\param value the place to write the result
//      \return Returns false if the translation step from
//              virtual to physical memory failed, true otherwise.
*/
//----------------------------------------------------------------------
bool
MMU::ReadMem(uint64_t virtAddr, int size, uint64_t *value) {
  ExceptionType exc;
  uint32_t physAddr;
  uint32_t physAddrEnd;

  DEBUG('z', (char *) "Reading VA 0x%x, size %d\n", virtAddr, size);

  // Update statistics
  gCurrentThread->GetProcessOwner()->stat->incrMemoryAccess();

  // Perform address translation
  exc = Translate(virtAddr, &physAddr, size, false);
  Translate(virtAddr, &physAddrEnd, size, false);
  if (exc == NO_EXCEPTION)
    ASSERT(physAddr == physAddrEnd);

  // Raise an exception if one has been detected during address translation
  if (exc != NO_EXCEPTION) {
    gMachine->RaiseException(exc, virtAddr);
    return false;
  }

  // Read data from main memory
  switch (size) {
  case 1:
    *value = gMachine->mainMemory[physAddr];
    break;

  case 2:
    *value = *(uint16_t *) &gMachine->mainMemory[physAddr];
    break;

  case 4:
    *value = *(uint32_t *) &gMachine->mainMemory[physAddr];
    break;

  case 8:
    *value = *(uint64_t *) &gMachine->mainMemory[physAddr];
    break;

  default:
    ASSERT(false);
  }

  DEBUG('z', (char *) "\tValue read = %8.8x\n", *value);

  return (true);
}

//----------------------------------------------------------------------
// MMU::WriteMem
/*!     Write "size" (1, 2, 4, 8) bytes of the contents of "value" into
//	virtual memory at location "addr".
//
//   	Returns false if the translation step from virtual to physical memory
//   	failed.
//
//	\param addr the virtual address to write to
//	\param size the number of bytes to be written (1, 2, 4, 8)
//	\param value the data to be written
*/
//----------------------------------------------------------------------
bool
MMU::WriteMem(uint64_t addr, int size, uint64_t value) {
  ExceptionType exc;
  uint32_t physicalAddress;
  uint32_t physAddrEnd;

  DEBUG('z', (char *) "Writing VA 0x%x, size %d, value 0x%x\n", addr, size,
        value);

  // Update statistics
  gCurrentThread->GetProcessOwner()->stat->incrMemoryAccess();

  // Perform address translation
  exc = Translate(addr, &physicalAddress, size, true);
  Translate(addr, &physAddrEnd, size, true);
  if (exc == NO_EXCEPTION)
    ASSERT(physicalAddress == physAddrEnd);

  if (exc != NO_EXCEPTION) {
    gMachine->RaiseException(exc, addr);
    return false;
  }

  // Write into the machine main memory
  switch (size) {
  case 1:
    gMachine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
    break;

  case 2:
    *(uint16_t *) &gMachine->mainMemory[physicalAddress] =
        (uint16_t) (value & 0xffff);
    break;

  case 4:
    *(uint32_t *) &gMachine->mainMemory[physicalAddress] = (uint32_t) value;
    break;
  case 8:
    *(uint64_t *) &gMachine->mainMemory[physicalAddress] = (uint64_t) value;
    break;
  default:
    ASSERT(false);
  }
  DEBUG('h', (char *) "\tValue written");

  return true;
}

//----------------------------------------------------------------------
// MMU::Translate(uint32_t virtAddr, uint32_t *physAddr, int size, bool writing)
/*! 	Translate a virtual address into a physical address, using
//      a linear page table.
//         - Look for a translation of the virtual page in the page table
//             - if found, check access rights and physical address
//                correctness, returns the physical page
//         - else Look for a translation of the virtual page in the
//           translation pages
//             - make sure the entry corresponds to a correct entry,
//               ie it maps something (phys mem or disk) <=> at least one
//               of the readAllowed or writeAllowed bits is set ?
//             - check access rights
//	       - If bit valid=true : physical page already known
//	       - Else if bit valid=false : raise a page fault exception
//             - returns the physical page
//
//      If everything is ok, set the use/dirty bits in
//	the translation table entry, and store the translated physical
//	address in "physAddr".  If there was an error, returns the type
//	of the exception.
//
//	\param virtAddr the virtual address to translate
//	\param physAddr pointer to the place to store the physical address
*/
ExceptionType
MMU::Translate(uint32_t virtAddr, uint32_t *physAddr, int size, bool writing) {
  DEBUG('h', (char *) "\tTranslate 0x%x, %s: ", virtAddr,
        writing ? "write" : "read");

  // Check for alignment errors
  if (((size == 4) && (virtAddr & 0x3))
      || ((size == 2) && (virtAddr & 0x1))){
    DEBUG('h', (char *)"alignment problem at %d, size %d!\n", virtAddr, size);
    return BUSERROR_EXCEPTION;
  }

  // Compute virtual page number and offset in the page
  int vpn = virtAddr / gConfig->PageSize;
  int offset = virtAddr % gConfig->PageSize;

  /*
   * Complete the addres translation
   */

  // check the virtual page number
  if (vpn >= translationTable->getMaxNumPages()) {
    DEBUG('h', (char *) "virtual page # %d too large for page table size %d!\n",
          vpn, translationTable->getMaxNumPages());
    return ADDRESSERROR_EXCEPTION;
  }

  // is the page correctly mapped ?
  if (!translationTable->getBitReadAllowed(vpn) &&
      !translationTable->getBitWriteAllowed(vpn)) {
    DEBUG('h', (char *) "virtual page # %d not mapped !\n", vpn);
    return ADDRESSERROR_EXCEPTION;
  }

  // Check access rights
  if (writing && !translationTable->getBitWriteAllowed(vpn)) {
    DEBUG('h', (char *) "write access on read-only virtual page # %d !\n", vpn);
    return READONLY_EXCEPTION;
  }

  // If the page is not yet in main memory, run the page fault manager
  if (!translationTable->getBitValid(vpn)) {
    // Update statistics
    gCurrentThread->GetProcessOwner()->stat->incrPageFault();
    DEBUG('h', (char *) "Raising page fault exception for page number %i\n",
          vpn);

    // call the page fault manager
    gMachine->RaiseException(PAGEFAULT_EXCEPTION, virtAddr);

    if (!translationTable->getBitValid(vpn)) {
      printf("Error: page fault failed (bit valid should be set to 1)\n");
      exit(ERROR);
    }
  }

  // Make sure physical address is correct
  if ((translationTable->getPhysicalPage(vpn) < 0) ||
      (translationTable->getPhysicalPage(vpn) >= gConfig->NumPhysPages)) {
    DEBUG('h', (char *) "MMU: Translated physical page out of bounds (0x%x)\n",
          translationTable->getPhysicalPage(vpn));
    return BUSERROR_EXCEPTION;
  }

  // Set the U/M bits
  if (writing) {
    translationTable->setBitM(vpn);
  }
  translationTable->setBitU(vpn);
  gCurrentThread->GetProcessOwner()->stat->incrMemoryAccess();

  *physAddr = translationTable->getPhysicalPage(vpn) * gConfig->PageSize + offset;
  DEBUG('h', (char *) "phys addr = 0x%x\n", *physAddr);
  return NO_EXCEPTION;
}
