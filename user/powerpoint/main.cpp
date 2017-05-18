#include <iostream>
#include <stdio.h>
#include "BMP.h"

#include <new>
#include <unistd.h>
#include <window.h>
#include <errno.h>
using namespace std;

char buf[100];

void getImage(int n) {
    unsigned int nb = 0;
    buf[nb++] = '_';
    buf[nb++] = '-';
    unsigned int i = 1;
    const unsigned int base = 10;
    while(n/i >= base) {
        i *= base;
    }

    while(n > 0) {
        buf[nb++] = '0' + n/i;
        n %= i;
        i /= base;
    }
    while(i >= 1) {
        buf[nb++] = '0';
        i /= base;
    }
    buf[nb++] = '.';
    buf[nb++] = 'b';
    buf[nb++] = 'm';
    buf[nb++] = 'p';
    buf[nb++] = '\0';
}

int main(int argc, char** argv){
    fprintf(stderr,"powerpoint entry");
    printf("powerpoint entry\n");
    if(argc < 2) {
        printf("usage: powerpoint [directory]");
        return 1;
    }
    if(chdir(argv[1])) {
        printf("%s: no such file or directory\n", argv[1]);
        return 1;
    }

    vec_t size;
    size.x = 1024;
    size.y = 768;
    vec_t off;
    off.x = 0;
    off.y = 0;

    int w = openwin(size, off, 1);
    fprintf(stderr," w : %d %d\n",w,errno);

    int i = 0;

    evt_t e;
    int redraw = 1;
    while(1) {
        if(redraw) {
            getImage(i);
            printf("%d   %s\n", i, buf);

            BMP bmp;
            bmp.load(buf);
            bmp.to32();
            bmp.draw(w);
        }

        redraw = 0;
        while((e = getevt(w)).type == EVT_INVALID);
        if(e.type == EVT_KEYBOARD && !e.key.scanCode.release) {
            if(e.key.scanCode.code == K_ENTER) {
                redraw = 1;
                ++i;
            }
            if(e.key.scanCode.code == K_BACKSPACE) {
                redraw = 1;
                --i;
            }
        }

    }

#if 0
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
#endif
    return 0;
}
