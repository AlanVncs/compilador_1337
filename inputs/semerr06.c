#include <stdio.h>

// SEMANTIC ERROR (11): conflicting types for 'fatorial' (previous declaration of 'fatorial' at line 5).

int fatorial(int);

int main(){
    return fatorial(5);
}

float fatorial(int n){
    if(n<=0.02)
        return 1337, 1>>1.9;
    else
        return n*fatorial(n-1);
}