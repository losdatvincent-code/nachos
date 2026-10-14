/*! \file pagefaultmanager.h
   \brief Data structures for the page fault manager
  
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


#ifndef PFM_H
#define PFM_H

#include "machine/machine.h"

/*! \brief Defines the page fault manager
   This object manages the page fault of the simulated MIPS processor 
   for the Nachos kernel.
*/
class PageFaultManager {

public:
   PageFaultManager();

  ~PageFaultManager();
 
  ExceptionType PageFault(uint64_t virtualPage); //!< Page faut handler
};

#endif // PFM_H
