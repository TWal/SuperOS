#include <unistd.h>


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
    for(volatile int i = 0 ; i < 10 ; ++i){
        systesti(i);
        for(volatile int j = 0 ; j < 10000000; ++j);
    }
    int pid = fork();
    if(pid){
        systesti(1000+pid);
    }
    else{
        systesti(55);
        for(volatile int i = 0 ; i < 5 ; ++i){
            systesti(i+100);
            for(volatile int j = 0 ; j < 10000000; ++j);
        }
        return 13;
    }
    for(volatile int i = 0 ; i < 20 ; ++i){
        systesti(i+500);
        for(volatile int j = 0 ; j < 10000000; ++j);
    }
    return factorial(5);
}
