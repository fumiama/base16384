/* base16384.c
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2020-2024 Fumiama Minamoto.
 *
 * This program is distributed in MIT license.
 * Initially created at 20200416.
 */

#include <stdio.h>
#include <stdlib.h>
#include "bitio.h"

#define HEAD 0x4E
static BIT outbuf;
static unsigned char buf[BUFSIZ/8*8+2];

static inline void push(int istrue, FILE *fpo, int offset){
    if(!~pushbit(&outbuf, istrue)){
        if(offset) for(int i = 0; i < BITBUFSIZE; i += 2) outbuf.b[i] += offset;
        fwrite(outbuf.b, sizeof(char), BITBUFSIZE, fpo);
        outbuf.p = 0;
        pushbit(&outbuf, istrue);
    }
}

static void encode(FILE *fp, FILE *fpo){
    int cnt, prevcnt;

    fputc(0xFE, fpo);
    fputc(0xFF, fpo);
    while((cnt = fread(buf, sizeof(char), BUFSIZ/7*7, fp))){
        prevcnt = cnt;
        for(int i = 0; i < cnt * 8; i++){
            if(!(outbuf.p % 16)){
                push(0, fpo, HEAD);
                push(0, fpo, HEAD);
            }
            push(buf[i / 8] & (128u >> (i % 8)), fpo, HEAD);
        }
    }
    if(outbuf.p){
        outbuf.b[outbuf.p / 8] &= ~(255u >> outbuf.p % 8);
        for(int i = 0; i < outbuf.p / 8 + 1; i += 2) outbuf.b[i] += HEAD;
        fwrite(outbuf.b, sizeof(char), (outbuf.p % 8)?outbuf.p / 8 + 1:outbuf.p / 8 , fpo);
        if(((outbuf.p % 8)?outbuf.p / 8 + 1:outbuf.p / 8) % 2) fputc(0, fpo);
        if(outbuf.p % 16) {
            fputc('=', fpo);
            fputc(prevcnt%7, fpo);
        }
    }
}

static uint8_t map[] = {0, 6, 12, 4, 10, 2, 8};

static void decode(FILE *fp, FILE *fpo){
    int cnt;
    int cnt8;
    while((cnt = fread(buf, sizeof(char), BUFSIZ/8*8, fp))){
        cnt8 = cnt * 8;
        if(buf[cnt-2] == '=') {
            cnt8 -= 2*8 + map[buf[cnt-1]&7];
        } else if(cnt == BUFSIZ && !feof(fp)) {
            int ch;
            if((ch=fgetc(fp)) != '=') ungetc(ch, fp);
            else cnt8 -= map[fgetc(fp)&7];
        }
        for(int i = ((buf[0] == 0xFE)?16:0); i < cnt8; i++){
            if(!(i % 16)){
                buf[i / 8] -= HEAD;
                i += 2;
            }
            push(buf[i / 8] & (128u >> (i % 8)), fpo, 0);
        }
    }
    if(outbuf.p) fwrite(outbuf.b, sizeof(char), (outbuf.p % 8)?outbuf.p / 8 + 1:outbuf.p / 8, fpo);
}

#define CHOICE argv[1][1]
#define INPUT argv[2]
#define OUTPUT argv[3]

#define is_standard_io(filename) (*(uint16_t*)(filename) == *(uint16_t*)"-")

int main(int argc, char **argv){
    FILE *fp = NULL;
    FILE *fpo = NULL;
    
    if(argc != 4){
        fputs("base16384 [-ed] [inputfile] [outputfile]\n", stderr);
        fputs("  -e\t\tencode (default)\n", stderr);
        fputs("  -d\t\tdecode\n", stderr);
        fputs("  inputfile\tpass - to read from stdin\n", stderr);
        fputs("  outputfile\tpass - to write to stdout\n", stderr);
        exit(EXIT_FAILURE);
    }

    if(is_standard_io(INPUT)) {
        fp = stdin;
    } else {
        fp = fopen(INPUT, "rb");
        if(!fp){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }
    if(is_standard_io(OUTPUT)) {
        fpo = stdout;
    } else {
            fpo = fopen(OUTPUT, "wb");
        if(!fpo){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    switch(CHOICE){
        case 'e': encode(fp, fpo); break;
        case 'd': decode(fp, fpo); break;
        default: break;
    }

    if(!is_standard_io(INPUT)) fclose(fp);
    if(!is_standard_io(OUTPUT)) fclose(fpo);

    return 0;
}
