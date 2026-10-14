/*! \file oftable.cc
// \brief Routines for managing the open file table.
//
// The open file table is used to synchronize all the
// access to the files. When a file is open, every read
// or write will use this class synchronisation methods
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

#include "filesys/oftable.h"
#include "filesys/directory.h"
#include "filesys/filesys.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "utility/bitmap.h"
#include <string.h>

//----------------------------------------------------------
// OpenFileTableEntry::OpenFileTableEntry()
/*! initialize an open file table entry.
 */
//----------------------------------------------------------
OpenFileTableEntry::OpenFileTableEntry() {
  name = new char[gConfig->MaxFileNameSize];
  numthread = 1;
  ToBeDeleted = false;
  lock = new Lock((char *) "File Synchronisation");
  file = NULL;
  sector = INVALID_SECTOR;
}

//----------------------------------------------------------
// OpenFileTableEntry::~OpenFileTableEntry()
/*! Delete an entry and delete the file from
// the file system if the boolean ToBeDeleted is
// set to true.
*/
//----------------------------------------------------------
OpenFileTableEntry::~OpenFileTableEntry() {
  if (ToBeDeleted) {
    // Get the freemap from disk
    BitMap freeMap(NUM_SECTORS);
    freeMap.FetchFrom(gFileSystem->GetFreeMapFile());

    // Indicate that some sectors are freed due to the file deletion
    file->GetFileHeader()->Deallocate(&freeMap);
    freeMap.Clear(sector);
    // Write the freemap back to disk
    freeMap.WriteBack(gFileSystem->GetFreeMapFile());
  }
  delete[] name;
  delete file;
  delete lock;
}

//----------------------------------------------------------
// OpenFileTable::OpenFileTable()
/*! initialize the open file table.
 */
//----------------------------------------------------------
OpenFileTable::OpenFileTable() {
  createLock = new Lock((char *) "Creation Synch");
  for (int i = 0; i < NBOFTENTRY; i++) {
    table[i] = NULL;
  }
  nbentry = 0;
}
//----------------------------------------------------------
// OpenFileTable::~OpenFileTable()
/*! initialize the open file table.
 */
//----------------------------------------------------------
OpenFileTable::~OpenFileTable() { delete createLock; }

//----------------------------------------------------------
// void OpenFileTable::Open(char *name,Openfile *file)
/*! check if the file is already open
// and if not creates a new entry in
// the table.
//
// \return the open file
// \param name is the name of the file
// \param file is an Openfile object to strore in the table
*/
//----------------------------------------------------------
OpenFile *
OpenFileTable::Open(char *name) {
  OpenFile *newfile = NULL;
  uint32_t num, sector, dirsector;
  char filename[gConfig->MaxFileNameSize];

  // Find the file in the open file table
  num = findl(name);
  DEBUG('f', (char *) "opening file %s\n", name);
  if (num != ERROR) {
    // The file is opened by another thread
    if (!table[num]->ToBeDeleted) {
      // Update the reference count and return an OpenFile
      table[num]->numthread++;
      newfile = new OpenFile(table[num]->sector);
      newfile->SetName(name);
      DEBUG('f', (char *) "File %s was in the table\n", name);
      return newfile;
    } else
      return NULL;
  } else {
    if (nbentry != (uint32_t) ERROR) {   // there is some place in the table
      OpenFileTableEntry *entry = new OpenFileTableEntry;
      OpenFile *openfile = NULL;
      Directory directory(gConfig->NumDirEntries);

      strcpy(entry->name, name);
      strcpy(filename, name);

      // Find the directory containing the file and read it from the disk
      dirsector = FindDir(filename);
      if (dirsector == INVALID_SECTOR)
        return NULL;
      OpenFile dirfile(dirsector);
      directory.FetchFrom(&dirfile);

      // Find the file in the directory
      sector = directory.Find(filename);
      if (sector >= 0) {
        openfile = new OpenFile(sector);   // name was found in directory
        if (openfile->IsDir()) {           // name is a directory ...
          delete openfile;
          delete entry;
          return NULL;
        }
      } else {   // name isn't in directory
        delete entry;
        return NULL;
      }

      // We found the file
      newfile = new OpenFile(sector);   // we fill the new entry
      newfile->SetName(name);
      openfile->SetName(name);
      entry->sector = sector;
      entry->file = openfile;
      table[nbentry] = entry;
      nbentry = next_entry();

      DEBUG('f', (char *) "File %s has been opened successfully\n", name);
      return newfile;
    } else {
      printf("OFT OPEN: File %s cannot be opened ", name);
      return NULL;
    }
  }
}

//----------------------------------------------------------
// void OpenFileTable::Close(char *name)
/*! called when a thread closes a file : this method
// decrease numthread and if it is null then the entry is
// deleted.
// \param name is the name of the file to close
*/
//----------------------------------------------------------
void
OpenFileTable::Close(char *name) {
  uint32_t num;
  DEBUG('f', (char *) "Closing File %s \n", name);
  num = findl(name);
  if (num != (uint32_t)ERROR) {          // the file is in the table
    table[num]->numthread--;   // the thread has no longer this file opened
    if (table[num]->numthread <= 0) {   // if no threads has this file opened
      DEBUG('f', (char *) "File %s is no more in the table\n", name);
      delete table[num];   // then remove it from the table
      table[num] = NULL;
    }
    DEBUG('f', (char *) "File %s has been closed successfully\n", name);
  }
}

//----------------------------------------------------------
// void OpenFileTable::Lock(char *name)
/*! Lock the access to a file. It is used to
// synchronise reads and writes.
//
// \param name is the name of the file we want to lock
*/
//----------------------------------------------------------
void
OpenFileTable::FileLock(char *name) {
  int num;
  num = findl(name);
  if (num != ERROR) {
    table[num]->lock->Acquire();
    DEBUG('f', (char *) "File %s has been locked\n", name);
  }
}

//----------------------------------------------------------
// void OpenFileTable::Release(char *name
/*!release the lock after the disk operation
//
// \param name is the name of the file we want to unlock
*/
//----------------------------------------------------------
void
OpenFileTable::FileRelease(char *name) {
  int num;
  num = findl(name);
  if (num != ERROR) {
    table[num]->lock->Release();
    DEBUG('f', (char *) "File %s has been released\n", name);
  }
}
//----------------------------------------------------------
// int OpenFileTable::findl(char *name)
/*!find a file in the table return
//
// \return ERROR if the file is not in the table
//         or its place in the table if it was already opened
// \param name is the name of the file we want to find
*/
//----------------------------------------------------------
int
OpenFileTable::findl(char *name) {
  int i = 0;
  while (i < NBOFTENTRY) {
    if (table[i] != NULL)
      if (strcmp(table[i]->name, name) == 0)
        return i;
    i++;
  }
  return ERROR;
}

//----------------------------------------------------------
// bool OpenFileTable::Remove(char *name
/*! remove the file from the directory  and put ToBeDeleted
//  to true. after removing a file nobody can open the
//  deleted file but every thread wich has it open can
//  access the datas. The data on disk will be deleted
//  only after every thread had closed this file.
//
// \return NO_ERROR if the file has been successfuly removed
// \param name is the name of the file we want to delete
*/
//----------------------------------------------------------
int
OpenFileTable::Remove(char *name) {
  Directory directory(gConfig->NumDirEntries);
  uint32_t num, sector, dirsector;
  char filename[gConfig->MaxFileNameSize];

  DEBUG('f', (char *) "Removing file %s\n", name);
  strcpy(filename, name);

  // Find the directory containing the file
  dirsector = FindDir(filename);
  if (dirsector == INVALID_SECTOR)
    return INEXIST_FILE_ERROR;

  // Fetch it from disk
  OpenFile dirfile(dirsector);
  directory.FetchFrom(&dirfile);
  sector = directory.Find(filename);
  if (sector == INVALID_SECTOR)
    return INEXIST_FILE_ERROR;   // file not found

  // Scan the open file table
  num = findl(name);
  if (num != ERROR) {   // file is opened by a thread
    table[num]->ToBeDeleted = true;
    directory.Remove(filename);
    directory.WriteBack(&dirfile);
  } else {   // file isn't opened
    return (gFileSystem->Remove(name));
  }
  return NO_ERROR;
}
//----------------------------------------------------------
// int OpenFileTable::next_entry()
/*! this function looks in the table where
//  it is possible to add a new entry
//
// \return the next valid entry in the table
*/
//----------------------------------------------------------
int
OpenFileTable::next_entry() {
  int i = 0;
  while (i < NBOFTENTRY) {
    if (table[i] == NULL)
      return i;
    i++;
  }
  return ERROR;
}
