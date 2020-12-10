#include <stdio.h>

int printint(int x);

int main(){    
    int counter=20;
    int tester=15;

    while (counter>0){
        if(counter-tester==0){
            printint(counter);
            return 0;
        }
        printint(counter);
        counter-=1;
    }

    return 0;
}