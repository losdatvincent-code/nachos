/*! \file stats.cc
//  \brief Routines for managing statistics about Nachos performance.

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
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//

#include "utility/stats.h"
#include "kernel/copyright.h"
#include "kernel/system.h"

//----------------------------------------------------------------------
// Statistics::Statistics
//!     Initializes performance metrics to zero, at system startup.
//
//
//----------------------------------------------------------------------
Statistics::Statistics() {
  allStatistics = new ListStats;
  idleTicks = totalTicks = 0;
}

//----------------------------------------------------------------------
// Statistics::Print
/*!     Prints performance metrics, when we've finished everything
//      at system shutdown. Thanks to AllStatistics, we are here able
//      to print each process performances.
*/
//----------------------------------------------------------------------
void
Statistics::Print() {
  ProcessStat *s;
  ListStats *list = new ListStats;

  printf("\n");

  while (!(allStatistics->IsEmpty())) {
    s = ((ProcessStat *) allStatistics->SortedRemove(NULL));
    s->Print();
    printf("\n");
    list->Append((void *) s);
  }

  delete allStatistics;
  allStatistics = list;
  printf("\nConcerning Nachos : \n");
  printf("   Idle time : \t\t%" PRIu64 " cycles on a %" PRIu32
         " Mz processor (%" PRIu64 " sec, %" PRIu64 " nanos)\n",
         idleTicks, gConfig->ProcessorFrequency,
         cycle_to_sec(idleTicks, gConfig->ProcessorFrequency),
         cycle_to_nano(idleTicks, gConfig->ProcessorFrequency));
  printf("   Total time : \t%" PRIu64 " cycles on a %" PRIu32
         "Mz processor (%" PRIu64 " sec, %" PRIu64 " nanos) \n",
         totalTicks, gConfig->ProcessorFrequency,
         cycle_to_sec(totalTicks, gConfig->ProcessorFrequency),
         cycle_to_nano(totalTicks, gConfig->ProcessorFrequency));
}

ProcessStat *
Statistics::NewProcStat(char *name) {
  ProcessStat *procstat = new ProcessStat(name);
  allStatistics->Append((void *) procstat);
  return procstat;
}

//----------------------------------------------------------------------
// Statistics::~Statistics
//!    De-allocate all ProcessStats and the allStatistics list
//
//----------------------------------------------------------------------

Statistics::~Statistics() {
  ProcessStat *s;
  while (!(allStatistics->IsEmpty())) {
    s = ((ProcessStat *) allStatistics->SortedRemove(NULL));
    delete s;
  }
  delete allStatistics;
}

//----------------------------------------------------------------------
// ProcessStat::ProcessStat
/*!     Initializes performance metrics to zero, when a process startups
.
//      \param name name of the process
*/
//----------------------------------------------------------------------
ProcessStat::ProcessStat(char *processName) {
  strcpy(name, processName);
  numInstruction = numDiskReads = numDiskWrites = 0;
  numConsoleCharsRead = numConsoleCharsWritten = 0;
  numMemoryAccess = numPageFaults = 0;
  systemTicks = userTicks = 0;
}

//----------------------------------------------------------------------
// ProcessStat::incrSystemTicks
/*!     Increments the time spent in the operating system, at the process and
system level
.
//      \param val increment
*/
//----------------------------------------------------------------------
void
ProcessStat::incrSystemTicks(Time val) {
  systemTicks += val;             // Process level
  gStats->incrTotalTicks(val);   // System level
}

//----------------------------------------------------------------------
// ProcessStat::incrSystemTicks
/*!     Increments the time spent in user mode, at the process and system level
.
//      \param val increment
*/
//----------------------------------------------------------------------
void
ProcessStat::incrUserTicks(Time val) {
  userTicks += val;               // Process level
  gStats->incrTotalTicks(val);   // System level
}

//----------------------------------------------------------------------
// ProcessStat::incrMemoryAccess(void)
/*!     Updates stats concerning a memory access (process and system level)
.
*/
//----------------------------------------------------------------------
void
ProcessStat::incrMemoryAccess(void) {

  // Process level
  numMemoryAccess++;
  userTicks += MEMORY_TICKS;

  // System level
  gStats->incrTotalTicks(MEMORY_TICKS);
}

//----------------------------------------------------------------------
// ProcessStat::Print
/*!     Prints per-process statistics
.
//      \param val increment
*/
//----------------------------------------------------------------------
void
ProcessStat::Print(void) {
  printf("------------------------------------------------------------\n");
  printf("Statistics for process : \t%s \n", name);
  printf("   Number of instructions executed : \t%" PRIu64 " \n",
         numInstruction);
  printf("   System time : \t\t%" PRIu64 " cycles on a %" PRIu32
         "Mz processor (%" PRIu64 " sec, %" PRIu64 " nanos)\n",
         systemTicks, gConfig->ProcessorFrequency,
         cycle_to_sec(systemTicks, gConfig->ProcessorFrequency),
         cycle_to_nano(systemTicks, gConfig->ProcessorFrequency));
  printf("   User time   : \t\t%" PRIu64 " cycles on a %" PRIu32
         "Mz processor (%" PRIu64 " sec, %" PRIu64 " nanos)\n",
         userTicks, gConfig->ProcessorFrequency,
         cycle_to_sec(userTicks, gConfig->ProcessorFrequency),
         cycle_to_nano(userTicks, gConfig->ProcessorFrequency));
  printf("   Disk Input/Output : \t\treads  %" PRIu64 ", writes  %" PRIu64 "\n",
         numDiskReads, numDiskWrites);
  printf("   Console Input Output : \treads  %" PRIu64 ", writes  %" PRIu64
         "\n",
         numConsoleCharsRead, numConsoleCharsWritten);
  printf("   Memory Management :  \t%" PRIu64 " accesses,  %" PRIu64
         " page faults\n",
         numMemoryAccess, numPageFaults);

  printf("------------------------------------------------------------\n");
}
