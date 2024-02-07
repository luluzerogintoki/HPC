#include <stdio.h>  
#include <stdlib.h>
/*Implement two programs. 

One that
1. Opens a file for writing,
2. Writes a square matrix of size 10 with int entries 
(ix*jx in the ix-th row and jx-th column) to that file,
3. Closes the file.

The other one
1. Reopens the file for reading,
2. Reads the matrix from the file,
3. Checks that the entry in the ix-th row and jx-th column equals ix*jx.

How does your choice of memory allocation (contiguous vs. noncontiguous) 
impact the possibilties to write the matrix in text and binary format?*/

int main(){
{ 
 // Part I: writing to file

    //Opens a file for writing
    FILE *file =  fopen("a0.dat", "w");

    //Check if the file is successfuly opened or not
    if (file == NULL){
        printf("Error opening file\n");
        return -1;
    }

    // 1.1 Use contiguous memory allocation
    int *asentries = (int*) malloc(10*10*sizeof(int));

    int **as = (int**) malloc(10*sizeof(int*));

    for (int ix = 0; ix < 10; ++ix )
        as[ix] = asentries + 10*ix;
    
    // Write the matrix
    for (int ix = 0; ix < 10; ++ix)
        for (int jx = 0; jx < 10; ++jx)
            as[ix][jx] = ix * jx;

    //Print the matric for check
    for (int ix = 0; ix < 10; ++ix){
        for (int jx = 0; jx < 10; ++jx){
            printf("%i ", as[ix][jx]);
        }
        printf("\n");
    }

    // Write to the file
    fwrite(asentries, sizeof(int), 100, file);
    
    // Close the file
    fclose(file);

    // Free the dynamic allocated memory
    free(as);
    free(asentries);

}

  // Part 2: Read the file and check:
  /*The other one
    1. Reopens the file for reading,
    2. Reads the matrix from the file,
    3. Checks that the entry in the ix-th row and jx-th column equals ix*jx.*/


{
    // 2.1 Contiguous Memory
    // Reppsen the file
    FILE *file = fopen("a0.dat", "r");
    // Check if the file is successfully opened
    if (file == NULL){
        printf("Error opening file\n");
        return -1;
    }

    // Reads the matrix from the file
    int *b = (int*) malloc(10*10*sizeof(int));

    int **bs = (int**) malloc(10*sizeof(int));

    for (int ix = 0; ix < 10; ++ix){
        bs[ix] = b + 10*ix;
    }

    // Read the 100 integers in file to the pointer b 
    fread(b, sizeof(int), 100, file);

    // Checks the entry of the matrix
    for (int ix = 0; ix<10; ++ix){
        for (int jx = 0; jx < 10; ++jx){
            int c = ix * jx;
            if (bs[ix][jx] != c){
                printf("Error in row %i, column %i", ix, jx);
            }
            //printf("%i ", bs[ix][jx]);
        }
        //printf("\n");
    }

    // Close the file
    fclose(file);

    // Free the dynamic allocated memory
    free(bs);
    free(b);
}

}