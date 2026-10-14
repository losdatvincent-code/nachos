/*! \file directory.cc
//  \brief Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
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

#include "filesys/directory.h"
#include "filesys/filehdr.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "utility/config.h"
#include "utility/utility.h"

//----------------------------------------------------------------------
// Directory::Directory
/*! 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	\param size is the number of entries in the directory
*/
//----------------------------------------------------------------------
Directory::Directory(int size) {
  table = new DirectoryEntry[size];
  tableSize = size;
  for (int i = 0; i < tableSize; i++) {
    table[i].inUse = false;
  }
}

//----------------------------------------------------------------------
// Directory::~Directory
//! 	De-allocate directory data structure.
//----------------------------------------------------------------------
Directory::~Directory() { delete[] table; }

//----------------------------------------------------------------------
// Directory::FetchFrom
/*! 	Read the contents of the directory from disk.
//
//	\param file is the file containing the directory contents
*/
//----------------------------------------------------------------------
void
Directory::FetchFrom(OpenFile *file) {
  (void) file->ReadAt((char *) table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
/*! 	Write any modifications to the directory back to disk
//
//	\param file is the file to contain the new directory contents
*/
//----------------------------------------------------------------------
void
Directory::WriteBack(OpenFile *file) {
  (void) file->WriteAt((char *) table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
/*! 	Look up file name in directory.
//
//      \return   its location in the table of directory entries,
//                ERROR if the name isn't in the directory.
//
//	\param name the file name to look up
*/
//----------------------------------------------------------------------
int
Directory::FindIndex(char *name) {
  for (int i = 0; i < tableSize; i++)
    if (table[i].inUse && !strncmp(table[i].name, name, FILENAMEMAXLEN))
      return i;
  return ERROR;   // name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
/*! 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return ERROR if the name isn't
//	in the directory.
//
//      \return the disk sector number where the file's header is stored
//              or ERROR if the name isn't in the directory.
//
//	\param name the file name to look up
*/
//----------------------------------------------------------------------
int
Directory::Find(char *name) {
  int i = FindIndex(name);
  if (i != ERROR)
    return table[i].sector;
  return ERROR;
}

//----------------------------------------------------------------------
// Directory::Add
/*! 	Add a file into the directory.
//
//	\param name the name of the file being added
//	\param newSector the disk sector containing the added file's header
//      \return NO_ERROR, ALREADY_IN_DIRECTORY or NOSPACE_IN_DIRECTORY.
*/
//----------------------------------------------------------------------
int
Directory::Add(char *name, int newSector) {
  if (FindIndex(name) != ERROR)
    return ALREADY_IN_DIRECTORY;

  for (int i = 0; i < tableSize; i++) {
    if (!table[i].inUse) {
      table[i].inUse = true;
      strncpy(table[i].name, name, FILENAMEMAXLEN);
      table[i].sector = newSector;
      return NO_ERROR;
    }
  }

  // no space.  Fix when we have extensible files.
  return NOSPACE_IN_DIRECTORY;
}

//----------------------------------------------------------------------
// Directory::Remove
/*! 	Remove a file name from the directory.
//
//	\param name the file name to be removed
//      \return NO_ERROR, or INEXIST_DIRECTORY_ERROR
*/
//----------------------------------------------------------------------
int
Directory::Remove(char *name) {
  int i = FindIndex(name);
  if (i == ERROR)
    return INEXIST_DIRECTORY_ERROR;   // name not in directory
  table[i].inUse = false;
  return NO_ERROR;
}

//----------------------------------------------------------------------
// Directory::List
/*! 	List all the file names in the directory.(recursive function)
 *
 *   \param name the name of the Dir to print
 *   \param depth the depth in the recursion (to print a nice
 *          hierarchy with spaces)
 */
//----------------------------------------------------------------------
void
Directory::List(char *name, int depth) {
  Directory dir(gConfig->NumDirEntries);

  for (int i = 0; i < tableSize; i++) {
    if (table[i].inUse) {

      /* Print a nice Tree branch dependent on the depth in the
       * recursion, like in:
       * " +-- toto"
       */
      for (int j = 0; j < depth; j++) {
        if (j < depth - 3)
          printf(" ");
        if (j == depth - 3)
          printf("+");
        if (j > depth - 3)
          printf("-");
      }
      printf("%s", table[i].name);

      OpenFile file(table[i].sector);
      if (file.IsDir()) {
        printf("(D)\n");
        char dirname[gConfig->MaxFileNameSize];
        strcpy(dirname, name);
        strcat(dirname, table[i].name);
        dir.FetchFrom(&file);
        dir.List(dirname, depth + 4);
      } else
        printf("\n");
    }
  }
}

//----------------------------------------------------------------------
// Directory::Print
/*! 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
*/
//----------------------------------------------------------------------
void
Directory::Print() {
  FileHeader hdr;

  printf("Directory contents:\n");
  for (int i = 0; i < tableSize; i++) {
    if (table[i].inUse) {
      printf("Name: %s, Sector: %" PRIu32 "\n", table[i].name, table[i].sector);
      hdr.FetchFrom(table[i].sector);
      hdr.Print();
    }
  }
  printf("\n");
}

//----------------------------------------------------------------------
// Directory::empty
/*! 	Tests if a directory is empty.
//	\return true if the directory is empty.
*/
//----------------------------------------------------------------------
bool
Directory::empty() {
  bool empti = true;
  for (int i = 0; i < tableSize; i++) {
    if (table[i].inUse) {
      return false;
    }
  }
  return empti;
}
