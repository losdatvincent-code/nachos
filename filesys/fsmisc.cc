/*! \file fsmisc.cc
//  \brief Miscellaneous routines for the Nachos file system
//
//	We implement:
//	   Copy -- copy a file from UNIX to Nachos
//	   Print -- cat the contents of a Nachos file
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

#include "filesys/filesys.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "utility/stats.h"
#include "utility/utility.h"

#define TransferSize 10   // make it small (10), just to be difficult

//----------------------------------------------------------------------
// Copy
// 	Copy the contents of the UNIX file "from" to the Nachos file "to"
//----------------------------------------------------------------------
void
Copy(char *from, char *to) {
  FILE *fp;
  OpenFile *openFile = NULL;
  int amountRead, fileLength;
  // Open UNIX file
  if ((fp = fopen(from, "r")) == NULL) {
    printf("Copy: couldn't open Unix file %s\n", from);
    exit(ERROR);
    return;
  }

  // Figure out length of UNIX file
  fseek(fp, 0, 2);
  fileLength = ftell(fp);
  fseek(fp, 0, 0);

  // Create a Nachos file of the same length
  printf("Copying Unix file %s to Nachos file %s\n", from, to);
  if (gFileSystem->Create(to, fileLength) !=
      NO_ERROR) {   // Create Nachos file
    printf("Copy: couldn't create Nachos file %s\n", to);
    fclose(fp);
    exit(ERROR);
    return;
  }
  openFile = gFileSystem->Open(to);
  ASSERT(openFile != NULL);

  // Copy the data in TransferSize chunks
  char buffer[TransferSize];
  while ((amountRead = fread(buffer, sizeof(char), TransferSize, fp)) > 0)
    openFile->Write(buffer, amountRead);

  // Close the UNIX and the Nachos files
  delete openFile;
  fclose(fp);
}

//----------------------------------------------------------------------
// Print
// 	Print the contents of the Nachos file "name".
//----------------------------------------------------------------------
void
Print(char *name) {
  OpenFile *openFile;
  int i, amountRead;

  if ((openFile = gFileSystem->Open(name)) == NULL) {
    printf("Print: unable to open Nachos file %s\n", name);
    return;
  }

  char buffer[TransferSize];
  while ((amountRead = openFile->Read(buffer, TransferSize)) > 0) {
    for (i = 0; i < amountRead; i++) {
      printf("%1x ", buffer[i]);
    }
  }

  delete openFile;   // close the Nachos file
  return;
}
