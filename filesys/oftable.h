/*! \file oftable.h
   \brief Data structure used to synchronize operations on files.

   This data structure maintain a list of all the files open in the
   system. It keep track of all of the files currently open. When a
   new thread opens a file, the open table would be checked to see
   if any other thread already has it open. The processes still
   maintain a private copy of the openfile objects. The file table
   is only used to synchronise accesses to these files.

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

#ifndef FS_OFT
#define FS_OFT

#include "filesys/filehdr.h"
#include "filesys/openfile.h"
#include "kernel/synch.h"

// the max number of file nachos can open at the
// same time.
#define NBOFTENTRY 15

/*! \brief defines the structure of a record in the open file table
 */
class OpenFileTableEntry {
public:
  char *name;       //!< the name of the file
  OpenFile *file;   //!< open file descriptor
  int numthread;    //!< number of thread that has this file open
  Lock *lock;       //!< used to synchronize file access
  bool ToBeDeleted; /*!< true if the file has to be deleted
                      when every thread will close it.
                    */
  int sector;       //!< the disc sector where is located the fileheader
  OpenFileTableEntry();
  ~OpenFileTableEntry();   // delete the file if necessary
};

/*! \brief Defines a list of all opened files
//  the class OpenFileTable maintain a list of all the open files
//  in nachos and provides synchronisation between several
//  threads wich use the same file.
*/
class OpenFileTable {
public:
  OpenFileTable();   // initialize the open file table
  ~OpenFileTable();

  OpenFile *Open(char *name); /*!< check if the file is already open
                                   and if not creates a new entry in
                                   the table
                               */
  void Close(char *name);     /*!< decrease numthread and if numthread
                                is 0 then remove the file from
                                the open file table
                              */
  void FileLock(char *name);  /*!< lock the file name to implement
                               atomic write
                             */
  void
  FileRelease(char *name);   //!< release the lock after the disk operation.

  int Remove(char *name);   //!< remove the file from the file system

  int next_entry();   //!< this function give the next valid entry in the table
  Lock *createLock;

private:
  OpenFileTableEntry *table[NBOFTENTRY];   //!< the list of open files
  int nbentry;             //!< the number of the next valid entry in the table
  int findl(char *name);   // find a file in the table
};

#endif   // FS_OFT
