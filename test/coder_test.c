#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "base16384.h"

char encbuf[4096+16];
char decbuf[4096/7*8+16];
char tstbuf[4096+16];

#define loop_diff(target) \
    for(i = start; i < end; i++) { \
        if (encbuf[i] != tstbuf[i]) { \
            if(n) { \
                fprintf(stderr, " @%d", i); \
                n = 0; \
            } \
            fprintf(stderr, " %02x", (uint8_t)(target[i])); \
        } else if(!n) { \
            n = 1; \
            fprintf(stderr, " ..."); \
        } \
    }

#define return_error(i, n) { \
    int end = i; \
    int start; \
    for(start = 0; start < end; start++) { \
        if(encbuf[start] != tstbuf[start]) break; \
    } \
    fprintf(stderr, "result mismatch @ loop %d, decsz: %d, first diff @ %d\n", i, n, start); \
    fprintf(stderr, "expect"); \
    n = 1; \
    loop_diff(encbuf); \
    fprintf(stderr, "\ngot   "); \
    n = 1; \
    loop_diff(tstbuf); \
    fputc('\n', stderr); \
    return 1; \
}

int main() {
    srand(time(NULL));
    int i, n;
    for(i = 0; i < 4096; i += sizeof(int)) {
        *(int*)(&encbuf[i]) = rand();
    }
    fputs("testing base16384_encode/base16384_decode...\n", stderr);
    for(i = 0; i < 4096; i++) {
        n = base16384_encode(encbuf, i, decbuf);
        n = base16384_decode(decbuf, n, tstbuf);
        int decn = n;
        if (memcmp(encbuf, tstbuf, n)) return_error(i, n);
    }
    fputs("testing base16384_encode_unsafe/base16384_decode_unsafe...\n", stderr);
    for(i = 0; i < 4096; i++) {
        n = base16384_encode_unsafe(encbuf, i, decbuf);
        n = base16384_decode_unsafe(decbuf, n, tstbuf);
        if ((n = memcmp(encbuf, tstbuf, n))) return_error(i, n);
    }
    return 0;
}
