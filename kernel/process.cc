/*! \file  process.cc
//  \brief Routines to manage processes
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

#include "kernel/process.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"

//----------------------------------------------------------------------
// Process::Process
/*! 	Constructor. Create the environment to run a user program
//      (address space, statistics, ...)
//
//	\param executable is the file containing the object code
//             to load into memory
//      \param err: error code 0 if OK, -1 otherwise
*/
//----------------------------------------------------------------------
Process::Process(char *filename, uint64_t *err) {
  numThreads = 0;
  *err = NO_ERROR;
  if (filename == NULL) {
    DEBUG('t', (char *) "Create empty process\n");

    // Create a statistics object for the program
    stat = gStats->NewProcStat((char *) "BOOT");

    // Fake process Name
    name = new char[strlen("BOOT") + 1];
    strcpy(name, "BOOT");

    // No Executable to open
    elfFile = NULL;
  } else {
    DEBUG('t', (char *) "Create named process %s\n", filename);

    // Create a statistics object for the program
    stat = gStats->NewProcStat(filename);

    // Set process name
    name = new char[strlen(filename) + 1];
    strcpy(name, filename);

    // Open executable
    elfFile = gFileSystem->Open(filename);
    if (elfFile == NULL) {
      // NB : don't delete the stat object, so that statistics can
      // be displayed after the end of the process
      *err = INEXIST_FILE_ERROR;
      return;
    }
  }

  // Create the new address space associated with this file
  addrspace = new AddrSpace(elfFile, this, err);
  if (*err != NO_ERROR) {
    delete addrspace;
    // NB : don't delete the stat object, so that statistics can
    // be displayed after the end of the process
    delete[] name;
    return;
  }
}

//----------------------------------------------------------------------
// Process::~Process
//!   Destructor. De-alloate a process and all its components
//      (address space, ...)
//----------------------------------------------------------------------
Process::~Process() {
  ASSERT(numThreads == 0);

  // Delete the address space. Done for all processes, even the one created
  // for startup, for which there is no executable file attached
  delete addrspace;

  // Delete program name
  delete[] name;

  if (elfFile != NULL) {
    if (elfFile)
      delete elfFile;
    // NB : don't delete the stat object, so that statistics can
    // be displayed after the end of the process
  }
}
