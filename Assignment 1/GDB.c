#include <stdio.h>
#include <stdlib.h>

int main(){
    int *as = NULL;

    int sum = 0;

    for (int ix=0; ix<10; ++ix)
        as[ix] = ix;
    
    for (int ix=0; ix<10; ++ix)
        sum += as[ix];
    
    free(as);
}

/*
Compile the program with optimization level 0 and with the flag -g, 
then run the program with gdb. Follow the next steps:

Press "r" and enter to start running the program.
Once the execution stops (because of invalid memory access), type "focus src" and press enter.
Type "p as" and press enter to print the value of as at the time the program aborted execution.
You can quit the debugger by typing quit and then pressing enter.*/