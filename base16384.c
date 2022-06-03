//base16384.c
//MIT fumiama 20200416
#include <stdio.h>
#include <stdlib.h>
#include "bitio.h"

#define HEAD 0x4E
BIT outbuf;
unsigned char buf[BUFSIZ/8*8+2];

static void push(int istrue, FILE *fpo, int offset){
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
#define READ argv[2]
#define WRITE argv[3]

int main(int argc, char **argv){
    FILE *fp = NULL;
    FILE *fpo = NULL;
    
    if(argc != 4){
        fputs("Usage: -[e|d] inputfile outputfile\n", stderr);
        fputs("       -e encode\n", stderr);
        fputs("       -d decode\n", stderr);
        exit(EXIT_FAILURE);
    }

    fp = fopen(READ, "rb");
    fpo = fopen(WRITE, "wb");
    if(!fp || !fpo){
        fputs("IO error!", stderr);
        exit(EXIT_FAILURE);
    }
    switch(CHOICE){
        case 'e': encode(fp, fpo); break;
        case 'd': decode(fp, fpo); break;
        default: break;
    }
    return 0;
}
