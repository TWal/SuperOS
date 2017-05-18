#include <iostream>
#include <stdio.h>
#include "BMP.h"

#include <new>
#include <unistd.h>
#include <window.h>
using namespace std;

int main(int argc, char** argv){
    for(int i = 1 ; i < argc ; ++i){
        printf("loading %s\n",argv[i]);
        BMP bmp;
        bmp.load(argv[i]);
        bmp.to32();
        if(!bmp){
            cout << bmp.error() << endl;
        }
        vec_t size;
        size.x = bmp.w();
        size.y = bmp.h();
        vec_t off;
        off.x = 100;
        off.y = 100;


        int w = openwin(size,off,1);
        if(w == -1){
            printf("can't open window");
        }
        printf("opened window");
        bmp.draw(w);
    }
    while(true);
    return 0;
}
