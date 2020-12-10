#include <stdio.h>

/* 
    Esses testes soh funcionam pro parser
*/

int main(int argc){
    int x = 10;
    switch(x){
        case 1: {
            x+=10;
            break;
        }
        case 2: x*=5; break;

        default: return x;
    }
    
    return 0;
}