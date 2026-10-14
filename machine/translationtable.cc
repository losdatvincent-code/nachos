/*! \file translationtable..cc
// \brief Data structures for address translation
//
// DO NOT CHANGE -- part of the machine emulation
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

//
*/

#include "machine/translationtable.h"
#include "kernel/system.h"
#include "kernel/thread.h"
#include "utility/config.h"

//----------------------------------------------------------------------
// TranslationTable::TranslationTable
/*!  Constructor. Allocate the page table entries
 */
//----------------------------------------------------------------------
TranslationTable::TranslationTable() {

  // Init private fields
  maxNumPages = gConfig->MaxVirtPages;

  DEBUG('h', (char *) "Allocationg translation table for %d pages (%ld kB)\n",
        maxNumPages, ((long long) maxNumPages * gConfig->PageSize) >> 10);
  pageTable = new PageTableEntry[maxNumPages];
}

//----------------------------------------------------------------------
// TranslationTable::~TranslationTable()
/*!  Destructor. De-allocate the memory management structures
 */
//----------------------------------------------------------------------
TranslationTable::~TranslationTable() {
  delete[] pageTable;
  DEBUG('h', (char *) "Translation table destroyed");
}

//----------------------------------------------------------------------
// TranslationTable::getMaxNumPages()
/*! Get the number of pages that can be translated using the
//  translation table.
*/
//----------------------------------------------------------------------
int
TranslationTable::getMaxNumPages() {
  return maxNumPages;
}

//----------------------------------------------------------------------
// TranslationTable::setPhysicalPage
/*!  Set the physical page of a virtual page
//   \param virtualPage : the virtual page
//   \param physicalPage : the physical page
*/
//----------------------------------------------------------------------
void
TranslationTable::setPhysicalPage(uint64_t virtualPage, uint64_t physicalPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].physicalPage = physicalPage;
}

//----------------------------------------------------------------------
//  TranslationTable::getPhysicalPage
/*!  Get the physical page of a virtual page
//   \param virtualPage : the virtual page
//   \return the physical page
*/
//----------------------------------------------------------------------
uint64_t
TranslationTable::getPhysicalPage(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].physicalPage;
}

//----------------------------------------------------------------------
//  TranslationTable::setAddrDisk
/*!  Set the disk address of a virtual page
//   \param virtualPage : the virtual page
//   \param addrDisk : the address on disk (page number in swap, bytes in
//                     executable file)
*/
//----------------------------------------------------------------------
void
TranslationTable::setAddrDisk(uint64_t virtualPage, uint32_t addrDisk) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].addrDisk = addrDisk;
}

//----------------------------------------------------------------------
//  TranslationTable::getAddrDisk
/*!  Get the disk address of a virtual page
//   \param virtualPage : the virtual page
//   \return the disk address (page number in swap, or bytes in exec file)
*/
//----------------------------------------------------------------------
uint32_t
TranslationTable::getAddrDisk(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].addrDisk;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitValid
/*!  Set the bit valid of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::setBitValid(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].valid = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitValid
/*!  Clear the bit valid of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::clearBitValid(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].valid = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitValid
/*!  Get the bit valid of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit valid
*/
//----------------------------------------------------------------------
bool
TranslationTable::getBitValid(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].valid;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitIo
/*!  Set the bit io of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::setBitIo(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].io = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitIo
/*!  Clear the bit io of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::clearBitIo(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].io = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitIo
/*!  Get the bit io of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit io
*/
//----------------------------------------------------------------------
bool
TranslationTable::getBitIo(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].io;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitSwap
/*!  Set the bit swap of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::setBitSwap(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].swap = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitSwap
/*!  Clear the bit swap of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::clearBitSwap(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].swap = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitSwap
/*!  Get the bit swap of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit swap
*/
//----------------------------------------------------------------------
bool
TranslationTable::getBitSwap(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].swap;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitReadAllowed
/*!  Set the bit readAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::setBitReadAllowed(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].readAllowed = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitReadAllowed
/*!  Clear the bit readAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::clearBitReadAllowed(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].readAllowed = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitReadAllowed
/*!  Get the bit readAllowed of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit readAllowed
*/
//----------------------------------------------------------------------
bool
TranslationTable::getBitReadAllowed(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].readAllowed;
}

//----------------------------------------------------------------------
//  TranslationTable::setBitWriteAllowed
/*!  Set the bit writeAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::setBitWriteAllowed(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].writeAllowed = true;
}

//----------------------------------------------------------------------
//  TranslationTable::clearBitWriteAllowed
/*!  Clear the bit writeAllowed of a virtual page
//   \param virtualPage : the virtual page
*/
//----------------------------------------------------------------------
void
TranslationTable::clearBitWriteAllowed(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].writeAllowed = false;
}

//----------------------------------------------------------------------
//   TranslationTable::getBitWriteAllowed
/*!  Get the bit writeAllowed of a virtual page
//   \param virtualPage : the virtual page
//   \return value of the bit writeAllowed
*/
//----------------------------------------------------------------------
bool
TranslationTable::getBitWriteAllowed(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].writeAllowed;
}

void
TranslationTable::setBitU(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].U = true;
}

void
TranslationTable::clearBitU(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].U = false;
}
bool
TranslationTable::getBitU(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].U;
}

void
TranslationTable::setBitM(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].M = true;
}

void
TranslationTable::clearBitM(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  pageTable[virtualPage].M = false;
}
bool
TranslationTable::getBitM(uint64_t virtualPage) {
  ASSERT((virtualPage >= 0) && (virtualPage < maxNumPages));
  return pageTable[virtualPage].M;
}

//----------------------------------------------------------------------
//   PageTableEntry::PageTableEntry
/*!  Constructor. Defaut initialization of a page table entry
 */
//----------------------------------------------------------------------
PageTableEntry::PageTableEntry() {
  valid = false;
  swap = false;
  addrDisk = -1;
  readAllowed = false;
  writeAllowed = false;
  U = false;
  M = false;
}
