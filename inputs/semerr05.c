#include <stdio.h>

//SEMANTIC ERROR (7): 'fatorial' redeclared as different kind of symbol (previous declaration of 'fatorial' at line 5).

int fatorial(int);

int fatorial;

int main(){
    return fatorial(5);
}