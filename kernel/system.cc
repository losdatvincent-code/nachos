/*! \file system.cc
//  \brief Nachos initialization and cleanup routines.

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

#include "kernel/system.h"
#include "drivers/drvACIA.h"
#include "drivers/drvConsole.h"
#include "drivers/drvDisk.h"
#include "filesys/filesys.h"
#include "filesys/oftable.h"
#include "kernel/msgerror.h"
#include "kernel/scheduler.h"
#include "kernel/thread.h"
#include "utility/config.h"
#include "utility/objaddr.h"
#include "utility/stats.h"
#include "utility/utility.h"
#include "vm/pagefaultmanager.h"
#include "vm/physMem.h"
#include "vm/swapManager.h"

/*!  This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.
*/

// Hardware components
Machine *gMachine;   //!< Machine (includes CPU and peripherals)

// Thread management
Thread *gCurrentThread;         //!< The thread holding the CPU
Thread *gThreadToBeDestroyed;   //!< The thread that just finished
ListThread *gAlive;             //!< List of existing threads
Scheduler *gScheduler;          //!< Thread scheduler

// Device drivers
DriverDisk *gDiskDriver;         //!< Disk driver
DriverDisk *gSwapDiskDriver;    //!< Swap disk driver
DriverConsole *gConsoleDriver;   //!< Console driver
DriverACIA *gAciaDriver;         //!< Serial line driver

// Other Nachos components
FileSystem *gFileSystem;                //!< File system
OpenFileTable *gOpenFileTable;         //!< Open File Table
SwapManager *gSwapManager;              //!< Management of swap area
PageFaultManager *gPageFaultManager;   //!< Page fault handler (used in VMM)
PhysicalMemManager *gPhysicalMemManager;   //!< Physical memory manager
SyscallError *gSyscallError;                //!< Error management
Config *gConfig;                                //!< Configuration of Nachos
Statistics *gStats;                          //!< performance metrics
ObjAddr *gObjectAddrs;                      //!< addresses of kernel objets

// Endianess of data in ELF file and endianess of host
char risc_endianess;
char host_endianess;

//----------------------------------------------------------------------
// CleanupOK
/*!     Function called when the user presses Ctrl-C
 */
//----------------------------------------------------------------------
void
CleanupOK() {
  Cleanup();
  Exit(0);
}

//----------------------------------------------------------------------
// TimerInterruptHandler
/*! 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as
//	if the interrupted thread called Yield at the point it is
//	was interrupted.
//
//	\param dummy is because every interrupt handler takes one argument,
//		whether it needs it or not.
*/
//----------------------------------------------------------------------
/*
static void
TimerInterruptHandler(int dummy)
{
    if (machine->getStatus() != IdleMode)
        machine->interrupt->YieldOnReturn();
}
*/

//----------------------------------------------------------------------
// Initialize
/*! 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.
//
//	\param argc is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3
//	\param argv is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
*/
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv) {
  uint64_t errStatus;
  int argCount;
  char *debugArgs = (char *) "";
  char filename[MAXSTRLEN];
  bool debugUserProg = false;   //!< single step user program

  strcpy(filename, CONFIGFILENAME);

  // Scan the arguments
  for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
    argCount = 1;
    if (!strcmp(*argv, (char *) "-d")) {
      if (argc == 1)
        debugArgs = (char *) "+";   // turn on all debug flags
      else {
        debugArgs = *(argv + 1);
        argCount = 2;
      }
    }

    if (!strcmp(*argv, (char *) "-s"))
      debugUserProg = true;
    if (!strcmp(*argv, (char *) "-f")) {
      strcpy(filename, *(argv + 1));
    }
  }

  // Scan configuration file to set up Nachos parameters
  gConfig= new Config(filename);

  // Set up debug level
  DebugInit(debugArgs);   // initialize DEBUG messages

  // Create the statistics object (used from the very start)
  gStats = new Statistics();

  // Create the Nachos hardware
  gMachine = new Machine(debugUserProg);

  // Create the device drivers
  gDiskDriver = new DriverDisk((char *) "sem disk", (char *) "lock disk",
                                 gMachine->disk);
  if (gConfig->ACIA)
    gAciaDriver = new DriverACIA();
  gConsoleDriver = new DriverConsole();

  // Create the different objects making the Nachos kernel
  gScheduler = new Scheduler();   // Initialize the ready queue
  gPageFaultManager = new PageFaultManager();
  gSwapManager = new SwapManager();
  gSwapDiskDriver = gSwapManager->GetSwapDisk();
  gPhysicalMemManager = new PhysicalMemManager();
  gSyscallError = new SyscallError();

  // Init the Nachos internal data structures
  gAlive = new ListThread();   // List of threads (initially empty)
  gObjectAddrs = new ObjAddr();
  gThreadToBeDestroyed = NULL;
  gOpenFileTable = new OpenFileTable;

  // Cleanup if user presses Ctrl-C
  CallOnUserAbort(CleanupOK);

  // We didn't explicitly allocate the current thread we are running in.
  // But if it ever tries to give up the CPU, we better have a Thread
  // object to save its state.
  // It's just a temporary thread

  // Create the process (address space + statistics) context for this temporary
  // thread
  Process *rootProcess = new Process(NULL, &errStatus);
  if (errStatus != NO_ERROR) {
    fprintf(stderr, "Nachos boot error: cannot allocate process\n");
    Exit(ERROR);
  }

  // Create the root thread
  gCurrentThread = new Thread((char *) "main-temp");
  errStatus = gCurrentThread->Start(rootProcess, 0x0, -1);
  if (errStatus != NO_ERROR) {
    fprintf(stderr, "Nachos boot error: cannot start thread\n");
    Exit(ERROR);
  }

  // Remove gCurrentThread from ready list (inserted by default)
  // because it is currently executing
  ASSERT(gCurrentThread == gScheduler->FindNextToRun());

  // Enable interrupts
  gMachine->interrupt->SetStatus(INTERRUPTS_ON);

  // Init the Nachos file system
  // NB: uses the disk, so blocks the calling thread.
  // Thus; FileSystem initiaization has to be done after the first
  // (temporary) thread is created
  gFileSystem = new FileSystem(gConfig->FormatDisk);
}

//----------------------------------------------------------------------
// Cleanup
/*! 	Nachos is halting.  De-allocate global data structures.
 */
//----------------------------------------------------------------------
void
Cleanup() {
  // Delete currently executing thread if any. This has to be done
  // because the last running thread, even if finished, is not deleted
  // yet (deletion is done in the following context switch), for the
  // last executing thread after cleanup, there is no following
  // context switch, we have to free resources here.
  if (gCurrentThread != NULL) {
    delete gCurrentThread;
  }

  // Clean all global objects
  printf("\nCleaning up...\n");
  if (gConfig->PrintStat) {
    gStats->Print();
  }
  delete gDiskDriver;
  delete gConsoleDriver;
  if (gConfig->ACIA)
    delete gAciaDriver;
  delete gSyscallError;
  delete gFileSystem;
  delete gOpenFileTable;
  delete gSwapManager;
  delete gScheduler;
  delete gStats;
  delete gPhysicalMemManager;
  delete gPageFaultManager;
  delete gConfig;
  delete gAlive;
  delete gObjectAddrs;
  delete gMachine;
}
