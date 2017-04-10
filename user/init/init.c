
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


int main(){
    //systest();
    for(volatile int i = 0 ; i < 10 ; ++i){
        systest();
        for(volatile int j = 0 ; j < 10000000; ++j);
    }
    return factorial(5);
}
