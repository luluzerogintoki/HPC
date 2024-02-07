#include <stdio.h>
#include <time.h>
#include <math.h>

//https://en.cppreference.com/w/c/chrono/clock_t

/*Implement a program that

1. In a loop repeats,
    the naive computation of the sum of the first billion integers,
2. Writes the result to stdout,
3. Takes the time before and after the loop,
4. Computes the runtime of a single loop iteration from the difference of start 
and stop time.

Compile and run the program with all possible optimization levels 
(add the flags -O0, -O1, -O2, -O3, -Os, and -Og to the compiler).

Discuss the timing results and speculate about the possible cause.*/

int main(){
    unsigned long long sum = 0;
    clock_t start = clock();

    for (size_t i = 0; i < 1000000000; ++i){
        sum+=i;
    }

    clock_t end = clock();
    
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
 
    printf("for loop took %f seconds to execute \n", cpu_time_used);

}

/*
gcc -o time naive_benchmarking.c      for loop took 0.959520 seconds to execute 
gcc -O0 -o time naive_benchmarking.c  for loop took 0.958923 seconds to execute 
gcc -O1 -o time naive_benchmarking.c  for loop took 0.000011 seconds to execute 
gcc -O2 -o time naive_benchmarking.c  for loop took 0.000005 seconds to execute 
gcc -O3 -o time naive_benchmarking.c  for loop took 0.000002 seconds to execute 
gcc -Os -o time naive_benchmarking.c  for loop took 0.000004 seconds to execute
gcc -Og -o time naive_benchmarking.c  for loop took 0.000002 seconds to execute 
*/

/*
1. -O0: No Optimization (-O0): With no optimization, the program's runtime is likely to be the longest. 
The compiler doesn't apply any significant optimizations, which means the code remains relatively unoptimized.

2. Low Optimizations (-O1, -O2): These optimization levels introduce some basic optimizations that 
may result in faster execution. However, the difference in runtime between -O1 and -O2 might not be 
substantial for this simple program.

3. High Optimizations (-O3): The -O3 optimization level enables aggressive optimizations that 
can significantly speed up the program. You'll likely see the fastest runtime with this level.

4. Size Optimization (-Os): This level optimizes for code size rather than execution speed. 
While it may reduce the program's binary size, it could slightly affect execution speed.

5. Debugging Optimization (-Og): This level is primarily for debugging, so it may not result 
in significant speed improvements.*/