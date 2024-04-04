#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "base16384.h"

#define TEST_SIZE (4096)

char encbuf[TEST_SIZE+16];
char decbuf[TEST_SIZE/7*8+16];
char tstbuf[TEST_SIZE+16];

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

#define test_batch(encode, decode) \
    fputs("testing base16384_"#encode"/base16384_"#decode"...\n", stderr); \
    for(i = 0; i <= TEST_SIZE; i++) { \
        n = base16384_##encode(encbuf, i, decbuf); \
        n = base16384_##decode(decbuf, n, tstbuf); \
        if (memcmp(encbuf, tstbuf, n)) return_error(i, n); \
    }

int main() {
    srand(time(NULL));
    int i, n;
    for(i = 0; i <= TEST_SIZE; i += sizeof(int)) {
        *(int*)(&encbuf[i]) = rand();
    }

    test_batch(encode, decode);
    test_batch(encode, decode_unsafe);
    test_batch(encode, decode_safe);

    test_batch(encode_unsafe, decode);
    test_batch(encode_unsafe, decode_unsafe);
    test_batch(encode_unsafe, decode_safe);

    test_batch(encode_safe, decode);
    test_batch(encode_safe, decode_unsafe);
    test_batch(encode_safe, decode_safe);
    return 0;
}
