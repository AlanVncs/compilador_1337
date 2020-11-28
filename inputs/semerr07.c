#include <stdio.h>

// SEMANTIC ERROR (12): redefinition of ‘fatorial’ (previous definition of 'fatorial' at line 5).

int fatorial(int n){
    if(n<=0)
        return 1;
    else
        return n*fatorial(n-1);
}

int fatorial(int n){
    if(n<=0)
        return 1;
    else
        return n*fatorial(n-1);
}

int main(){
    return fatoriall(5);
}