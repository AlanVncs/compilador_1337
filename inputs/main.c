#include <stdio.h>

int printint(int x);
int faz_calc(int n1, int n2);

int main(){
    int num=faz_calc(7, 2);
    printint(num);

    return 0;
}

int faz_calc(int n1, int n2){
    printint(n1);
    printint(n2);
    return n1 + n2;
}