/*! \file config.cc
    \brief Routines for setting up the Nachos hardware and software
 confuguration
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
#ifndef CONFIG_CPP
#define CONFIG_CPP

#include "utility/config.h"
#include "filesys/directory.h"
#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "machine/machine.h"
#include "utility/utility.h"
#include <stdio.h>

#define power_of_two(size) (((size) & ((size) -1)) == 0)

void
fail(uint32_t numligne, char *name, char *ligne) {
  ligne[strlen(ligne) - 1] = '\0';
  printf("Config Error : File %s line %d ---> \"%s\"\n", name, numligne, ligne);
  exit(ERROR);
}

/**
 * Fill-in the configuration object from configuration information stored in a
 * file
 *
 * \param configname: file name of the configuration file
 */
Config::Config(char *configname) {

  // Default values for configuration parameters
  SectorSize = 128;
  PageSize = 128;
  NumPhysPages = 20;
  MaxVirtPages = 1024;
  UserStackSize = 8 * 1024;
  ProcessorFrequency = 100;
  MaxFileNameSize = 256;
  NbCopy = 0;
  NumDirEntries = 10;
  NumPortLoc = 32009;
  NumPortDist = 32009;
  PrintStat = false;
  FormatDisk = false;
  ListDir = false;
  PrintFileSyst = false;
  Print = false;
  Remove = false;
  MakeDir = false;
  RemoveDir = false;
  ACIA = ACIA_NONE;
  strcpy(ProgramToRun, "");

  uint32_t nblignes = 0;

  DEBUG('u', (char *) "Reading the configuration file\n");
  char ligne[MAXSTRLEN];
  char commande[MAXSTRLEN];

  cfg = fopen(configname, "r");

  if (cfg == (FILE *) NULL) {
    printf("Error: can't open file %s\n", configname);
    exit(ERROR);
  }

  while (!feof(cfg)) {

    fgets(ligne, MAXSTRLEN, cfg);
    nblignes++;
    if ((ligne[0] != '#') && (strlen(ligne) != 0)) {

      // Accepted NULL lines
      if (strcmp(ligne, "") == 0)
        continue;
      if (strcmp(ligne, "\n") == 0)
        continue;

      if (sscanf(ligne, " %s ", commande) != 1)
        fail(nblignes, configname, ligne);
      if (strlen(commande) != 0) {

        if (strcmp(commande, "ProcessorFrequency") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande,
                     &ProcessorFrequency) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "NumPhysPages") == 0) {
          if (sscanf(ligne, " %s = %" PRIu64 " ", commande, &NumPhysPages) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "MaxVirtPages") == 0) {
          if (sscanf(ligne, " %s = %" PRIu64 " ", commande, &MaxVirtPages) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }
        if (strcmp(commande, "SectorSize") == 0) {
          if (sscanf(ligne, " %s = %i ", commande, &SectorSize) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }
        if (strcmp(commande, "PageSize") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &PageSize) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }
        if (strcmp(commande, "UserStackSize") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &UserStackSize) !=
              2)
            fail(nblignes, configname, ligne);
          continue;
        }
        if (strcmp(commande, "MaxFileNameSize") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &MaxFileNameSize) !=
              2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "TargetMachineName") == 0) {
          if (sscanf(ligne, " %s = %s ", commande, TargetMachineName) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "ProgramToRun") == 0) {
          if (sscanf(ligne, " %s = %s ", commande, ProgramToRun) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "PrintStat") == 0) {
          uint32_t v;
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &v) == 2) {
            PrintStat = (bool) v;
            if (v == 0)
              PrintStat = false;
            else
              PrintStat = true;
          } else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "FormatDisk") == 0) {
          uint32_t v;
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &v) == 2) {
            if (v == 0)
              FormatDisk = false;
            else
              FormatDisk = true;
          } else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "ListDir") == 0) {
          uint32_t v;
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &v) == 2) {
            if (v == 0)
              ListDir = false;
            else
              ListDir = true;
          } else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "PrintFileSyst") == 0) {
          uint32_t v;
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &v) == 2) {
            if (v == 0)
              PrintFileSyst = false;
            else
              PrintFileSyst = true;
          } else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "FileToCopy") == 0) {
          if (sscanf(ligne, " %s = %s %s", commande, ToCopyUnix[NbCopy],
                     ToCopyNachos[NbCopy]) == 3)
            NbCopy++;
          else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "FileToPrint") == 0) {
          if (sscanf(ligne, " %s = %s ", commande, FileToPrint) == 2)
            Print = true;
          else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "FileToRemove") == 0) {
          if (sscanf(ligne, " %s = %s ", commande, FileToRemove) == 2)
            Remove = true;
          else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "DirToMake") == 0) {
          if (sscanf(ligne, " %s = %s ", commande, DirToMake) == 2)
            MakeDir = true;
          else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "DirToRemove") == 0) {

          if (sscanf(ligne, " %s = %s ", commande, DirToRemove) == 2)
            RemoveDir = true;
          else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "NumDirEntries") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &NumDirEntries) !=
              2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "UseACIA") == 0) {
          char acia_mode[MAXSTRLEN];
          if (sscanf(ligne, " %s = %s ", commande, acia_mode) == 2) {
            if (strcmp(acia_mode, "None") == 0)
              ACIA = ACIA_NONE;
            else if (strcmp(acia_mode, "BusyWaiting") == 0)
              ACIA = ACIA_BUSY_WAITING;
            else if (strcmp(acia_mode, "Interrupt") == 0)
              ACIA = ACIA_INTERRUPT;
            else
              fail(nblignes, configname, ligne);
          } else
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "NumPortLoc") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &NumPortLoc) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }

        if (strcmp(commande, "NumPortDist") == 0) {
          if (sscanf(ligne, " %s = %" PRIu32 " ", commande, &NumPortDist) != 2)
            fail(nblignes, configname, ligne);
          continue;
        }

        // Autres variables -> non reconnues
        fail(nblignes, configname, commande);
      }
      strcpy(commande, "\0");
    }
  }

  fclose(cfg);

  if (PageSize != SectorSize) {
    printf("Warning, PageSize<>SectorSize, setting both to %" PRIu32 "\n",
           SectorSize);
    PageSize = SectorSize;
  }

  // Check that sector size and page sizes are powers of two
  if (!power_of_two(SectorSize)) {
    printf(
        "Configuration error : SectorSize should be a power of two, exiting\n");
    exit(ERROR);
  }

  NumDirect = ((SectorSize - 4 * sizeof(uint32_t)) / sizeof(uint32_t));
  MagicNumber = 0x456789ab;
  MagicSize = sizeof(uint32_t);
  DiskSize = (MagicSize + (NUM_SECTORS * SectorSize));
  DirectoryFileSize = (sizeof(DirectoryEntry) * NumDirEntries);
  DEBUG('u', (char *) "End of reading of configuration file\n");
}

#endif   // CONFIG_CPP
