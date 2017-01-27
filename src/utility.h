#ifndef UTILITY_H
#define UTILITY_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;


#ifdef __cplusplus
extern "C" {
#endif

void outb(ushort port, uchar data);
int inb(ushort port);

#ifdef __cplusplus
}
#endif

#endif

