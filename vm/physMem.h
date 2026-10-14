//-----------------------------------------------------------------
/*! \file mem.h
    \brief Data structures for the physical page management

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
//-----------------------------------------------------------------

#ifndef __MEM_H
#define __MEM_H

class PhysicalMemManager;

#include "kernel/addrspace.h"
#include "kernel/synch.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "machine/machine.h"
#include "utility/list.h"
#include "vm/swapManager.h"

//-----------------------------------------------------------------
/*! \brief Implements the physical page management.

   This class manages the physical pages of the programs which run on the
   top of the Nachos kernel. It keeps track of which physical pages are used
   and which are free.

   It processes a new page demand by applying the clock algorithm when
   there is no page available. It requires an access to the thread list
   in order to choose which page will be swapped using the SwapManager
   class.
*/
//-----------------------------------------------------------------

class PhysicalMemManager {
public:
  PhysicalMemManager();    //!< initialize the memory manager
  ~PhysicalMemManager();   //!< de-allocate the page_flags bitmap

  void FreePhysicalPage(uint64_t numPage);   //!< Frees the page and deletes the
                                             //!< existing page mapping
  void UnlockPage(uint64_t numPage);         //!< Unlock physical page
  void Print(void);                          //!< Print the contents of a page

  uint64_t FindFreePage();   //!< Return a free page if there is one
  uint64_t EvictPage();      //!< Return a free page when there is none

  // Update the physical page table
  void SetTPREntry(uint64_t pp, uint64_t virtualpage, AddrSpace *owner,
                   bool locked);

private:
  /*! \brief Describes the allocation of physical pages. Bits U
    (used/referenced) and M (modified/dirty) are in the page table entry and are
    directly set by the MMU hardware */
  struct tpr_c {
    bool free;     //!< true if page is free
    bool locked;   //!< true if page is locked in memory (system page or page
                   //!< under sap in/out)
    uint64_t virtualPage;   //!< Number of the virtualPage which references this
                            //!< real page
    AddrSpace *owner;       //!< Address space of the owner process
  };

  struct tpr_c *tpr;   //!< RealPage Array to know the state of each real page

  ListInt free_page_list;   //!< List of available (unused) real page numbers

  uint64_t i_clock;   //!< Index for clock_algorithm

  friend class AddrSpace;   //!< Direct access to page table for programm
                            //!< loading
};

#endif   // __MEM_H
