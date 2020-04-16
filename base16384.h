#include "bitio.h"
#define HEAD 0x4E
BIT outbuf;
unsigned char buf[BUFSIZ];
void push(int istrue, FILE *fpo, int offset){
    if(!~pushbit(&outbuf, istrue)){
        if(offset) for(int i = 0; i < BITBUFSIZE; i += 2) outbuf.b[i] += offset;
        fwrite(outbuf.b, sizeof(char), BITBUFSIZE, fpo);
        outbuf.p = 0;
        pushbit(&outbuf, istrue);
    }
}
void encode(FILE *fp, FILE *fpo){
    int cnt = 0;

    fputc(0xFE, fpo);
    fputc(0xFF, fpo);
    while((cnt = fread(buf, sizeof(char), BUFSIZ, fp))){
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
        for(int i = 0; i < outbuf.p % 8; i++) fputc('=', fpo);
    }
}
void decode(FILE *fp, FILE *fpo){
    int cnt = 0;
    char ch = '=';
    while((cnt = fread(buf, sizeof(char), BUFSIZ, fp))){
        int cnt8 = cnt * 8;
        while(!feof(fp) && (ch = fgetc(fp)) == '=') cnt8 -= 7;
        if(ch != '=') {ungetc(ch, fp); ch = '=';}
        while(feof(fp) && buf[cnt - 1] == '=' && buf[cnt - 2] == '=') {cnt8 -= 7; cnt -= 2;}
        if(cnt8 % 8) cnt8 -= 8;
        for(int i = ((buf[0] == 0xFE)?16:0); i < cnt8; i++){
            if(!(i % 16)){
                buf[i / 8] -= HEAD;
                i += 2;
            }
            push(buf[i / 8] & (128u >> (i % 8)), fpo, 0);
        }
    }
    if(outbuf.p) fwrite(outbuf.b, sizeof(char), outbuf.p / 8 + ((outbuf.p % 8)?1:0), fpo);
}
