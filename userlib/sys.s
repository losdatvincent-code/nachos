/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 *
 * -----------------------------------------------------
 * This file is part of the BurritOS distribution
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

#define IN_ASM 
#include "userlib/syscall.h"

	 // Equivalent to ".text", but with a different name, in order
	 // to be correctly handled by the ldscript
        .section .sys,"ax",@progbits
	
        .align  2

/* -------------------------------------------------------------
 * __start
 *	Initialize running a C program, by calling "main". 
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

	.globl __start
	.type	__start, @function
__start:

/* Call the program entry point */
	call main
	li a0, 0
	call Exit
	jr	ra	 /* if we return from main, exit(0) */


/* -------------------------------------------------------------
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r10, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r12, arg2 is 
 *	in r13, arg3 is in r14, arg4 is in r15)
 *
 * 	The return value is in r10. This follows the standard C calling
 * 	convention on the RISC-V.
 * -------------------------------------------------------------
 */

		.globl Halt
	.type	__Halt, @function
Halt:
	addi a7,zero,SC_HALT
	ecall
	jr	ra


	.globl Exit
	.type	__Exit, @function
Exit:
	addi a7,zero,SC_EXIT
	ecall
	jr	ra


	.globl Exec
	.type	__Exec, @function
Exec:
	addi a7,zero,SC_EXEC
	ecall
	jr ra


	.globl Join
	.type	__Join, @function
Join:
	addi a7,zero,SC_JOIN
	ecall
	jr ra
	

	.globl Create
	.type	__Create, @function
Create:
	addi a7,zero,SC_CREATE
	ecall
	jr ra
	

	.globl Open
	.type	__Open, @function
Open:
	addi a7,zero,SC_OPEN
	ecall
	jr ra
	

	.globl Read
	.type	__Read, @function
Read:
	addi a7,zero,SC_READ
	ecall
	jr ra



	.globl Write
	.type	__Write, @function
Write:
	addi a7,zero,SC_WRITE
	ecall
	jr ra


	.globl Seek
	.type	__Seek, @function
Seek:
	addi a7,zero,SC_SEEK
	ecall
	jr ra
	

	.globl Close
	.type	__Close, @function
Close:
	addi a7,zero,SC_CLOSE
	ecall
	jr ra


	.globl FSList
	.type	__FSList, @function
FSList:
	addi a7,zero,SC_FSLIST
	ecall
	jr ra
	

	.globl newThreadHidden
	.type	__newThreadHidden, @function
newThreadHidden:
	addi a7,zero,SC_NEW_THREAD
	ecall
	jr ra
	
	
	.globl Remove
	.type	__Remove, @function
Remove:
	addi a7,zero,SC_REMOVE
	ecall
	jr ra
	

	.globl Yield
	.type	__Yield, @function
Yield:
	addi a7,zero,SC_YIELD
	ecall
	jr ra
	
	
	.globl PError
	.type	__PError, @function
PError:	
	addi a7,zero,SC_PERROR
	ecall
	jr ra


	.globl P
	.type	__P, @function
P:
	addi a7,zero,SC_P
	ecall
	jr ra
	
	
	.globl V
	.type	__V, @function
V:	
	addi a7,zero,SC_V
	ecall
	jr ra
	
	.globl SemCreate
	.type	__SemCreate, @function
SemCreate:	
	addi a7,zero,SC_SEM_CREATE
	ecall
	jr ra
	

	.globl SemDestroy
	.type	__SemDestroy, @function
SemDestroy:
	addi a7,zero,SC_SEM_DESTROY
	ecall
	jr ra
	

	.globl SysTime
	.type	__SysTime, @function
SysTime:
	addi a7,zero,SC_SYS_TIME
	ecall
	jr ra
	

	.globl LockCreate
	.type	__LockCreate, @function
LockCreate:
	addi a7,zero,SC_LOCK_CREATE
	ecall
	jr ra

	.globl LockDestroy
	.type	__LockDestroy, @function
LockDestroy:	
	addi a7,zero,SC_LOCK_DESTROY
	ecall
	jr ra
	
		
	.globl LockAcquire
	.type	__LockAquire, @function
LockAcquire:
	addi a7,zero,SC_LOCK_ACQUIRE
	ecall
	jr ra
	

	.globl LockRelease
	.type	__LockRelease, @function
LockRelease:
	addi a7,zero,SC_LOCK_RELEASE
	ecall
	jr ra
	

	.globl CondCreate
	.type	__CondCreate, @function
CondCreate:
	addi a7,zero,SC_COND_CREATE
	ecall
	jr ra
	

	.globl CondDestroy
	.type	__CondDestroy, @function
CondDestroy:	
	addi a7,zero,SC_COND_DESTROY
	ecall
	jr ra


	.globl CondWait
	.type	__CondWait, @function
CondWait:	
	addi a7,zero,SC_COND_WAIT
	ecall
	jr ra
	

	.globl CondSignal
	.type	__CondSignal, @function
CondSignal:	
	addi a7,zero,SC_COND_SIGNAL
	ecall
	jr ra
	

	.globl CondBroadcast
	.type	__CondBroadcast, @function
CondBroadcast:	
	addi a7,zero,SC_COND_BROADCAST
	ecall
	jr ra
	

	.globl TtySend
	.type	__TtySend, @function
TtySend:	
	addi a7,zero,SC_TTY_SEND
	ecall
	jr ra
	

	.globl TtyReceive
	.type	__TtyReceive, @function
TtyReceive:	
	addi a7,zero,SC_TTY_RECEIVE
	ecall
	jr ra
	
	
	.globl Mkdir
	.type	__Mkdir, @function
Mkdir:	
	addi a7,zero,SC_MKDIR
	ecall
	jr ra
	
	
	.globl Rmdir
	.type	__Rmdir, @function
Rmdir:	
	addi a7,zero,SC_RMDIR
	ecall
	jr ra
	
	
	.globl Mmap
	.type	__Mmap, @function
Mmap:	
	addi a7,zero,SC_MMAP
	ecall
	jr ra

	.globl Debug
	.type	__Debug, @function
Debug:	
	addi a7,zero,SC_DEBUG
	ecall
	jr ra

	
