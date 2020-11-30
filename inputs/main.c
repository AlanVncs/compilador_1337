#include <stdio.h>

int printint(int x);

int teste(int a){
    return a+1;
}

int main(){
    int x=10;
    x+=1;
    printint(x);
    x-=5;
    printint(x);
    x=teste(2);
    printint(x);
    return 0;
}











// int fatorial(int n){
//     if(n<=0)
//         return 1;
//     else
//         return n*fatorial(n-1);
// }