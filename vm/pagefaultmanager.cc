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
#include "kernel/msgerror.h"
#include "kernel/thread.h"
#include "vm/physMem.h"
#include "vm/swapManager.h"

PageFaultManager::PageFaultManager() {}

// PageFaultManager::~PageFaultManager()
/*! Nothing for now
 */
PageFaultManager::~PageFaultManager() {}

// ExceptionType PageFault(uint64_t virtualPage)
/*!
//	This method is called by the Memory Management Unit when there is a
//      page fault. This method loads the page from :
//      - read-only sections (text,rodata) $\Rightarrow$ executive
//        file
//      - read/write sections (data,...) $\Rightarrow$ executive
//        file (1st time only), or swap file
//      - anonymous mappings (stack/bss) $\Rightarrow$ new
//        page from the MemoryManager (1st time only), or swap file
//
//	\param virtualPage the virtual page subject to the page fault
//	  (supposed to be between 0 and the
//        size of the address space, and supposed to correspond to a
//        page mapped to something [code/data/bss/...])
//	\return the exception (generally the NO_EXCEPTION constant)
*/
ExceptionType
PageFaultManager::PageFault(uint64_t virtualPage) {
  printf("**** Warning: page fault manager is not implemented yet\n");
  exit(ERROR);
  return ((ExceptionType) 0);
}
