/*! \file openfile.h
    \brief Data structures to manipulate files

        Data structures for opening, closing, reading and writing to
        individual files.  The operations supported are similar to
        the UNIX ones -- type 'man open' to the UNIX prompt.

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

#ifndef OPENFILE_H
#define OPENFILE_H

#include "kernel/copyright.h"
#include "kernel/system.h"
#include "utility/utility.h"

class FileHeader;

/*!  \brief Defines the data structure maintained when a file is opened
//
//	This is the "real" implementation, that turns these
//	operations into read and write disk sector requests.
//	In this baseline implementation of the file system, we don't
//	worry about concurrent accesses to the file system
//	by different threads -- this is part of the assignment.
*/
class OpenFile {
public:
  /*! Open a file whose header is located
     at "sector" on the disk
  */
  OpenFile(int sector);

  //! Close the file
  ~OpenFile();

  /*! Set the position from which to
     start reading/writing -- UNIX lseek
  */
  void Seek(int position);

  /*! Read/write bytes from the file,
     starting at the implicit position.
     Return the # actually read/written,
     and increment position in file.
  */
  int Read(char *into, int numBytes);
  int Write(char *from, int numBytes);

  /*! Read/write bytes from the file,
     bypassing the implicit position.
  */
  int ReadAt(char *into, int numBytes, int position);
  int WriteAt(char *from, int numBytes, int position);

  int Length();                  /*!< Return the number of bytes in the
                                    file (this interface is simpler
                                    than the UNIX idiom -- lseek to
                                    end of file, tell, lseek back
                                 */
  FileHeader *GetFileHeader();   //!< return the file's header

  char *GetName();   //!< return the file's name

  void SetName(char *);   //!< Set the file's name

  bool IsDir();   //!< return true if the file is a directory
private:
  char *name;         //!< the file's name.
  FileHeader *hdr;    //!< Header for this file
  int seekPosition;   //!< Current position within the file
  int fSector;        //!< The file's first sector

public:
  //! Object type, for validity checks during system calls (must be the first
  //! public field)
  ObjectType type;
};

#endif   // OPENFILE_H
