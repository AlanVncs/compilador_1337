#include <stdio.h>

// SEMANTIC ERROR (8): variable 'fatorial' already declared at line 7.

int fatorial; // Escopo diferente (Não tem problema)

int main(int fatorial){
    int fatorial;
    return fatorial;
}