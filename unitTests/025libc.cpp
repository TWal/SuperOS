#include"../src/utility.h"
#include <stdlib.h>

void unittest() {
    const char* s = "Hello World!\n";
    const char* s2 = "Hello Worlc!\n";
    printf("%lu\n", strlen(s));
    printf("%d\n", strcmp(s, s2));
    printf("%d\n", strcmp(s2, s));
    printf("%d\n", strcmp(s, s));
}
