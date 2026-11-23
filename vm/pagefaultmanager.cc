/*! \file pagefaultmanager.cc
Routines for the page fault managerPage Fault Manager
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

#include "vm/pagefaultmanager.h"
#include "kernel/addrspace.h"
#include "kernel/msgerror.h"
#include "kernel/thread.h"
#include "machine/machine.h"
#include "vm/physMem.h"
#include "vm/swapManager.h"

PageFaultManager::PageFaultManager() {}

PageFaultManager::~PageFaultManager() {}

ExceptionType
PageFaultManager::PageFault(uint64_t virtualPage) {
#ifdef ETUDIANTS_TP
  AddrSpace *addrspace = gCurrentThread->GetProcessOwner()->addrspace;

  // Tâche 12 : Demander une case mémoire physique libre
  int pp = gPhysicalMemManager->FindFreePage();

  // Tâche 13 : Suspendre la machine s'il n'y a plus de RAM disponible
  if (pp == INVALID_PAGE) {
    printf("Not enough free space to handle page fault\n");
    gMachine->interrupt->Halt(ERROR);
  }

  char *physAddr = (char *) &(gMachine->mainMemory[pp * gConfig->PageSize]);

  // Tâche 14 : Identifier la nature de la page fautive et remplir la case physique
  ElfPageInfo &pageInfo = addrspace->elfPages[virtualPage];

  if (pageInfo.inElf && !pageInfo.noBits) {
    // Code ou data initialisée (présent dans l'ELF)
    memset(physAddr, 0x0, gConfig->PageSize);
    addrspace->execFile->ReadAt(physAddr, pageInfo.size, pageInfo.offset);
  } else {
    // Pile (inElf == false) ou section BSS / SHT_NOBITS (noBits == true)
    memset(physAddr, 0x0, gConfig->PageSize);
  }

  return NO_EXCEPTION;
#endif
}