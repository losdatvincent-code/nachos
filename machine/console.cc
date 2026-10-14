/*! \file console.cc
//  \brief Routines to simulate a serial port to a console device.
//
//	A console has input (a keyboard) and output (a display).
//	These are each simulated by operations on UNIX files.
//	The simulated device is asynchronous,
//	so we have to invoke the interrupt handler (after a simulated
//	delay), to signal that a byte has arrived and/or that a written
//	byte has departed.
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
//  DO NOT CHANGE -- part of the machine emulation
//

#include "machine/console.h"
#include "kernel/system.h"
#include "machine/machine.h"

//! Dummy function because C++ is weird about pointers to member functions
static void
ConsoleReadPoll(int64_t c) {
  Console *console = (Console *) c;
  console->CheckCharAvail();
}
static void
ConsoleWriteDone(int64_t c) {
  Console *console = (Console *) c;
  console->WriteDone();
}

//----------------------------------------------------------------------
/** 	Constructor. Initialize the simulation of a hardware console device.
//
//	\param readFile UNIX file simulating the keyboard (NULL -> use stdin)
//	\param writeFile UNIX file simulating the display (NULL -> use stdout)
// 	\param readAvail is the interrupt handler called when a character
//              arrives from the keyboard
// 	\param writeDone is the interrupt handler called when a character has
//		been output, so that it is ok to request the next char be
//		output
*/
//----------------------------------------------------------------------
Console::Console(char *readFile, char *writeFile,
                 VoidNoArgFunctionPtr readAvail,
                 VoidNoArgFunctionPtr writeDone) {
  if (readFile == NULL)
    readFileNo = 0;   // keyboard = stdin
  else
    readFileNo = OpenForReadWrite(readFile, true);   // should be read-only
  if (writeFile == NULL)
    writeFileNo = 1;   // display = stdout
  else
    writeFileNo = OpenForWrite(writeFile);

  // set up the stuff to emulate asynchronous interrupts
  writeHandler = writeDone;
  readHandler = readAvail;
  putBusy = false;
  incoming = EOF;

  intState = false;
}

//----------------------------------------------------------------------
//! 	Destructor. Clean up console emulation
//----------------------------------------------------------------------

Console::~Console() {
  if (readFileNo != 0)
    Close(readFileNo);
  if (writeFileNo != 1)
    Close(writeFileNo);
}

//----------------------------------------------------------------------
/*! 	Periodically called to check if a character is available for
//	input from the simulated keyboard (eg, has it been typed?).
//
//	Only read it in if there is buffer space for it (if the previous
//	character has been grabbed out of the buffer by the Nachos kernel).
//	Invoke the "read" interrupt handler, once the character has been
//	put into the buffer.
*/
//----------------------------------------------------------------------

void
Console::CheckCharAvail() {
  char c;

  // schedule the next time to poll for a packet
  if (intState)
    gMachine->interrupt->Schedule(
        ConsoleReadPoll, (int64_t) this,
        nano_to_cycles(CONSOLE_TIME, gConfig->ProcessorFrequency),
        CONSOLE_READ_INT);

  // do nothing if character is already buffered, or none to be read
  if ((incoming != EOF) || !PollFile(readFileNo))
    return;

  // otherwise, read character and tell user about it
  Read(readFileNo, &c, sizeof(char));
  incoming = c;
  (*readHandler)();
}

//----------------------------------------------------------------------
/*! 	Internal routine called when it is time to invoke the interrupt
//	handler to tell the Nachos kernel that the output character has
//	completed.
*/
//----------------------------------------------------------------------

void
Console::WriteDone() {
  putBusy = false;
  (*writeHandler)();
}

//----------------------------------------------------------------------
/*! 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//      \return read character
*/
//----------------------------------------------------------------------

char
Console::GetChar() {
  char ch = incoming;

  incoming = EOF;
  return ch;
}

//----------------------------------------------------------------------
/*! 	Write a character to the simulated display, schedule an interrupt
//	to occur in the future, and return.
*/
//----------------------------------------------------------------------

void
Console::PutChar(char ch) {
  ASSERT(putBusy == false);
  WriteFile(writeFileNo, &ch, sizeof(char));
  putBusy = true;
  gMachine->interrupt->Schedule(
      ConsoleWriteDone, (int64_t) this,
      nano_to_cycles(CONSOLE_TIME, gConfig->ProcessorFrequency),
      CONSOLE_WRITE_INT);
}

//----------------------------------------------------------------------
/*!  Enable the console interrupt
 */
//----------------------------------------------------------------------
void
Console::EnableInterrupt() {
  intState = true;
  gMachine->interrupt->Schedule(
      ConsoleReadPoll, (int64_t) this,
      nano_to_cycles(CONSOLE_TIME, gConfig->ProcessorFrequency),
      CONSOLE_READ_INT);
}

//----------------------------------------------------------------------
/*! 	Disable the console interrupt
 */
//----------------------------------------------------------------------
void
Console::DisableInterrupt() {
  intState = false;
}
