#include <stdio.h>

// Stack allocation is the fastest variant.
// While the C standard makes no assumption on what memory is allocated
// on the stack, it is reasonable to assume that arrays of static size
// are allocated on the stack.

//Write a program employing the following allocation pattern
int main(){
//  int size = 10;
//  int size = 100;
//  int size = 1000;
  int size = 1000000000;

  printf("Size:%d\n", size);

  int as[size];

  //size_t: unsigned interger
  for (size_t ix=0; ix<size; ++ix){
    as[ix]=0;
  //  printf("%d", as[ix]);
  }  

//  printf("\n");

  printf("%d\n", as[0]);

  return 0;
}

//Compile and run the program. Then increase the size of the array, recompile and run,
// until the program fails with a segmentation fault.
// Explain this behavior assuming that that allocation was performed on the stack.

//Ans: Comparing with heap memory, access on stack memory is faster, while stack is limited in size,
//i.e. smaller storage space. Hence, when the size of the array increases, the stack may not have enough space
//to store the decalred  array,(no available stack space), the stack capacity is exceeded by the program,
// resulting in the program failure with a segmentation fault.

//Since heap has a larger space, using dynamic memory allocation to allocates memory on the heap can help solving this problem