//-----------------------------------------------------------------
/*! \file mem.cc
  \brief Routines for the physical page management

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

#include "vm/physMem.h"
#include "kernel/msgerror.h"
#include <unistd.h>

//-----------------------------------------------------------------
// PhysicalMemManager::PhysicalMemManager
//
/*! Constructor. It simply clears all the page flags and inserts them in the
// free_page_list to indicate that the physical pages are free
*/
//-----------------------------------------------------------------
PhysicalMemManager::PhysicalMemManager() {

  uint64_t i;
  tpr = new struct tpr_c[gConfig->NumPhysPages];

  for (i = 0; i < gConfig->NumPhysPages; i++) {
    tpr[i].free = true;
    tpr[i].locked = false;
    tpr[i].owner = NULL;
    free_page_list.Append((void *) i);
  }
  i_clock = -1;
}

PhysicalMemManager::~PhysicalMemManager() {
  // Empty free page list
  while (!free_page_list.IsEmpty())
    free_page_list.Remove();

  // Delete physical page table
  delete[] tpr;
}

//-----------------------------------------------------------------
// PhysicalMemManager::FreePhysicalPage
//
/*! This method releases an unused physical page by adding
//  it in the free_page_list. Sets up all data structures
//  accordingly
//
//  \param num_page is the number of the real page to free
*/
//-----------------------------------------------------------------
void
PhysicalMemManager::FreePhysicalPage(uint64_t num_page) {

  // Check that the page is not already free
  ASSERT(!tpr[num_page].free);

  // Update the physical page table entry
  tpr[num_page].free = true;
  tpr[num_page].locked = false;
  if (tpr[num_page].owner->translationTable != NULL)
    tpr[num_page].owner->translationTable->clearBitValid(
        tpr[num_page].virtualPage);

  // Insert the page in the free list
  free_page_list.Prepend((void *) num_page);
}

//-----------------------------------------------------------------
// PhysicalMemManager::UnlockPage
//
/*! This method unlocks the page numPage, after
//  checking the page is in the locked state. Used
//  by the page fault manager to unlock at the
//  end of a page fault (the page cannot be evicted until
//  the page fault handler terminates).
//
//  \param num_page is the number of the real page to unlock
*/
//-----------------------------------------------------------------
void
PhysicalMemManager::UnlockPage(uint64_t num_page) {
  ASSERT(num_page < gConfig->NumPhysPages);
  ASSERT(tpr[num_page].locked == true);
  ASSERT(tpr[num_page].free == false);
  tpr[num_page].locked = false;
}

//-----------------------------------------------------------------
// PhysicalMemManager::SetTPREntry
//
/*! This method sets the virtualPage, owner and locked members of a TPR entry
//  to the values given as parameters
//
//  \param pp: physical page
//  \param owner address space (for backlink)
//  \param virtualPage is the number of virtualPage
//  \param locked: true if page has to be locked
//  \return no return
//
*/
//-----------------------------------------------------------------
void PhysicalMemManager::SetTPREntry(uint64_t pp, uint64_t virtualpage,AddrSpace *owner, bool locked) {
  tpr[pp].virtualPage = virtualpage;
  tpr[pp].owner = owner;
  tpr[pp].locked = locked;
}

//-----------------------------------------------------------------
// PhysicalMemManager::FindFreePage
//
/*! This method returns a new physical page number, if it finds one
//  free. If not, return INVALID_PAGE. Does not run the clock algorithm.
//
//  \return A new free physical page number.
*/
//-----------------------------------------------------------------
uint64_t
PhysicalMemManager::FindFreePage() {
  uint64_t page;

  // Check that the free list is not empty
  if (free_page_list.IsEmpty()) {
    return INVALID_PAGE;
  }

  // Update statistics
  gCurrentThread->GetProcessOwner()->stat->incrMemoryAccess();

  // Get a page from the free list
  page = (int64_t) free_page_list.Remove();

  // Check that the page is really free
  ASSERT(tpr[page].free);

  // Update the physical page table
  tpr[page].free = false;

  return page;
}

//-----------------------------------------------------------------
// PhysicalMemManager::EvictPage
//
/*! This method implements page replacement, using the well-known
//  clock algorithm.
//
//  \return A new free physical page number.
*/
//-----------------------------------------------------------------
uint64_t
PhysicalMemManager::EvictPage() {
  printf("**** Warning: page replacement algorithm is not implemented yet\n");
  exit(ERROR);
  return (0);
}

//-----------------------------------------------------------------
// PhysicalMemManager::Print
//
/*! print the current status of the table of physical pages
//
//  \param rpage number of real page
*/
//-----------------------------------------------------------------

void
PhysicalMemManager::Print(void) {
  uint64_t i;

  printf("Contents of TPR (%" PRIu64 " pages)\n", gConfig->NumPhysPages);
  for (i = 0; i < gConfig->NumPhysPages; i++) {
    printf("Page %" PRIu64 " free=%d locked=%d virtpage=%" PRIu64
           " owner=%lx U=%d M=%d\n",
           i, tpr[i].free, tpr[i].locked, tpr[i].virtualPage,
           (long int) tpr[i].owner,
           (tpr[i].owner != NULL)
               ? tpr[i].owner->translationTable->getBitU(tpr[i].virtualPage)
               : 0,
           (tpr[i].owner != NULL)
               ? tpr[i].owner->translationTable->getBitM(tpr[i].virtualPage)
               : 0);
  }
}
