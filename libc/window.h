#ifndef __SUPOS_WINDOW_H
#define WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct{
        unsigned int x;
        unsigned int y;
    } vec_t;

    int openwin(vec_t size, vec_t offset, int workspace);
    int opentwin(vec_t size, vec_t offset, int workspace);
    int resizewin(int fd, vec_t size, vec_t offset);



#ifdef __cplusplus
}
#endif

#endif
