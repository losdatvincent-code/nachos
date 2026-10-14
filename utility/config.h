/*! \file config.h
    \brief Data structures for setting up the Nachos hardware and
        software configuration
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
#ifndef CONFIG_H
#define CONFIG_H

#include "filesys/filesys.h"
#include "machine/translationtable.h"
#include <stdio.h>

/* Maximum length of strings in Nachos + max nb of files to copy */
#define MAXSTRLEN 256
#define MAXCOPIES 100

/* Default name of configuration file */
#define CONFIGFILENAME "nachos.cfg"

/* Running modes of the ACIA */
#define ACIA_NONE         0
#define ACIA_BUSY_WAITING 1
#define ACIA_INTERRUPT    2

/*! \brief Defines Nachos hardware and software configuration
 *
 * Used to avoid recompiling Nachos when a change in the configuration
 * is needed (turning on/of debug flags, changing memory size, etc.).
 * There is a default value for every configuration parameter when not
 * specified in the configuration file (see file config.cc)
 */
class Config {

public:
  // Hardware configuration
  uint32_t PageSize;       //!< Page size in bytes
  uint64_t NumPhysPages;   //!< Number of pages in the memory of the simulated
                           //!< MIPS machine
  uint32_t SectorSize;   //!< Disk sector size in bytes (should be equal to the
                         //!< page size)
  uint32_t ProcessorFrequency;   //!< Frequency of the processor (MHz) used for
                                 //!< having statistics
  uint32_t DiskSize;             //!< Total size of the disk (number of sectors)
  uint8_t ACIA;   //!< Use ACIA if USE_ACIA, don't use it if ACIA_NONE

  // File system configuration
  uint32_t NumDirect;   //!< Number of data sectors storable in the first header
                        //!< sector
  uint32_t MaxFileSize;       //!< Maximum length of a file
  uint32_t MaxFileNameSize;   //!< Maximum length of a file name (absolute, path
                              //!< included)
  uint32_t NumDirEntries;     //!< Maximum number of files in a directory
  uint32_t DirectoryFileSize;          //!< Length of a directory file
  uint32_t NumPortLoc;                 //!< Local ACIA's port number
  uint32_t NumPortDist;                //!< Distant ACIA's port number
  char TargetMachineName[MAXSTRLEN];   //!< The name of the target machine for
                                       //!< the ACIA

  // Kernel (process and address space) configuration
  uint64_t
      MaxVirtPages;   //!< Maximum number of virtual pages in each address space
  bool TimeSharing;   //!< Use the time sharing mode if true (1) - not
                      //!< implemented in the base code
  uint32_t MagicNumber;     //!< 0x456789ab
  uint32_t MagicSize;       //!< Size of an integer
  uint32_t UserStackSize;   //!< Stack size of user threads in bytes

  // Configuration of actions to be done when Nachos is started and exited
  uint32_t NbCopy;      //!< Number of files to copy
  bool ListDir;         //!< List all the files and directories if true
  bool PrintFileSyst;   //!< Print all the files in the file system if true
  bool PrintStat;       //!< Print the statistics if true
  bool FormatDisk;      //!< Format the disk if true
  bool Print;           //!< Print  FileToPrint if true
  bool Remove;          //!< Remove FileToRemove if true
  bool MakeDir;         //!< Make DirToMake if true
  bool RemoveDir;       //!< remove DirToRemove if true
  char ToCopyUnix[MAXCOPIES][MAXSTRLEN];   //!< The table of files to copy from
                                           //!< the UNIX filesystem
  char ToCopyNachos[MAXCOPIES][MAXSTRLEN];   //!< The table of files to copy to
                                             //!< the nachos filesystem
  char ProgramToRun[MAXSTRLEN];   //!< The name of the program to execute
  char FileToPrint[MAXSTRLEN];    //!< The name of the file to print
  char FileToRemove[MAXSTRLEN];   //!< The name of the file to remove
  char DirToMake[MAXSTRLEN];      //!< The name of the directory to make
  char DirToRemove[MAXSTRLEN];    //!< The name of the directory to remove

  /**
   * Fill-in the configuration object from configuration information stored in a
   * file
   *
   * \param configname: file name of the configuration file
   */
  Config(char *configname);

  /**
   * Destructor
   */
  ~Config() { ; }

private:
  /** File descriptor of the configuration file */
  FILE *cfg;
};

#endif   // CONFIG_H
