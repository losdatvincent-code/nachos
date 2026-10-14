/*! \file bitmap.h
    \brief Data structures defining a bitmap

        An array of bits each of which
        can be either on or off.

        Represented as an array of unsigned integers, on which we do
        modulo arithmetic to find the bit we are interested in.

        The bitmap can be parameterized with with the number of bits being
        managed.
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

#ifndef BITMAP_H
#define BITMAP_H

#include "filesys/openfile.h"
#include "kernel/copyright.h"
#include "utility/utility.h"

// Definitions helpful for representing a bitmap as an array of integers
#define BITS_IN_BYTE 8
#define BITS_IN_WORD 32

/*!  \brief Defines a "bitmap" -- an array of bits

   Each of which can be independently set, cleared, and tested.

   Most useful for managing the allocation of the elements of an array --
   for instance, disk sectors, or main memory pages.
   Each bit represents whether the corresponding sector or page is
   in use or free.
*/

class BitMap {
public:
  BitMap(int nitems);   // Initialize a bitmap, with "nitems" bits
                        // initially, all bits are cleared.
  ~BitMap();   // De-allocate bitmap

  void Mark(int which);    // Set the "nth" bit
  void Clear(int which);   // Clear the "nth" bit
  bool Test(int which);    // Is the "nth" bit set?
  int Find();              // Return the # of a clear bit, and as a side
                // effect, set the bit.
                // If no bits are clear, return -1.
  int NumClear();   // Return the number of clear bits

  void Print();   // Print contents of bitmap

  // These aren't needed until filesystem, when we will need to read and
  // write the bitmap to a file
  void FetchFrom(OpenFile *file);   // fetch contents from disk
  void WriteBack(OpenFile *file);   // write contents to disk

private:
  int numBits;         //!< Number of bits in the bitmap
  int numWords;        /*!< Number of words of bitmap storage
                        (rounded up if numBits is not a
                         multiple of the number of bits in
                         a word)
                       */
  unsigned int *map;   //!< Bit storage
};

#endif   // BITMAP_H
