#include <stdio.h>

int printint(int x);

int teste(int k){
    if (k==2){
        printint(k);
        return 1;
    } else {
        printint(k);
        return 2;
    }
}

int main(int argc){    
    int x=2;
    int y=3;

    int j;
    int l;
    j=teste(x);
    l=teste(y);

    printint(j);
    printint(l);

    return 0;
}