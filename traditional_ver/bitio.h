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

int pushbit(BIT *buffer, const int isture){
    if(buffer->p >= BITBUFSIZE * 8) return EOF;
    else if(isture) buffer->b[buffer->p / 8] |= 128u >> buffer->p % 8;
    else buffer->b[buffer->p / 8] &= ~(128u >> buffer->p % 8);
    buffer->p++;
    return isture;
}

int fpushbit(BIT *buffer, FILE *fp){
    memset(buffer, 0, sizeof(BIT));
    if((buffer->p = fread(buffer->b, sizeof(char), BITBUFSIZE, fp)) && feof(fp))
        buffer->p = (buffer->p - 2) * 8 + buffer->b[buffer->p - 1] + 1;
    else buffer->p *= 8;
    return buffer->p;
}

int changebit(BIT *buffer, const int isture, const int position){
    if(position >= buffer->p) return EOF;
    else if(isture) buffer->b[position / 8] |= 128u >> position % 8;
    else buffer->b[position / 8] &= ~(128u >> position % 8);
    buffer->p++;
    return isture;
}

int readbit(const BIT *buffer, const int position){
    if(position >= buffer->p) return EOF;
    else return buffer->b[position / 8] & (128u >> position % 8);
}

int popbit(BIT *buffer){
    if(buffer->p >= BITBUFSIZE * 8 || buffer->p < 0) return EOF;
    buffer->p--;
    return buffer->b[(buffer->p+1) / 8] & (128u >> (buffer->p+1) % 8);
}
