#include <stdio.h>
#include <stdlib.h>

int main(){
    int *as;
    
    as = (int*)malloc(10*sizeof(int));
    
    int sum = 0;

    for ( int ix = 0; ix < 10; ++ix )
        as[ix] = ix;

    for ( int ix = 0; ix < 10; ++ix )
        sum += as[ix];

    printf("%d\n", sum);

    free(as);
}

/*
Compile the program with optimization level 0 and with the flag -g, then run the program with valgrind memcheck.

Discuss the respective valgrind output.
1. Comment out the initialization of as.



2. Comment out the freeing of as.



3. Amend the code with an additional free(as) at the end.


*/