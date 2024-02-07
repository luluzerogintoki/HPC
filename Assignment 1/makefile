.PHONY : all
all : gdb valgrind ass
gdb : GDB.c
        gcc -o gdb GDB.c -g -Og 
valgrind : Valgrind.c
        gcc -o valgrind Valgrind.c -g -Og -D
ass : assembler.c
gcc -o time naive_benchmarking.c -g   
gcc -O0 -o time naive_benchmarking.c 
gcc -O1 -o time naive_benchmarking.c
gcc -O2 -o time naive_benchmarking.c 
gcc -O3 -o time naive_benchmarking.c
gcc -Os -o time naive_benchmarking.c
gcc -Og -o time naive_benchmarking.c