
#ifdef _WIN32
	#include <io.h>
    #define ftruncate _chsize_s
#else
    #define _POSIX1_SOURCE 2
    #include <unistd.h>
#endif
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "base16384.h"
#include "binary.h"

#define TEST_SIZE (4096)
#define TEST_INPUT_FILENAME "wrap_test_input.bin"
#define TEST_OUTPUT_FILENAME "wrap_test_output.bin"
#define TEST_VALIDATE_FILENAME "wrap_test_validate.bin"

char encbuf[BASE16384_ENCBUFSZ];
char decbuf[BASE16384_DECBUFSZ];
char tstbuf[BASE16384_ENCBUFSZ];

#define ok(has_failed, reason) \
    if (has_failed) { \
        perror(reason); \
        return 1; \
    }

#define loop_ok(has_failed, i, reason) \
    if (has_failed) { \
        fprintf(stderr, "loop @%d: ", i); \
        perror(reason); \
        return 1; \
    }

#define reset_and_truncate(fd, i) { \
    fd = open(TEST_INPUT_FILENAME, O_RDWR); \
    ok(!fd, "open"); \
    loop_ok(lseek(fd, 0, SEEK_SET), i, "lseek"); \
    loop_ok(ftruncate(fd, i), i, "ftruncate"); \
}

#define base16384_loop_ok(err) \
    if (err) { \
        fprintf(stderr, "loop @%d: ", i); \
        base16384_perror(err); \
        return 1; \
    }

#define validate_result() \
    uint64_t buf, sum_input = 0, sum_validate = 0; \
    fp = fopen(TEST_INPUT_FILENAME, "rb"); { \
        loop_ok(!fp, i, "fopen"); \
        while (fread(&buf, sizeof(sum_input), 1, fp) == 1) sum_input += buf; \
        buf = 0; \
        while (fread(&buf, 1, 1, fp) == 1) { \
            sum_input += buf; \
            sum_input = LEFTROTATE(sum_input, 4); \
        } \
    } fclose(fp); \
    fp = fopen(TEST_VALIDATE_FILENAME, "rb"); { \
        loop_ok(!fp, i, "fopen"); \
        while (fread(&buf, sizeof(sum_validate), 1, fp) == 1) sum_validate += buf; \
        buf = 0; \
        while (fread(&buf, 1, 1, fp) == 1) { \
            sum_validate += buf; \
            sum_validate = LEFTROTATE(sum_validate, 4); \
        } \
    } fclose(fp); \
    if (sum_input != sum_validate) { \
        fprintf(stderr, "loop @%d, expect: %016llx, got: %016llx: ", i, (unsigned long long)sum_input, (unsigned long long)sum_validate); \
        fputs(TEST_INPUT_FILENAME " and " TEST_VALIDATE_FILENAME " mismatch.", stderr); \
        return 1; \
    }

#define init_input_file() \
    for(i = 0; i < TEST_SIZE; i += sizeof(int)) { \
        *(int*)(&encbuf[i]) = rand(); \
    } \
    fp = fopen(TEST_INPUT_FILENAME, "wb"); \
    ok(!fp, "fopen"); \
    ok(fwrite(encbuf, TEST_SIZE, 1, fp) != 1, "fwrite"); \
    ok(fclose(fp), "fclose"); \
    fputs("input file created.\n", stderr);

int main() {
    srand(time(NULL));

    FILE* fp;
    int fd, i;
    base16384_err_t err;

    fputs("testing base16384_en/decode_file...\n", stderr);
    init_input_file();
    for(i = TEST_SIZE; i > 0; i--) {
        reset_and_truncate(fd, i);
        loop_ok(close(fd), i, "close");

        err = base16384_encode_file(TEST_INPUT_FILENAME, TEST_OUTPUT_FILENAME, encbuf, decbuf);
        base16384_loop_ok(err);

        err = base16384_decode_file(TEST_OUTPUT_FILENAME, TEST_VALIDATE_FILENAME, encbuf, decbuf);
        base16384_loop_ok(err);

        validate_result();
    }

    fputs("testing base16384_en/decode_fp...\n", stderr);
    init_input_file();
    for(i = TEST_SIZE; i > 0; i--) {
        reset_and_truncate(fd, i);
        loop_ok(close(fd), i, "close");

        FILE* fpin = fopen(TEST_INPUT_FILENAME, "rb");
        loop_ok(!fpin, i, "fopen");

        FILE* fpout = fopen(TEST_OUTPUT_FILENAME, "wb+");
        loop_ok(!fpout, i, "fopen");

        err = base16384_encode_fp(fpin, fpout, encbuf, decbuf);
        base16384_loop_ok(err);

        loop_ok(fclose(fpin), i, "fclose");

        FILE* fpval = fopen(TEST_VALIDATE_FILENAME, "wb");
        loop_ok(!fpval, i, "fopen");

        rewind(fpout);

        err = base16384_decode_fp(fpout, fpval, encbuf, decbuf);
        base16384_loop_ok(err);

        loop_ok(fclose(fpout), i, "fclose");
        loop_ok(fclose(fpval), i, "fclose");

        validate_result();
    }

    fputs("testing base16384_en/decode_fd...\n", stderr);
    init_input_file();
    for(i = TEST_SIZE; i > 0; i--) {
        reset_and_truncate(fd, i);

        int fdout = open(TEST_OUTPUT_FILENAME, O_RDWR|O_TRUNC|O_CREAT|O_APPEND);
        loop_ok(!fdout, i, "open");

        err = base16384_encode_fd(fd, fdout, encbuf, decbuf);
        base16384_loop_ok(err);
        loop_ok(close(fd), i, "close");

        int fdval = open(TEST_VALIDATE_FILENAME, O_WRONLY|O_TRUNC|O_CREAT);
        loop_ok(!fdval, i, "open");

        loop_ok(lseek(fdout, 0, SEEK_SET), i, "lseek");

        err = base16384_decode_fd(fdout, fdval, encbuf, decbuf);
        base16384_loop_ok(err);

        loop_ok(close(fdout), i, "close");
        loop_ok(close(fdval), i, "close");

        validate_result();
    }

    remove(TEST_INPUT_FILENAME);
    remove(TEST_OUTPUT_FILENAME);
    remove(TEST_VALIDATE_FILENAME);

    return 0;
}
