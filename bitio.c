/* bitio.c
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2020-2024 Fumiama Minamoto.
 *
 * This program is distributed in MIT license.
 * Initially created at 20200416.
 */

#include "bitio.h"

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
