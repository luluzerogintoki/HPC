#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>

int main(int argc, char *argv[]){
    for (int ix = 1; ix < argc; ++ix){
        printf("%s\n", argv[ix]);
        int len = strlen(argv[ix]);
        //Conversion to lowercase
        char *str;
        str = argv[ix];
        for (int ix = 0; ix < len; ++ix){
            str[ix] = tolower(str[ix]);
        }
        for (int ix = 0; ix < len; ++ix){
            int d = isdigit(str[ix]);
            if (d){
                printf("Digit: %c\n", str[ix]);
            }
        } 
    }
}