/*! \file drvDisk.h
    \brief Data structures to export a synchronous interface to the raw
           disk device.

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

#include "kernel/copyright.h"

#ifndef SYNCHDISK_H
#define SYNCHDISK_H

#include "kernel/synch.h"
#include "machine/disk.h"

class Semaphore;
class Lock;

/*! \brief Defines a "synchronous" disk abstraction.
//
// As with other I/O devices, the raw physical disk is an asynchronous
// device -- requests to read or write portions of the disk (sectors)
// return immediately, and an interrupt occurs later to signal that the
// operation completed.  (Also, the physical characteristics of the
// disk device assume that only one operation can be requested at a
// time).
//
// This class provides the abstraction that for any individual thread
// making a request, it waits around until the operation finishes before
// returning.
*/
class DriverDisk {
public:
  DriverDisk(char *sem_name, char *lock_name, Disk *theDisk);
  // Constructor. Initializes the disk
  // driver by initializing the raw Disk.
  ~DriverDisk();   // Destructor. De-allocate the driver data

  void ReadSector(uint32_t sectorNumber, char *data);
  // Read/write a disk sector, returning
  // only once the data is actually read
  // or written.
  void WriteSector(uint32_t sectorNumber, char *data);

  void RequestDone();   // Called by the disk device interrupt
                        // handler, to signal that the
                        // current disk operation is complete.

private:
  Semaphore *semaphore; /*!< To synchronize requesting thread
                             with the interrupt handler
                        */
  Lock *lock;           /*!< Mutual exclusion on the disk device
                         */
  Disk *disk;           /* The disk */
};

void DiskRequestDone();
void DiskSwapRequestDone();

#endif   // SYNCHDISK_H
