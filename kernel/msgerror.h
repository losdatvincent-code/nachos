/*! \file msgerror.h
    \brief Defines the data structure to store error messages during
           a syscall process
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

#ifndef MSGERROR_H
#define MSGERROR_H

// Forward declarations
class SyscallError;
class DriverConsole;

// ERROR error code for syscalls
#define ERROR -1

// Markers for invalid addresses and sectors
#define INVALID_ADDRESS ((uint64_t) -1)
#define INVALID_PAGE    -1
#define INVALID_SECTOR  ((uint32_t) -1)

//! List of error message numbers
//! (see their meaning in msgerror.cc)
enum {
  NO_ERROR = 0,
  INC_ERROR,
  OPENFILE_ERROR,
  EXEC_FILE_FORMAT_ERROR,
  OUT_OF_MEMORY,

  OUT_OF_DISK,
  ALREADY_IN_DIRECTORY,
  INEXIST_FILE_ERROR,
  INEXIST_DIRECTORY_ERROR,
  NOSPACE_IN_DIRECTORY,
  NOT_A_FILE,
  NOT_A_DIRECTORY,
  DIRECTORY_NOT_EMPTY,
  INVALID_COUNTER,

  /* Invalid typeId fields: */
  INVALID_SEMAPHORE_ID,
  INVALID_LOCK_ID,
  INVALID_CONDITION_ID,
  INVALID_FILE_ID,
  INVALID_THREAD_ID,

  /* Other messages */
  WRONG_FILE_ENDIANESS,
  NO_ACIA,

  NUMMSGERROR /* Must always be last */
};

/*! \brief Defines a structure to store syscall error messages
//
//
//  When an error occurs during a system call, a negative value
//  is return to the user program and then an error message
//  can be printed using the system call PError(). This structure
//  contains the last error message with the corresponding context
//  string (eg. the name of an unknow file).
*/
class SyscallError {
public:
  SyscallError();    // Initialize the structure
  ~SyscallError();   // De-allocate the structure

  void SetMsg(char *about, int num);
  //!< Set the current error message

  void PrintLastMsg(DriverConsole *cons, char *ch);
  //!< Print the error message with a user defined
  //!< string

  //! Get the error format string (with a %s) associated with error num
  const char *GetFormat(int num);

private:
  int lastError;             //!< last error's ident
  char *errorAbout;          //!< context string
  char *msgs[NUMMSGERROR];   //!< The array of strings for the error messages
};

#endif   // MSGERROR_H
