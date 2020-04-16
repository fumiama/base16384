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
#define setoffset(x, offset){\
    char flag = 0;\
    ch1 = ch2 = x;\
    while(!feof(fp) && (ch1 = fgetc(fp)) == x && (ch2 = fgetc(fp)) == x) {cnt8 -= offset; flag++;}\
    if(ch2 != x) {ungetc(ch2, fp); ch2 = x;}\
    if(ch1 != x) {ungetc(ch1, fp); ch1 = x;}\
    while(feof(fp) && buf[cnt - 1] == x && buf[cnt - 2] == x) {cnt8 -= offset; cnt -= 2; flag++;}\
    if(flag) cnt8 -= 8;\
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
        if(((outbuf.p % 8)?outbuf.p / 8 + 1:outbuf.p / 8) % 2){
            fputc(0, fpo);
            fputc('>', fpo);
            fputc('>', fpo);
        }
        for(int i = 0; i < outbuf.p % 8; i++) fputc('=', fpo);
    }
}
void decode(FILE *fp, FILE *fpo){
    int cnt = 0;
    int cnt8 = 0;
    int tc = 0;
    char ch1;
    char ch2;
    while((cnt = fread(buf, sizeof(char), BUFSIZ, fp))){
        cnt8 = cnt * 8;
        tc = cnt;
        setoffset('=', 14)
        setoffset('>', 16)

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
