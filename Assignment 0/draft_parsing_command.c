#include <stdio.h>
#include<string.h> 
#include <ctype.h>
/*Programs can be called with command line arguments. 
For example, a program printargs might be called as

./printargs -a2 -b4
These arguments are passed to the main function

int
main(
    int argc,
    char *argv[]
    )
as an array of null-terminated strings argv (argument vector), 
the number of which is argc (argument count). 
In the above example the argument count is 3 and the argument vector accordingly 
contains 3 strings "./printargs", "-a2", and "-b4".

Implement a program that

1. Accepts two arguments -aA and -bB for integers A and B in arbitrary order, 
i.e. -aA -bB and -bB -aA are both legitimate arguments when calling your program,
2. Converts A and B to integers, and
3. Writes to stdout the line "A is (INSERT A) and B is (INSERT B)".

For example calling

printargs -a2 -b4
or
printargs -b4 -a2
should output

A is 2 and B is 4
As a final remark observe that standard programs would equally accept "-a2" and "-a 2". 
When using standard solutions like POSIX getopt, 
this is automatically taken care of.*/

//as an array of null-terminated strings argv (argument vector), 
//the number of which is argc (argument count). 

/*
int main(int argc, char *argv[])
{
    printf("Starting Parsing Command\n");
    //1. Accepts two arguments -aA and -bB for integers A and B in arbitrary order, 
    //i.e. -aA -bB and -bB -aA are both legitimate arguments when 
    //calling your program,
    char arg[2];
    int num[2];
    int aj = 0; //index of the array arg to store the alphabet and numbers
    int nj = 0;
    int len; //a variable to store the string length of the input argument

    
    for (int i = 1; i < 2; ++i){
        str = argv[i];
        printf("%s\n", argv[i]);
        len = strlen(argv[i]);
        printf("length: %i\n", len);
        for (int ix = 0; ix < len; ++ix){
            str[ix] = tolower(str[ix]);
            printf("%c\n", str[ix]);}
        printf("%s\n", str);
    }

    for (int ix = 1; ix < argc; ++ix){
        printf("%s\n", argv[ix]);
        len = strlen(argv[ix]);

        // Used to store the arguments
        char *str;
        str = argv[ix];
        //Convert the input arguments to lowercase for comparison
        for (int ix = 0; ix < len; ++ix){
            str[ix] = tolower(str[ix]);
        }
        
        // command like -a -b 
        if (len == 2){
            int t = isalpha(str[1]);
            if (t){
                arg[j] = str[1];
                j++;
            }
        }
        //command like -aA -Ba -A3 -b3
        else if (len == 3){
            if (str[1] == str[2]){
                int t = isalpha(str[1]);
                if (t){
                    arg[j] = str[1];
                    j++;
                }
            }
            else{
                int t = isalpha(str[1]);
                if (t){
                    arg[j] = str[1];
                    j++;
                }

            }
        }
        
        len = strlen(argv[ix]);
        for(int i = 0; i < len; ++i){
            argv[ix] = tolower(argv[ix]);
        }
        // command like -a -b 
        if (len == 2){
            int t = isalpha(argv[ix][1]);
            if (t){
                arg[j] = argv[ix][1];
            }
        }


        printf("size of command: %lu\n", strlen(argv[ix]));
        printf("Single element expression: %c, %c\n", argv[ix][1], argv[ix][2]);
    }



    //2. Converts A and B to integers, and


    //3. Writes to stdout the line "A is (INSERT A) and B is (INSERT B)".



}*/