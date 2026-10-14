/*! \file  main.cc
//  \brief Bootstrap code to initialize the operating system kernel.
//
// Usage: nachos -d <debugflags>
//		-s -x <nachos file>
//              -z -f <configfile>
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -s causes user programs to be executed in single-step mode
//    -z prints the copyright message
//    -f <configfile> gives the name of a configuration file for Nachos
//    -x runs a user program
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

#define MAIN
#include "kernel/copyright.h"
#undef MAIN

#include "kernel/msgerror.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "utility/config.h"
#include "utility/utility.h"

// External functions used by this file
extern void Copy(char *unixFile, char *nachosFile);
extern void Print(char *file);
extern void StartProcess(char *file);

//----------------------------------------------------------------------
// main
/*! 	Bootstrap the operating system kernel.
//
//	Check command line arguments
//	Initialize data structures
//	(optionally) Call test procedure
//
//	\param argc is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3
//	\param argv is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
*/
//----------------------------------------------------------------------

int
main(int argc, char **argv) {
  int argCount;   // Number of arguments for a particular command
  uint64_t err;   // Error code

  // Init Nachos data structures
  Initialize(argc, argv);
  char *startfilename = gConfig->ProgramToRun;

  // Process command line arguments
  for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
    argCount = 1;

    if (!strcmp(*argv, "-z")) {   // print copyright
      printf("%s", copyright);
      exit(0);
    }

    if (!strcmp(*argv, "-h")) {   // give commande line arguments
      printf("Usage: %s [opts]\n", argv[0]);
      printf("   -d <debugflags> : turn on debug flags specified in "
             "<debugflags>\n");
      printf("   -s              : single step mode\n");
      printf("   -x <elf_file>   : execute MIPS binary file <elf_file>\n");
      printf("   -z              : print copyright information\n");
      printf("   -f <cfg_file>   : use <cfg_file> instead of default "
             "configuration file\n");
      printf("   -h              : list command line arguments\n");
      exit(0);
    }
    if (!strcmp(*argv, "-x")) {   // run a user program
      ASSERT(argc > 1);
      argCount = 2;
      startfilename = argv[1];
    }
  }

  if (gConfig->Remove) {   // remove Nachos file
    gFileSystem->Remove(gConfig->FileToRemove);
  }
  if (gConfig->MakeDir) {   // Make Nachos directory
    gFileSystem->Mkdir(gConfig->DirToMake);
  }
  if (gConfig->RemoveDir) {   // Remove Nachos file
    gFileSystem->Rmdir(gConfig->DirToRemove);
  }
  if (gConfig->NbCopy != 0) {   // copy from UNIX to Nachos

    for (uint32_t i = 0; i < gConfig->NbCopy; i++) {
      if ((strlen(gConfig->ToCopyUnix[i]) != 0) &&
          (strlen(gConfig->ToCopyNachos[i]) != 0))
        Copy(gConfig->ToCopyUnix[i], gConfig->ToCopyNachos[i]);
    }
  }
  if (gConfig->Print) {   // print a Nachos file
    Print(gConfig->FileToPrint);
  }
  if (gConfig->ListDir) {   // list Nachos directory
    gFileSystem->List();
  }
  if (gConfig->PrintFileSyst) {   // print entire filesystem
    gFileSystem->Print();
  }

  if (!strcmp(startfilename, "")) {
    printf("Warning: No program to start\n");
  } else {
    Process *p = new Process(startfilename, &err);
    if (err != NO_ERROR) {
      fprintf(stderr, gSyscallError->GetFormat(err), startfilename);
      exit(ERROR);
    }
    gMachine->mmu->translationTable = p->addrspace->translationTable;
    Thread *t = new Thread(startfilename);
    gObjectAddrs->AddObject(t);
    err = t->Start(p, p->addrspace->getCodeStartAddress64(), -1);
    if (err != NO_ERROR) {
      fprintf(stderr, "Unable to start initial process: %s\n", startfilename);
      exit(ERROR);
    }
  }

  gCurrentThread->Finish();   // NOTE: if the procedure "main"
                                // returns, then the program "nachos"
                                // will exit (as any other normal program
                                // would).  But there may be other
                                // threads on the ready list.  We switch
                                // to those threads by saying that the
                                // "main" thread is finished, preventing
                                // it from returning.
  return (0);                   // Not reached...
}
