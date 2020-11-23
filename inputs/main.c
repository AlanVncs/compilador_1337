#include <stdio.h>

int fatorial(int n);

int main(int argceta, char argv, char l){

    while(argceta) for(int l=8; l<19; l++){
        if(argv){
            return sizeof(float);
        }
        else{
            argceta = +argv*l;
            goto gg_iz;
        }
        argv;
    }


    gg_iz: switch(argv+5^2){
        case 4: {int a; return a;}
        case 5: {int b; return b;} break;
        default: return fatorial(5);
    }
}

int fatorial(int n){
    if(n<=0)
        return 1;
    else
        return n*fatorial(n-1);
}