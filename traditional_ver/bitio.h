#ifndef _BITIO_H_
#define _BITIO_H_
//bitio.h
//made by fumiama
//20200413
#include <stdio.h>
#include <string.h>
#ifndef BITBUFSIZE
#define BITBUFSIZE 1024
#endif
struct BIT{
    char b[BITBUFSIZE];
    int p;
};
typedef struct BIT BIT;

int pushbit(BIT *buffer, const int isture);

int fpushbit(BIT *buffer, FILE *fp);

int changebit(BIT *buffer, const int isture, const int position);

int readbit(const BIT *buffer, const int position);

int popbit(BIT *buffer);

#endif