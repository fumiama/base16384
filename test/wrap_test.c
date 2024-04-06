/* test/wrap_test.c
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2022-2024 Fumiama Minamoto.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "file_test.h"

#define TEST_SIZE (4096)
#define TEST_INPUT_FILENAME "wrap_test_input.bin"
#define TEST_OUTPUT_FILENAME "wrap_test_output.bin"
#define TEST_VALIDATE_FILENAME "wrap_test_validate.bin"

static char encbuf[BASE16384_ENCBUFSZ];
static char decbuf[BASE16384_DECBUFSZ];
static char tstbuf[BASE16384_ENCBUFSZ];

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
