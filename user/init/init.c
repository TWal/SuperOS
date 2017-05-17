#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <window.h>
#include <string.h>


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


void thread(){
    printf("[th 2] Hey !\n");
    for(int i = 0 ; i < 20000000 ; ++i);
    printf("[th 2] will die now !\n");
    _texit(13);
}


int main(){
    printf("[Init] Init start\n");
    errno = 0;
    /*mkdir("/lol");
    chdir("/lol");
    printf("errno %d",errno);
    int fd = open("testfile",O_RDWR | O_CREAT);
    if(fd == -1){
        printf ("open failed %d %d",fd,errno);
    }
    FILE* f = fdopen(fd,"rw");

    fprintf(f,"hello %d\n",42);
    errno = 0;
    int i = seek(fd,0,SEEK_CUR);
    printf("errno %d",errno);
    printf("size %d\n",i);

    seek(fd,0,SEEK_SET);
    int c;
    while((c = fgetc(f)) != EOF){
        putchar(c);
        }*/

    /*vec_t size;
    size.x = 100;
    size.y = 200;
    vec_t offset;
    offset.x = 300;
    offset.y = 400;


    int fd = openwin(size,offset,1);
    printf("fd : %d, errno : %d",fd,errno);
    int* data = malloc(400);
    memset(data,-1,400);

    for(int i = 0 ; i < 200 ; ++ i){
        printf("CouCou");
        errno = 0;
        write(fd,data,400);
        printf(" errno : %d\n",errno);
        for(int i = 0 ; i < 10000 ; ++i);
        }*/

    /*int tfd = opentwin(size,offset,1);
    printf("fd : %d, errno : %d",tfd,errno);
    FILE* f = fdopen(tfd,"");
    fprintf(f,"Hello world!\n");*/


    /*while(1){
        evt_t evt = getevt(fd);
        if(evt.type != EVT_INVALID)
            printf("type : %d\n",evt.type);
        for(volatile int i = 0 ; i < 1000000 ; ++i);
        }*/

    char c = getchar();
    printf("char : %c",c);


    //   int pid = fork();



    /*void* test = malloc(1024);

    pid_t t = clone(thread,(char*)test +1024);
    printf("[init] Created thread %d\n",t);

    for(int i = 0 ; i < 10000000 ; ++i);

    pid_t pid = fork();
    if(pid == 0){
        printf("First Process");
        for(int i = 0 ; i < 10000000 ; ++i);
        return 56;
    }
    pid = fork();
    if(pid == 0){
        printf("Second Process");
        for(int i = 0 ; i < 10000000 ; ++i);
        return 57;
        }

    int status;
    errno = 0;
    printf("[init] before wait\n",errno);
    wait(&status);
    printf("[init] errno : %lld and status %d\n",errno,status);
    printf("[init] before wait 2\n",errno);
    wait(&status);
    printf("[init] errno : %lld and status %d\n",errno,status);

    printf("[init] end of test");

    while(1);

    _texit(42);*/


    printf("[Init] Init end\n");
    while(1);

    return factorial(5);
}
