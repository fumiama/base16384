#ifndef _FILE_TEST_H_
#define _FILE_TEST_H_

/* test/file_test.h
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
        int cnt; \
        while ((cnt = fread(&buf, 1, sizeof(sum_input), fp)) > 0) { \
            int n; \
            buf = 0; \
            while(cnt < sizeof(sum_input)) { \
                n = fread((uint8_t*)(&buf)+cnt, 1, 1, fp); \
                if (n) cnt++; \
                else break; \
            } \
            sum_input += buf; \
        } \
    } fclose(fp); \
    fp = fopen(TEST_VALIDATE_FILENAME, "rb"); { \
        loop_ok(!fp, i, "fopen"); \
        int cnt; \
        while ((cnt = fread(&buf, 1, sizeof(sum_validate), fp)) > 0) { \
            int n; \
            buf = 0; \
            while(cnt < sizeof(sum_validate)) { \
                n = fread((uint8_t*)(&buf)+cnt, 1, 1, fp); \
                if (n) cnt++; \
                else break; \
            } \
            sum_validate += buf; \
        } \
    } fclose(fp); \
    if (sum_input != sum_validate) { \
        fprintf(stderr, "loop @%d, expect: %016llx, got: %016llx: ", i, (unsigned long long)sum_input, (unsigned long long)sum_validate); \
        fputs(TEST_INPUT_FILENAME " and " TEST_VALIDATE_FILENAME " mismatch.", stderr); \
        return 1; \
    }

#define init_input_file() \
    for(i = 0; i < BASE16384_ENCBUFSZ; i += sizeof(int)) { \
        *(int*)(&encbuf[i]) = rand(); \
    } \
    fp = fopen(TEST_INPUT_FILENAME, "wb"); \
    ok(!fp, "fopen"); \
    ok(fwrite(encbuf, BASE16384_ENCBUFSZ, 1, fp) != 1, "fwrite"); \
    ok(fclose(fp), "fclose"); \
    fputs("input file created.\n", stderr);

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static ssize_t base16384_test_file_reader(const void *client_data, void *buffer, size_t count) {
    int fd = (int)((uintptr_t)client_data);
    ssize_t ret = read(fd, buffer, count);
    if(ret < 0) return ret;
    ssize_t i;
    for(i = 0; i < ret; i++) {
        ((uint8_t*)(buffer))[i] = ~((uint8_t*)(buffer))[i];
    }
    return ret;
}

static ssize_t base16384_test_file_writer(const void *client_data, const void *buffer, size_t count) {
    int fd = (int)((uintptr_t)client_data);
    if(count <= 0) {
        errno = EINVAL;
        return -100;
    }
    uint8_t* wbuf = (uint8_t*)malloc(count);
    if(!wbuf) return -200;
    ssize_t i;
    for(i = 0; i < count; i++) {
        wbuf[i] = ~((uint8_t*)(buffer))[i];
    }
    ssize_t ret = write(fd, buffer, count);
    int errnobak = errno;
    free(wbuf);
    errno = errnobak;
    return ret;
}

#endif