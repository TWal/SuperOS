#include <unistd.h>
#include <stdlib.h>


int factorial(int i){
    if (!i) return 1;
    else return i * factorial(i-1);
}

void systest(){
    asm volatile(
        "mov $42,%%rax;"
        "syscall": : :
        "%eax"
        );
}
void systesti(int i){
    (void)i;
    asm volatile(
        "mov $42,%%rax;"
        "syscall": : :
        "%eax"
        );
}


int main(){
    //systest();
    char* buffer = malloc(40);
    buffer[0] = 42;
    buffer[1] = -12;
    for(volatile int i = 0 ; i < 2 ; ++i){
        systesti(i);
        for(volatile int j = 0 ; j < 10000000; ++j);
    }
    int pid = fork();
    if(pid){
        buffer[0] = 56;
        systesti(buffer[0]);
        systesti(buffer[1]);
    }
    else{
        for(volatile int i = 0 ; i < 2 ; ++i){
            systesti(i+100);
            for(volatile int j = 0 ; j < 10000000; ++j);
        }
        free(buffer);
        buffer = malloc(5);
        buffer[0] = 89;
        return 13;
    }
    for(volatile int i = 0 ; i < 2 ; ++i){
        systesti(i+500);
        for(volatile int j = 0 ; j < 10000000; ++j);
    }
    return factorial(5);
}
