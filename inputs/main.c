#include <stdio.h>

int printint(int n);
int fatorial(int n);
int r = 42;

int main(){
    int r = fatorial(5);
    printint(r);
}

int fatorial(int n){
    if(n<=0) return 1;
    else     return n*fatorial(n-1);
}