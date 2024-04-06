#ifndef _BITIO_H_
#define _BITIO_H_

/* bitio.h
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2020-2024 Fumiama Minamoto.
 *
 * This program is distributed in MIT license.
 * Initially created at 20200413.
 */

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
