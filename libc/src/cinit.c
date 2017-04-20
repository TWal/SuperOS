void __initmalloc();

void __cinit(){ // all c runtime initialization
    __initmalloc();
}
