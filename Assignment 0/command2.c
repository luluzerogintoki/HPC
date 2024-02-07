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

int main(int argc, char *argv[]){
    //An array to store the alphabet
    char alpha[2];
    int aj = 0;
    //An array to store the number
    int num[2];
    int nj = 0;

    if ((argc > 4) || (aj == 2) || (nj == 2)){
        printf("Too many arguments.");
        return -1;
    }

    for (int ix=1; ix<argc; ++ix){
        
        int len = strlen(argv[ix]); //the length of the input argument
        //Conversion to uppercase
        char *str;
        str = argv[ix];
        for (int ix = 0; ix < len; ++ix){
            str[ix] = toupper(str[ix]);
        }
        printf("After conversion: %s\n", str);

        // len == 1: a number
        if (len == 1){
            if(isdigit(str[0])){
                if (aj == 0){
                    printf("Wrong Input order");
                    return -1;
                }
                else{
                    num[nj] = str[0];
                    nj++;
                }
            }
        }
        // len == 2: a letter
        if (len == 2){
            int a = isalpha(str[1]);
            if (a){
                alpha[aj] = str[1];
                aj++;
            }
            else{
                printf("Wrong Input order");
                    return -1;
            }
        }

        // len == 3: either two same letter, or a letter followed by a number
        if (len == 3){
            //Two same letter
            if (str[1] == str[2] && isalpha(str[1])){
                alpha[aj] = str[1];
                aj++;
            } 
            // A letter followed by a number
            else if (isalpha(str[1]) && isdigit(str[2])){
                alpha[aj] = str[1];
                aj++;
                num[nj] = str[2];
                nj++;
            }
            else{
                printf("Invalid argument.");
                return -1;
            }
        }


        // len > 3
        if (len > 3){
            printf("Invalid argument.");
            return -1;
        }
    }

    printf("%c is %c and %c is %c", alpha[0], num[0], alpha[1], num[1]);
    return 0;

}