#include <stdio.h>
#include <stdlib.h>

void target(int a){
    printf("[+] test %02d\n", a);
}

int main(){
    int i;
    puts("Hello");
    for(i = 0; i < 10; ++i){
        target(i);
    }
}