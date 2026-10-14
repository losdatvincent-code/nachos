//-----------------------------------------------------------------
/*! \file  swapManager.cc
//  \brief Routines of the swap manager
//
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

#include <unistd.h>

#include "drivers/drvDisk.h"
#include "kernel/msgerror.h"
#include "kernel/thread.h"
#include "utility/bitmap.h"
#include "vm/swapManager.h"

//-----------------------------------------------------------------
/**
 * Initializes the swapping area
 *
 * Initialize the page_flags bitmap to specify that the sectors
 * of the swapping area are free
 */
//-----------------------------------------------------------------
SwapManager::SwapManager() {

  swap_disk = new DriverDisk((char *) "sem swap disk",
                             (char *) "lock swap disk", gMachine->diskSwap);
  page_flags = new BitMap(NUM_SECTORS);
}

//-----------------------------------------------------------------
/**
 * De-allocate the swapping area
 *
 * De-allocate the page_flags bitmap
 */
//-----------------------------------------------------------------
SwapManager::~SwapManager() {

  delete page_flags;
  delete swap_disk;
}

//-----------------------------------------------------------------
/** Returns the number of a free sector in the swap area
 *
 * This method scans the allocation bitmap page_flags to decide which
 * page is used
 *
 * \return Number of free sector in the swap area, or ERROR of
 * there is no free sector available
 */
//-----------------------------------------------------------------
uint64_t
SwapManager::GetFreeSwapSector() {

  // Scan the page allocation bitmap
  for (int i = 0; i < NUM_SECTORS; i++) {
    if (!page_flags->Test(i)) {
      // the page #i is free
      page_flags->Mark(i);
      return i;
    }
  }

  // There is no available page, return ERROR
  return ERROR;
}

//-----------------------------------------------------------------
/** This method frees an unused page in the swap area by modifying the
 * page allocation bitmap. This method is called when exiting a
 * process to de-allocate its swap area
 *
 *  \param disk_addr: the sector number to free
 */
//-----------------------------------------------------------------
void
SwapManager::ReleasePageSwap(uint32_t disk_addr) {

  DEBUG('v', (char *) "Swap page %" PRIu32 " released for thread \"%s\"\n",
        disk_addr, gCurrentThread->GetName());
  // clear the #num_sector bit of page_flags
  page_flags->Clear(disk_addr);
}

//-----------------------------------------------------------------
/** Fill a buffer with the swap information in a specific sector in the swap
 * area
 *
 * \param disk_addr: disk address in the swap area
 * \param pp: number of the physical page in the machine memory
 */
//-----------------------------------------------------------------
void
SwapManager::GetPageSwap(uint32_t disk_addr, uint64_t pp) {

  DEBUG('v', (char *) "Reading swap page %" PRIu32 " for \"%s\"\n", disk_addr,
        gCurrentThread->GetName());
  swap_disk->ReadSector(disk_addr, (char*) &(gMachine->mainMemory[pp * gConfig->PageSize]));
}

//-----------------------------------------------------------------
/** This method puts a page into the swapping area. If the sector
 *  number given in parameters is set to INVALID_SECTOR, the swap manager
 *  chooses a free sector and return its number.
 *
 *  \param disk_addr: disk address in the swap area
 *  \param pp:  number of the physical page in the machine memory
 *  \return The sector number used in the swapping area. This number
 *          is used to update the field disk_page in the translation
 *          table entry.
 */
//-----------------------------------------------------------------
uint64_t
SwapManager::PutPageSwap(uint32_t disk_addr, uint64_t pp) {

  if (disk_addr != (uint32_t) INVALID_SECTOR) {
    DEBUG('v', (char *) "Writing swap page %" PRIu32 " for \"%s\"\n", disk_addr,
          gCurrentThread->GetName());
    swap_disk->WriteSector(disk_addr, (char *)&(gMachine->mainMemory[pp * gConfig->PageSize]));
    return disk_addr;
  } else {
    uint64_t newsect = GetFreeSwapSector();
    if (newsect == (uint64_t) INVALID_SECTOR) {
      return INVALID_SECTOR;
    } else {
      DEBUG('v', (char *) "Writing swap page %" PRIu32 " for \"%s\"\n", newsect,
            gCurrentThread->GetName());
      swap_disk->WriteSector(newsect, (char *)&(gMachine->mainMemory[pp * gConfig->PageSize]));
      return newsect;
    }
  }
}

//-----------------------------------------------------------------
/** This method gives to the DriverDisk for the swap area */
//-----------------------------------------------------------------
DriverDisk *
SwapManager::GetSwapDisk() {
  return swap_disk;
}
