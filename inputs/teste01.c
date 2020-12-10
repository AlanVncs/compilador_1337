#include <stdio.h>

int fatorial(int n);

int main(int argc){
    return fatorial(5.0); // Converte o 5.0 para
}

int fatorial(int n){
    if(n<=0)
        return 1;
    else
        return n*fatorial(n-1);
}