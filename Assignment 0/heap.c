#include <stdio.h>
#include <stdlib.h>

/*Heap allocation

Heap allocation is the alternative to stack allocation. 
It is generally slower, but more flexible. 
In particular, it has less restrictions on the size of allocated memory.

Write a program with the following allocation pattern

int size = 10;

int * as = (int*) malloc(sizeof(int) * size);
for ( size_t ix = 0; ix < size; ++ix )
  as[ix] = 0;

printf("%d\n", as[0]);

free(as);
Compile and run as above, and verify that the program does not fail for 
sizes that triggered a segmentation fault before.*/

int main(){
    int size = 10;

    printf("Size:%d\n", size);

    //dynamica memory allocation using the function malloc (return type void*)
    int *as = (int*) malloc(sizeof(int) * size);

    for (size_t ix = 0; ix < size; ++ix){
        as[ix] = 0;
    }

    printf("%d\n", as[0]);

    //Always remember to free the dynamica allocated memory
    free(as);

    return 0;

}

//Compile and run as above, and verify that the program does not fail for sizes 
//that triggered a segmentation fault before.

/*This is because, using malloc to do the dynamic memory allocation will allocate the
array on heap. While heap has a larger space than stack. As the size increases,
there is still enough space to store the array on heap, so it will not trigger the 
segmentation fault error that encountered in stack allocation before.
*/
