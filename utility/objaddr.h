/*! \file objaddr.h
    \brief Object address data structure

    Nachos stores a data structure associating object ids with
    their pointers in the kernel address space. The ObjAddr class
    allows to maintain this data structure.

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
 * ----------------------------------------------------
*/

#ifndef OBJADDR_H
#define OBJADDR_H

#include "kernel/copyright.h"
#include "kernel/system.h"
#include "utility/list.h"
#include "utility/utility.h"
#include <map>

using namespace std;

/*! \brief Definition of object identifiers
//
// The ObjId class defines a set of object identifiers.  By object, we
// mean every Nachos object (Process, Thread, Semaphore, Lock; etc).
// The class stores the list of created objects and for each of them
// associates an object identifier than can be passed to subsequent
// system calls on the object.
//
// A method allows to detect of an object corresponding to a given
// identifier exists; this is used to check the parameters of system
// calls.
*/
class ObjAddr {
private:
  int last_id;
  map<const int32_t, void *> ids;

public:
  ObjAddr() { last_id = 3; /* 0, 1 and 2 used for file descriptors */ }
  ~ObjAddr() { ids.clear(); };

  int32_t AddObject(void *ptr) {
    int32_t res = last_id++;
    ids[res] = ptr;
    if (res < 0) {
      printf("**** Nachos kernel panic, not enough object identifiers\n");
      extern void Cleanup();
      Cleanup();
    }
    return res;
  }

  void *SearchObject(int32_t id) {
    if (ids.find(id) == ids.end()) {
      return (void *) NULL;
    }
    return ids[id];
  }

  void RemoveObject(int32_t id) { ids.erase(id); }

  void RemoveObject(void *ptr) {
    int32_t id = -1;
    for (map<const int32_t, void *>::iterator it = ids.begin(); it != ids.end();
         ++it) {
      if (it->second == ptr) {
        id = it->first;
        break;
      }
    }
    if (id != -1)
      ids.erase(id);
  }
};

#endif   // OBJADDR_H
