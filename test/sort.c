/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
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

// Nachos system calls
#include "userlib/syscall.h"
#include "userlib/libnachos.h"

// Table to be sorted
#define NUM 30
int A[NUM];	

int
main()
{
    int i, j, key;

    Write("Start sort\n",11,CONSOLE_OUTPUT);
    
    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < NUM; i++)		
        A[i] = NUM - i;

    for (i = 0; i < NUM; i++) {
      n_printf("%d ",A[i]);
    }
    n_printf("\n");
    

    /* then sort! */
    for (j=1;j<NUM;j++) {
      key = A[j];
      i = j - 1;
      while (i>= 0 && A[i] > key) {
	A[i+1] = A[i];
	i--;
      }
      A[i+1] = key;
    }

    for (i = 0; i < NUM; i++) {
      n_printf("%d ",A[i]);
    }
    n_printf("\n");

    Write("End sort\n",9,CONSOLE_OUTPUT);
    Exit(A[0]);		/* and then we're done -- should be 0! */

    return 0;
}
