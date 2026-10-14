/*! \file  bitmap.cc
//  \brief Routines to manage a bitmap
//
//      An array of bits each of which
//	can be either on or off.  Represented as an array of integers.
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

#include "utility/bitmap.h"
#include "kernel/msgerror.h"

//----------------------------------------------------------------------
// BitMap::BitMap
/*! 	Initialize a bitmap with "nitems" bits, so that every bit is clear.
//	it can be added somewhere on a list.
//
//	\param nitems is the number of bits in the bitmap.
*/
//----------------------------------------------------------------------
BitMap::BitMap(int nitems) {
  numBits = nitems;
  numWords = divRoundUp(numBits, BITS_IN_WORD);
  map = new unsigned int[numWords];
  for (int i = 0; i < numBits; i++)
    Clear(i);
}

//----------------------------------------------------------------------
// BitMap::~BitMap
//!	De-allocate a bitmap.
//----------------------------------------------------------------------
BitMap::~BitMap() { delete[] map; }

//----------------------------------------------------------------------
// BitMap::Set
/*! 	Set the "nth" bit in a bitmap.
//
//	\param which is the number of the bit to be set.
*/
//----------------------------------------------------------------------
void
BitMap::Mark(int which) {
  ASSERT(which >= 0 && which < numBits);
  map[which / BITS_IN_WORD] |= 1 << (which % BITS_IN_WORD);
}

//----------------------------------------------------------------------
// BitMap::Clear
/*! 	Clear the "nth" bit in a bitmap.
//
//	\param which is the number of the bit to be cleared.
*/
//----------------------------------------------------------------------
void
BitMap::Clear(int which) {
  ASSERT(which >= 0 && which < numBits);
  map[which / BITS_IN_WORD] &= ~(1 << (which % BITS_IN_WORD));
}

//----------------------------------------------------------------------
// BitMap::Test
/*! 	\return true if the "nth" bit is set.
//
//	\param which is the number of the bit to be tested.
*/
//----------------------------------------------------------------------
bool
BitMap::Test(int which) {
  ASSERT(which >= 0 && which < numBits);

  if (map[which / BITS_IN_WORD] & (1 << (which % BITS_IN_WORD)))
    return true;
  else
    return false;
}

//----------------------------------------------------------------------
// BitMap::Find
/*! 	Return the number of the first bit which is clear.
//	As a side effect, set the bit (mark it as in use).
//	(In other words, find and allocate a bit.)
//
//	\return If no bits are clear, return ERROR
*/
//----------------------------------------------------------------------
int
BitMap::Find() {
  for (int i = 0; i < numBits; i++)
    if (!Test(i)) {
      Mark(i);
      return i;
    }
  return ERROR;
}

//----------------------------------------------------------------------
// BitMap::NumClear
/*! 	Return the number of clear bits in the bitmap.
//	(In other words, how many bits are unallocated?)
*/
//----------------------------------------------------------------------
int
BitMap::NumClear() {
  int count = 0;

  for (int i = 0; i < numBits; i++)
    if (!Test(i))
      count++;
  return count;
}

//----------------------------------------------------------------------
// BitMap::Print
/*! 	Print the contents of the bitmap, for debugging.
//
//	Could be done in a number of ways, but we just print the #'s of
//	all the bits that are set in the bitmap.
*/
//----------------------------------------------------------------------
void
BitMap::Print() {
  printf("Bitmap set:\n");
  for (int i = 0; i < numBits; i++)
    if (Test(i))
      printf("%" PRIu32 ", ", i);
  printf("\n");
}

// These aren't needed until the filesystem assignment

//----------------------------------------------------------------------
// BitMap::FetchFromFile
/*! 	Initialize the contents of a bitmap from a Nachos file.
//
//	\param file is the place to read the bitmap from
*/
//----------------------------------------------------------------------
void
BitMap::FetchFrom(OpenFile *file) {
  file->ReadAt((char *) map, numWords * sizeof(unsigned), 0);
}

//----------------------------------------------------------------------
// BitMap::WriteBack
/*! 	Store the contents of a bitmap to a Nachos file.
//
//	\param file is the place to write the bitmap to
*/
//----------------------------------------------------------------------
void
BitMap::WriteBack(OpenFile *file) {
  file->WriteAt((char *) map, numWords * sizeof(unsigned), 0);
}
