#include <stdio.h>

// int oi(int x){
//     return ++x * 2;
// }

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

// int haha(int a, int b){
//     return 10;
// }

int main(int argc){    
    int x=2;
    int y=3;
    // int a=haha(1,2);
    // printint(a);
    // printint(y==2);

    int j;
    int l;
    j=teste(x);
    l=teste(y);

    printint(j);
    printint(l);

    return 0;
}


// int fatorial(int n){
//     if(n<=0)
//         return 1;
//     else
//         return n*fatorial(n-1);
// }