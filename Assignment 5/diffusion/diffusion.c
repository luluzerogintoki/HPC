#define _XOPEN_SOURCE 700

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//mpirun -n 1 ./diffusion -n20 -d0.02
// 1: number of processes
// -n20: number of iterations
// -d0.02: difussion constant

/*
1. Reads array size and initial values from a text file called "init".
2. Executes a given number of steps of heat diffusion with given diffusion constant, and
3. Outputs the average of temperatures, say X, as average: X.
5. Outputs the average absolute difference of each temperature to the average of 
all temperatures, say Y, as average absolute difference: Y.
*/

int main(int argc, char *argv[]){
    
    // Initialization of MPI
    MPI_Init(&argc, &argv);

    int nmb_mpi_proc, mpi_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nmb_mpi_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    // I. Parasing Arguments
    /* mpirun -n 1 ./diffusion -n20 -d0.02
    1: number of processes
    -n20: number of iterations
    -d0.02: difussion constant*/
	int iter;
	float diffusion;
    char *n,*d;
    
    if (argc != 3){
			printf("Invalid input!");
			exit(1);
		}
    
    for (int ix = 1; ix < argc; ++ix){
        n = strchr(argv[ix], 'n');
        d = strchr(argv[ix], 'd');
        if (n){
            iter = strtol(++n, NULL, 10);
        }
        else if (d){
            diffusion = atof(++d);;
        }
    }
    /*
    if (mpi_rank == 0){
        printf("Number of iterations:%d\n", iter);
        printf("Difussion constant:%f\n", diffusion);
    }*/

    // II. Read file
    int scatter_root = 0;//root that reand and split the data
    int reduce_root = 0;//root that get the results
    int width, height; // width and height
    int size;//total size of the box

    float* box;

    if (mpi_rank == scatter_root){
        FILE* file = fopen("init", "r");

        if (file == NULL){
            printf("Failed to open the file init.\n");
            exit(1);
        }
        /*
        3 3     : two positive integers,determine the width and the height. 
        1 1 1e6 : two integer values: valid coordinates; an initial value: a floating point number
        */
        //int width, height; // width and height
        int x, y;//the valid coordinates: x: row; y:col
        float initVal;//initial value

        fseek(file, 0, SEEK_SET); //to the beginning of the file
        fscanf(file, "%d %d", &height, &width);//get the width and height
        
        //printf("Width: %d, Height: %d\n", width, height);
        
        size = width * height;

        //Create and Initialize the box array
        box = (float *)malloc(size * sizeof(float));
        memset(box, 0, sizeof(float) * size);

        while(fscanf(file, "%d %d %f", &y, &x, &initVal) != EOF){
            box[y*width+x] = initVal;
        }
        
        fclose(file);
        /*
        for (int ix = 0; ix < size; ++ix){
            printf("position %d: %f\n", ix, box[ix]);
        }
        */

    }
    // Pass the value of width and height to all the processes
    MPI_Bcast(&width, 1, MPI_INT, scatter_root, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, scatter_root, MPI_COMM_WORLD);
    size = width * height;
    //MPI_Bcast(&size, 1, MPI_INT, scatter_root, MPI_COMM_WORLD);

    // III. Split the data(scatter) to other processes
    int num_active_proc = nmb_mpi_proc > height ? height : nmb_mpi_proc;
    // Only <= height processes will be used for the computation.
    
    //Compute the position and length passed for each process
    const int sz_loc = (height - 1) / nmb_mpi_proc + 1; //round the quotient up
    
    int pos, poss[nmb_mpi_proc];
    int len, lens[nmb_mpi_proc]; 
    // pos: local variable of the process to store the position of the data splited
    // local variable to store the length of the received data of process
    
    //Initialize poss and lens
    /*for (size_t ix = 0; ix < nmb_mpi_proc; ++ix){
        poss[ix] = 0;
        lens[ix] = 0;
    }*/

    if (mpi_rank == scatter_root){
        for (size_t jx=0, pos = 0; jx < nmb_mpi_proc; ++jx, pos += sz_loc){
            poss[jx] = pos;
            poss[jx] *= width;
            lens[jx] = pos + sz_loc <= height ? sz_loc: height - pos;
            lens[jx] *= width;

            if (lens[jx] <= 0){
                poss[jx] = 0;
                lens[jx] = 0;
                break;
            }
            //printf("process %d: len %d pos %d \n", jx, lens[jx], poss[jx]);
        }
        //printf("loc size:%d\n", sz_loc);
    }

    // Send out the positions and lens to each process
    MPI_Scatter(poss, 1, MPI_INT, &pos, 1, MPI_INT, scatter_root, MPI_COMM_WORLD);
    MPI_Scatter(lens, 1, MPI_INT, &len, 1, MPI_INT, scatter_root, MPI_COMM_WORLD);

    // local box variable to store the received data
    float* box_loc = (float *)malloc(sizeof(float) * len);
    memset(box_loc, 0, sizeof(float) * len);

    MPI_Scatterv(box, lens, poss, MPI_FLOAT, box_loc, len, MPI_FLOAT, scatter_root, MPI_COMM_WORLD);

    
    // IV. Do the computation
    // Get the number of lines the process need to compute
    int line = len / width;

    // Create the matrix to store the data and the adjacent lines (previous and next line)
    // For each row, add one value at the beginning and at the end respectively for convenient computation 
    float (*loc_1)[width+2] = (float (*)[width+2])malloc((line+2) * sizeof(float[width+2]));
    float (*loc_2)[width+2] = (float (*)[width+2])malloc((line+2) * sizeof(float[width+2]));//For updating the value
    float (*temp)[width+2] = NULL;
    memset(loc_1, 0, sizeof(float) * (line + 2) * (width + 2));
    //memset(loc_2, 0, sizeof(float) * (line + 2) * (width + 2));

    // Copy the value of data received into the corresponding position of the local array loc_1
    for (size_t ix = 0; ix < line; ++ix){
        memcpy(&loc_1[ix+1][1], box_loc+ix*width, sizeof(float)*width);
    }

    float sum_loc = 0.;
    float sum; //the total sum received at the reduction root
    
    // Start the computation and message exchange between each processes
    MPI_Status status;
    for (size_t ix = 0; ix < iter; ++ix){//number of iterations
        if (num_active_proc > 1){ // more than one process, need communication
            // Check if it is the first process (processing the first line)
            if (mpi_rank == 0){
               // send the last line to the next process
               MPI_Send(&loc_1[line][1], width, MPI_FLOAT, mpi_rank+1, 0, MPI_COMM_WORLD); 
               // receive the next line from the next process
               MPI_Recv(&loc_1[line+1][1], width, MPI_FLOAT, mpi_rank+1, 0, MPI_COMM_WORLD, &status);
            }
            // Check if it is the last process (processing the last line)
            else if (mpi_rank == num_active_proc - 1){
                // receive the previous line from the last process
                MPI_Recv(&loc_1[0][1], width, MPI_FLOAT, mpi_rank-1, 0, MPI_COMM_WORLD, &status);
                // send the first line to the previous process
                MPI_Send(&loc_1[1][1], width, MPI_FLOAT, mpi_rank-1, 0, MPI_COMM_WORLD);
            }
            // Check if number of processes is more than the height (total rows)
            else if (mpi_rank >= num_active_proc){
                //do not do any communication and computation
            }
            // Otherwise
            else{
                // Receivie the previous line
                MPI_Recv(&loc_1[0][1], width, MPI_FLOAT, mpi_rank-1, 0, MPI_COMM_WORLD, &status);
                // Send the last line to next process
                MPI_Send(&loc_1[line][1],  width, MPI_FLOAT, mpi_rank+1, 0, MPI_COMM_WORLD);
                // Receivie the next line
                MPI_Recv(&loc_1[line+1][1], width, MPI_FLOAT, mpi_rank+1, 0, MPI_COMM_WORLD, &status);                
                // Send the first line to previous process
                MPI_Send(&loc_1[1][1],  width, MPI_FLOAT, mpi_rank-1, 0, MPI_COMM_WORLD);        
            }
        }
        // Computation for diffusion
        for (size_t i = 1; i < line + 1; ++i){
            for (size_t j = 1; j < width + 1; ++j){
                //h(i,j) + c * ( (h(i-1,j) + h(i+1,j) + h(i,j-1) + h(i,j+1))/4 - h(i,j) )
                loc_2[i][j] = loc_1[i][j] + diffusion * ((loc_1[i-1][j] + loc_1[i+1][j] + loc_1[i][j-1] + loc_1[i][j+1])/4 - loc_1[i][j]);
            
                if (ix + 1 == iter){
                    sum_loc +=loc_2[i][j];
                }
            }
        }

        // Replace loc_1 by the value of loc_2 (updated value)
        temp = loc_2;
        loc_2 = loc_1;
        loc_1 = temp;
    }


    MPI_Allreduce(&sum_loc, &sum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    float avg = sum / size;

    float abs_sum_loc = 0;
    float abs_sum = 0;
    if (len > 0){
        for (size_t i = 1; i < line + 1; ++i){
            for (size_t j = 1; j < width + 1; ++j){
                float diff = loc_1[i][j] - avg;
                diff = (diff < 0.0 ? -diff : diff);
                abs_sum_loc += diff;
            }
        }
        //printf("process %d, local abs sum: %f\n", mpi_rank, abs_sum_loc);
    }


/*
    // Copy the value of loc_1(finished computation) to box_loc
    for (size_t ix = 0; ix < line; ++ix){
        memcpy(box_loc+ix*width, &loc_1[ix+1][1], sizeof(float)*width);
    }*/

    free(loc_1);
    free(loc_2);
    /// V. Receive the local sum & compute the average
    // Compute the local sum
    /*
    float sum_loc = 0.;
    float sum; //the total sum received at the reduction root
    for (int ix = 0; ix < len; ++ix){
        sum_loc += box_loc[ix];
    }*/
    //MPI_Reduce(&sum_loc, &sum, 1, MPI_FLOAT, MPI_SUM, scatter_root, MPI_COMM_WORLD);
    //MPI_Allreduce(&sum_loc, &sum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    //float avg = sum / size;


    // VI. Receive the local matrix and compute the average difference of each value
    //MPI_Scatterv(box, lens, poss, MPI_FLOAT, box_loc, sz_loc, MPI_FLOAT, scatter_root, MPI_COMM_WORLD);
    //MPI_Gatherv(box_loc, len, MPI_FLOAT, box, lens, poss, MPI_FLOAT, scatter_root, MPI_COMM_WORLD);
    /*
    int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm)
    */
    /*
    float abs_sum_loc = 0;
    float abs_sum = 0;
    if (len > 0){
        for (int ix = 0; ix < len; ++ix){
            float diff;
            if (box_loc[ix] > avg){
                diff = box_loc[ix] - avg;
            }
            else{
                diff = avg - box_loc[ix];
            }
            //double diff = box_loc[ix] - avg;
            //diff = (diff < 0.0 ? -diff : diff);
            //printf("diff:%f\n", diff);
            abs_sum_loc += diff;
        }
        //printf("process %d, local abs sum: %f\n", mpi_rank, abs_sum_loc);
    }
    */
    MPI_Reduce(&abs_sum_loc, &abs_sum, 1, MPI_FLOAT, MPI_SUM, scatter_root, MPI_COMM_WORLD);
    //float avg_abs = abs_sum / size;



    if (mpi_rank == scatter_root){
        /*
        float abs_sum = 0.;
        for (size_t ix = 0; ix < height; ++ix){
            for (size_t jx = 0; jx < width; jx++){
                abs_sum += fabsf(box[ix*width+jx] - avg);
            }
            //abs_sum += fabsf(box[ix] - avg);
        }
        float avg_abs = abs_sum / size;*/
        float avg_abs = abs_sum / size;
        printf("average: %f\n", avg);
        printf("average absolute difference: %f\n", avg_abs);
        free(box);
    }
    
    MPI_Finalize();

    return 0;


}

/*
./diffusion -n2 -d0.03333
Average: 509.781616
Average absolute difference: 1019.845764

*/