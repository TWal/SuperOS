typedef void(*funcp)();
extern funcp __init_array_start;
extern funcp __init_array_end;
void __cppinit(){
    funcp *beg = &__init_array_start, *end = & __init_array_end;
    for (funcp*p = beg; p < end; ++p){
        (*p)();
    }
}
