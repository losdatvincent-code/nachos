/*
//  shell.c: a very simple shell running on Nachos
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
*/

// Nachos system calls
#include "userlib/libnachos.h"

int
main()
{
    ThreadId newProc;
    OpenFileId input = CONSOLE_INPUT;
    OpenFileId output = CONSOLE_OUTPUT;
    char prompt[2], buffer[60];
    int i,bg;

    prompt[0] = '-';
    prompt[1] = '>';

    // Welcome message
    n_printf("Welcome to NachOS\n");

    while( 1 )
    {

      // Write the prompt
      Write(prompt, 2, output);
 
      // Wait for a command
      Read(buffer, 60, input); 
      
	i=0;
	bg=0;
	while(buffer[i++] != '\n' ) {};
	buffer[--i]=' ';
	
	
	
	while(buffer[--i] == ' ') {};
	
	
	// Background execution
	if (buffer[i]=='&') {
	    bg=1;
	    buffer[i]=' ';
	    while(buffer[--i] == ' ') {};
	    buffer[++i]='\0';
	    
	} else {buffer[++i] = '\0';}
	
	if (n_strcmp(buffer,"exit")==0) {
	    break;
	  }
	    
	// Execute the command
	// In the case it is a background command, don't wait for its completion
	if( i > 0 ) {
	  newProc = Exec(buffer);
	  if (newProc == -1) {
	    n_printf("\nUnable to run %s\n", buffer);
	  }
	  else if (!bg) Join(newProc);

	}
    }
    return 0;
}

