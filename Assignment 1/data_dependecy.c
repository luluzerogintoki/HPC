#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


static inline void row_sums(
  double * sums,const double ** matrix,size_t nrs,size_t ncs)
{
  for ( size_t ix = 0; ix < nrs; ++ix ) {
    double sum = 0.;
    for ( size_t jx = 0; jx < ncs; ++jx )
      sum += matrix[ix][jx];
    sums[ix] = sum;
  }
}

static inline void row_sums_unrolled2(
  double * sums,const double ** matrix,size_t nrs,size_t ncs)
{
  for ( size_t ix = 0; ix < nrs; ++ix ) {
    double sum0 = 0.;
    double sum1 = 0.;
    for ( size_t jx = 0; jx < ncs; jx += 2 ) {
      sum0 += matrix[ix][jx];
      sum1 += matrix[ix][jx+1];
    }
    sums[ix] = sum0 + sum1;
  }
}

static inline void row_sums_unrolled4(
  double * sums,const double ** matrix,size_t nrs,size_t ncs)
{
  for ( size_t ix = 0; ix < nrs; ++ix ) {
    double sum0 = 0.;
    double sum1 = 0.;
    double sum2 = 0.;
    double sum3 = 0.;
    for ( size_t jx = 0; jx < ncs; jx += 4 ) {
      sum0 += matrix[ix][jx];
      sum1 += matrix[ix][jx+1];
      sum2 += matrix[ix][jx+2];
      sum3 += matrix[ix][jx+3];
    }
    sums[ix] = sum0 + sum1 + sum2 + sum3;
  }
}

static inline void row_sums_unrolled8(
  double * sums,const double ** matrix,size_t nrs,size_t ncs)
{
  for ( size_t ix = 0; ix < nrs; ++ix ) {
    double sum0 = 0.;
    double sum1 = 0.;
    double sum2 = 0.;
    double sum3 = 0.;
    double sum4 = 0.;
    double sum5 = 0.;
    double sum6 = 0.;
    double sum7 = 0.;
    for ( size_t jx = 0; jx < ncs; jx += 8 ) {
      sum0 += matrix[ix][jx];
      sum1 += matrix[ix][jx+1];
      sum2 += matrix[ix][jx+2];
      sum3 += matrix[ix][jx+3];
      sum4 += matrix[ix][jx+4];
      sum5 += matrix[ix][jx+5];
      sum6 += matrix[ix][jx+6];
      sum7 += matrix[ix][jx+7];
    }
    sums[ix] = sum0 + sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7;
  }
}

/*
creates 1000 x 1000 doubles,
stores them in a matrix in row major order,
computes the row sums.
*/

int main(int argc, char *argv[]){
    const int sz = 1000;
    const long int bench_iter = 5000;
    struct timespec bench_start_time;
    struct timespec bench_stop_time;
    double bench_diff_time;

    // Dynamic memory allocation the matrix
    double **matrix = (double **)malloc(sz * sizeof(double *));
    for (size_t ix = 0; ix < sz; ++ix){
        matrix[ix] = (double *)malloc(sz * sizeof(double));
    }
    // Initialize the matrix
    for (size_t ix = 0; ix < sz; ++ix) {
        for (size_t jx = 0; jx < sz; ++jx) {
            matrix[ix][jx] = (double)rand() / RAND_MAX;
        }
    }
    // Create the row and column sums array
    double *row_sum = (double *)malloc(sz * sizeof(double));


    // I. Use row_sums
    timespec_get(&bench_start_time, TIME_UTC);

    for (int bx = 0; bx < bench_iter; ++bx){
        row_sums(row_sum, (const double **)matrix, sz, sz);
    }

    timespec_get(&bench_stop_time, TIME_UTC);

    printf("A random element of row sum: %.4f\n", row_sum[30]);
    
    bench_diff_time = 
        difftime(bench_stop_time.tv_sec, bench_start_time.tv_sec) * 1000000
        + (bench_stop_time.tv_nsec - bench_start_time.tv_nsec) / 1000;
    printf("benchmark time for one iteration of row sum (row_sums): %fmus\n",
        bench_diff_time / bench_iter);

    // II. Use row_sums_unrolled2
    timespec_get(&bench_start_time, TIME_UTC);

    for (int bx = 0; bx < bench_iter; ++bx){
        row_sums_unrolled2(row_sum, (const double **)matrix, sz, sz);
    }

    timespec_get(&bench_stop_time, TIME_UTC);

    printf("A random element of row sum: %.4f\n", row_sum[30]);
    
    bench_diff_time = 
        difftime(bench_stop_time.tv_sec, bench_start_time.tv_sec) * 1000000
        + (bench_stop_time.tv_nsec - bench_start_time.tv_nsec) / 1000;
    printf("benchmark time for one iteration of row sum (row_sums_unrolled2): %fmus\n",
        bench_diff_time / bench_iter);

    // III. Use row_sums_unrolled4
    timespec_get(&bench_start_time, TIME_UTC);

    for (int bx = 0; bx < bench_iter; ++bx){
        row_sums_unrolled4(row_sum, (const double **)matrix, sz, sz);
    }

    timespec_get(&bench_stop_time, TIME_UTC);

    printf("A random element of row sum: %.4f\n", row_sum[30]);
    
    bench_diff_time = 
        difftime(bench_stop_time.tv_sec, bench_start_time.tv_sec) * 1000000
        + (bench_stop_time.tv_nsec - bench_start_time.tv_nsec) / 1000;
    printf("benchmark time for one iteration of row sum (row_sums_unrolled4): %fmus\n",
        bench_diff_time / bench_iter);

    // IV. Use row_sums_unrolled8
    timespec_get(&bench_start_time, TIME_UTC);

    for (int bx = 0; bx < bench_iter; ++bx){
        row_sums_unrolled8(row_sum, (const double **)matrix, sz, sz);
    }

    timespec_get(&bench_stop_time, TIME_UTC);

    printf("A random element of row sum: %.4f\n", row_sum[30]);
    
    bench_diff_time = 
        difftime(bench_stop_time.tv_sec, bench_start_time.tv_sec) * 1000000
        + (bench_stop_time.tv_nsec - bench_start_time.tv_nsec) / 1000;
    printf("benchmark time for one iteration of row sum (row_sums_unrolled8): %fmus\n",
        bench_diff_time / bench_iter);

    // Free allocated memory
    for (size_t ix = 0; ix < sz; ++ix){
        free(matrix[ix]);
    }
    free(matrix);
    free(row_sum);


    return 0;

}

/*
1. optimization level 0
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums): 1055.501200mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled2): 530.547600mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled4): 324.654600mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled8): 328.241400mus

2. optimization level 2
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums): 1031.806600mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled2): 498.948400mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled4): 303.353800mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled8): 302.903600mus


3. optimization level 2 including native architecture
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums): 1019.894800mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled2): 525.636400mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled4): 303.727600mus
A random element of row sum: 509.9507
benchmark time for one iteration of row sum (row_sums_unrolled8): 303.116800mus

*/