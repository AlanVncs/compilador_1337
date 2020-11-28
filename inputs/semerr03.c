#include <stdio.h>

// SEMANTIC ERROR (7): called object ‘fatorial’ is not a function or function pointer (previous declaration of 'fatorial' at line 3).

int fatorial;

int main(){
    return fatorial(5);
}