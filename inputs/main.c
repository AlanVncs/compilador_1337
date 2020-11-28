#include <stdio.h>

int fatorial(int n);

int main(int l){
    char c;
    for(int i=0; i<10; i++){
        c+=i;
    }
    if(fatorial(100)) while(c=2, c<3){
        switch (c){
        case 1:
            c='a';
            continue;
        
        default:
            c=2.4;
            break;
        }
    }
    return 0;
}

int fatorial(int n){
    if(n<=0)
        return 1;
    else
        return n*fatorial(n-1);
}