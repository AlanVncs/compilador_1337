#include <stdio.h>

// int oi(int x){
//     return ++x * 2;
// }

int printint(int x);

int main(int argc){
    int x=0;
    while(x<5){
        printint(x++);
    }
    return 0;
}


// int fatorial(int n){
//     if(n<=0)
//         return 1;
//     else
//         return n*fatorial(n-1);
// }