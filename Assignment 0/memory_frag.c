#include <stdio.h>
#include <stdlib.h>

/*
Reducing memory fragmentation

When allocating memory on the heap, there is no guarantee about where it is positioned. 
A common strategy to avoid this is to allocate memory in large blocks (contiguous memory).
Implement a program based on the next two code snippets. 
Explain the meaning of all variables, and exhibit the possible layout of allocated 
memory in both cases.

Using contiguous memory and thus avoiding memory fragmentation:

int size = 10;

int * asentries = (int*) malloc(sizeof(int) * size*size);
int ** as = (int**) malloc(sizeof(int*) * size);
for ( size_t ix = 0, jx = 0; ix < size; ++ix, jx+=size )
  as[ix] = asentries + jx;

for ( size_t ix = 0; ix < size; ++ix )
  for ( size_t jx = 0; jx < size; ++jx )
    as[ix][jx] = 0;

printf("%d\n", as[0][0]);

free(as);
free(asentries);

Not avoiding memory fragmentation:

int size = 10;

int ** as = (int**) malloc(sizeof(int*) * size);
for ( size_t ix = 0; ix < size; ++ix )
  as[ix] = (int*) malloc(sizeof(int) * size);

for ( size_t ix = 0; ix < size; ++ix )
  for ( size_t jx = 0; jx < size; ++jx )
    as[ix][jx] = 0;

printf("%d\n", as[0][0]);

for ( size_t ix = 0; ix < size; ++ix )
    free(as[ix]);
free(as);*/

//Explain the meaning of all variables, and 
//exhibit the possible layout of allocated memory in both cases.

int main(){

    //Part I: Using contiguous memory & avoiding memory fragmentation
    {
    int size = 10;
    //a pointer to arrays with size size*size, i.e. 10x10 here
    int *asentries = (int*) malloc(sizeof(int) * size * size);
    // a pointer to the pointer to int (to the row)
    int **as = (int**) malloc(sizeof(int*) * size);

    for ( size_t ix = 0, jx = 0; ix < size; ++ix, jx+=size )
        as[ix] = asentries + jx; //assign as to the row of the pointer to array

    for ( size_t ix = 0; ix < size; ++ix )
        for ( size_t jx = 0; jx < size; ++jx )
            as[ix][jx] = 0; //assign the value of each array element

    printf("%d\n", as[0][0]); //print out the first element in the first row of the array

    free(as); //free the pointers point to the address of the row of the array
    free(asentries); //free the pointer to the array
    }
// a contiguous memory, the elements of the array are stored in contiguous memory blocks
// So we can access the array lineraly as they are stored in this contiguous blocks

    //Part II: Not avoiding memory fragmentation
    {
    int size = 10;
    // a pointer point to the pointer to int with size 10 int (10 x 4 bytes)
    int **as = (int**) malloc(sizeof(int*) * size);
    
    for ( size_t ix = 0; ix < size; ++ix )
        as[ix] = (int*) malloc(sizeof(int) * size);//allocate each row of as

    for ( size_t ix = 0; ix < size; ++ix )
        for ( size_t jx = 0; jx < size; ++jx )
            as[ix][jx] = 0;

    printf("%d\n", as[0][0]);

    for ( size_t ix = 0; ix < size; ++ix )
        free(as[ix]);//free each of the allocation (point to each row of the array)
    free(as);
    }

//Since the memory are allocated separately for each row, the rows of the array may stored in separate memory block on heap
//So we cant access this array linearly 
}