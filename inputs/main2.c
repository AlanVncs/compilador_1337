#include <stdio.h>

int printint(int x);

int main(){    
    int counter=10;

    while (counter){
        printint(counter--);
    }

    return 0;
}