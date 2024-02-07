#include <stdio.h>  
#include <stdlib.h>

int main()
{
    {   
    //Opens a file for writing
    FILE *file =  fopen("a0_2.dat", "w");

    //Check if the file is successfuly opened or not
    if (file == NULL){
        printf("Error opening file\n");
        return -1;
    }
    // 1.2 Use noncontiguous memory alloca tion
    int **as = (int**) malloc(10*sizeof(int*));
    for (int ix = 0; ix < 10; ++ix){
        as[ix] = (int*) malloc(10*sizeof(int));
    }

    
    for (int ix = 0; ix < 10; ++ix){
        for (int jx = 0; jx < 10; ++jx){
            as[ix][jx] = ix * jx;
        }
    }

    
    //Print the matrix for check
    printf("Matrix content check for writing.\n");
    for (int ix = 0; ix < 10; ++ix){
        for (int jx = 0; jx < 10; ++jx){
            printf("%i ", as[ix][jx]);
        }
        printf("\n");
    }

    // Write to the file
    for (int ix = 0; ix < 10; ++ix)
        fwrite(as[ix], sizeof(int), 10, file);
    // Close the file
    fclose(file);
    // Free the dynamic allocated memory
    
    for (int ix = 0; ix < 10; ++ix)
        free(as[ix]);
    free(as);
    }
    
    {
    // 2.2 Noncontiguous Memory
    // Reppsen the file
    FILE *file = fopen("a0_2.dat", "r");
    // Check if the file is successfully opened
    if (file == NULL){
        printf("Error opening file\n");
        return -1;
    }

    // Reads the matrix from the file
    int **bs = (int**) malloc(10*sizeof(int*));

    for (int ix = 0; ix < 10; ++ix){
        bs[ix] = (int*) malloc(10*sizeof(int));
    }



    // Read total 100 integers in file to the pointer bs
    for (int ix = 0; ix < 10; ++ix)
        fread(bs[ix], sizeof(int), 10, file);

    //Print the matrix for check
    printf("Matrix content check for reading.\n");
    for (int ix = 0; ix < 10; ++ix){
        for (int jx = 0; jx < 10; ++jx){
            printf("%i ", bs[ix][jx]);
        }
        printf("\n");
    }
    
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
    for (int ix = 0; ix < 10; ++ix)
        free(bs[ix]);
    
    free(bs);
    }
}