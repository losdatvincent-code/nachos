/*! \file drvDisk.cc
//  \brief Routines to synchronously access the disk
//
//      The physical disk
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
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

#include "drivers/drvDisk.h"

//----------------------------------------------------------------------
// DiskRequestDone
/*! 	Disk interrupt handler.  Need this to be a C routine, because
//	C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------

void
DiskRequestDone() {
  gDiskDriver->RequestDone();
}

//----------------------------------------------------------------------
// DiskSwapRequestDone
/*! 	Disk Swap interrupt handler.  Need this to be a C routine, because
//	C++ can't handle pointers to member functions.
*/
//----------------------------------------------------------------------

void
DiskSwapRequestDone() {
  gSwapDiskDriver->RequestDone();
}

//----------------------------------------------------------------------
// DriverDisk::DriverDisk
/*! 	Constructor.
//      Initialize the disk driver, in turn
//	initializing the physical disk.
*/
//----------------------------------------------------------------------

DriverDisk::DriverDisk(char *sem_name, char *lock_name, Disk *theDisk) {
  semaphore = new Semaphore((char *) sem_name, 0);
  lock = new Lock((char *) lock_name);
  disk = theDisk;
}

//----------------------------------------------------------------------
// DriverDisk::~DriverDisk
/*! 	Destructor.
//      De-allocate data structures needed for the disk driver.
*/
//----------------------------------------------------------------------

DriverDisk::~DriverDisk() {
  delete lock;
  delete semaphore;
}

//----------------------------------------------------------------------
// DriverDisk::ReadSector
/*! 	Read the contents of a disk sector into a buffer. Return only
//	after the data has been read.
//
//	\param sectorNumber the disk sector to read
//	\param data the buffer to hold the contents of the disk sector
*/
//----------------------------------------------------------------------

void
DriverDisk::ReadSector(uint32_t sectorNumber, char *data) {
  DEBUG('d', (char *) "[sdisk] rd req\n");
  lock->Acquire();   // only one disk I/O at a time
  disk->ReadRequest(sectorNumber, data);
  DEBUG('d', (char *) "[sdisk] rd req: wait irq\n");
  semaphore->P();   // wait for interrupt
  DEBUG('d', (char *) "[sdisk] rd req: wait irq OK\n");
  lock->Release();
}

//----------------------------------------------------------------------
// DriverDisk::WriteSector
/*! 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	\param sectorNumber  the disk sector to be written
//	\param data  the new contents of the disk sector
*/
//----------------------------------------------------------------------

void
DriverDisk::WriteSector(uint32_t sectorNumber, char *data) {
  DEBUG('d', (char *) "[sdisk] wr req\n");
  lock->Acquire();   // only one disk I/O at a time
  disk->WriteRequest(sectorNumber, data);
  DEBUG('d', (char *) "[sdisk] wr req: wait irq...\n");
  semaphore->P();   // wait for interrupt
  DEBUG('d', (char *) "[sdisk] wr req: wait irq OK\n");
  lock->Release();
}

//----------------------------------------------------------------------
// DriverDisk::RequestDone
/*! 	Disk interrupt handler. Wake up any thread waiting for the disk
//	request to finish.
*/
//----------------------------------------------------------------------

void
DriverDisk::RequestDone() {
  DEBUG('d', (char *) "[sdisk] req done\n");
  semaphore->V();
}
