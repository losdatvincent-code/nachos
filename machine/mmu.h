/*! \file mmu.h
   \brief Data structures for the MMU (Memory Management Unit)

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

#ifndef MMU_H
#define MMU_H

/*! \brief Defines a MMU - Memory Management Unit
 */
// This object manages the memory of the simulated MIPS processor for
// the Nachos kernel.
class MMU {
public:
  MMU();

  ~MMU();

  bool ReadMem(uint64_t addr, int size, uint64_t *value);
  //!< Read or write 1, 2, 4 or 8 bytes
  //!< of virtual  memory (at addr).
  //!< Return FALSE if a correct
  //!< translation couldn't be found.

  bool WriteMem(uint64_t addr, int size, uint64_t value);
  //!< Write or write 1, 2, 4 or 8 bytes
  //!< of <virtual memory (at addr).
  //!< Return FALSE if a correct
  //!< translation couldn't be found.

  ExceptionType Translate(uint32_t virtAddr, uint32_t *physAddr, int size,
                          bool writing);
  //!< Translate an address, and check for
  //!< alignment. Set the use and dirty bits in
  //!< the translation entry appropriately,
  //!< and return an exception code if the
  //!< translation couldn't be completed.

  // NOTE: the hardware translation of virtual addresses in the user program
  // to physical addresses (relative to the beginning of "mainMemory")
  // is controlled by a traditional linear page table
  TranslationTable *translationTable;   //!< Pointer to the translation table
};

#endif   // MMU_H
