/* base16384.c
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2022 Fumiama Minamoto.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __cosmopolitan
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>
#endif
#endif
#include "base16384.h"

#ifdef __cosmopolitan
#define get_file_size(filepath) ((off_t)GetFileSize(filepath))
#else
static off_t get_file_size(const char* filepath) {
    struct stat statbuf;
    return stat(filepath, &statbuf)?-1:statbuf.st_size;
}
#endif

char encbuf[BUFSIZ*1024/7*7];
char decbuf[BUFSIZ*1024/8*8+2];

static int encode_file(const char* input, const char* output) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	if(*(uint16_t*)input == *(uint16_t*)"-") { // read from stdin
		inputsize = 0;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize < 0) {
		perror("Get file size error");
		return 1;
	}
	fpo = (*(uint16_t*)output == *(uint16_t*)"-")?stdout:fopen(output, "wb");
	if(!fpo) {
		perror("Fopen output file error");
		return 2;
	}
	if(!inputsize || inputsize > BUFSIZ*1024) { // stdin or big file, use encbuf & fread
		inputsize = BUFSIZ*1024/7*7;
		#ifdef _WIN32
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			perror("Fopen input file error");
			return 3;
		}

		int outputsize = encode_len(inputsize)+16;
		size_t cnt = 0;
		fputc(0xFE, fpo);
		fputc(0xFF, fpo);
		while((cnt = fread(encbuf, sizeof(char), inputsize, fp))) {
			int n = encode(encbuf, cnt, decbuf, outputsize);
			if(fwrite(decbuf, n, 1, fpo) <= 0) {
				perror("Write file error");
				return 4;
			}
		}
		/* 由操作系统负责释放资源
			fclose(fpo);
			fclose(fp);
		以缩短程序运行时间 */
	#ifndef _WIN32
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd <= 0) {
			perror("Open input file error");
			return 5;
		}
		char *input_file = mmap(NULL, (size_t)inputsize, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			perror("Map input file error");
			return 6;
		}
		int outputsize = encode_len(inputsize)+16;
		fputc(0xFE, fpo);
		fputc(0xFF, fpo);
		int n = encode(input_file, (int)inputsize, decbuf, outputsize);
		if(fwrite(decbuf, n, 1, fpo) <= 0) {
			perror("Write file error");
			return 7;
		}
		munmap(input_file, (size_t)inputsize);
		/* 由操作系统负责释放资源
			fclose(fpo);
			close(fd);
		以缩短程序运行时间 */	
	}
	#endif
	return 0;
}

#define rm_head(fp) {\
	int ch = fgetc(fp);\
	if(ch == 0xFE) fgetc(fp);\
	else rewind(fp);\
}

#define skip_offset(input_file) ((input_file[0]==(char)0xFE)?2:0)

static int is_next_end(FILE* fp) {
	int ch = fgetc(fp);
	if(ch == '=') return fgetc(fp);
	ungetc(ch, fp);
	return 0;
}

static int decode_file(const char* input, const char* output) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	if(*(uint16_t*)input == *(uint16_t*)"-") { // read from stdin
		inputsize = 0;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize < 0) {
		perror("Get file size error");
		return 1;
	}
	fpo = (*(uint16_t*)output == *(uint16_t*)"-")?stdout:fopen(output, "wb");
	if(!fpo) {
		perror("Fopen output file error");
		return 2;
	}
	if(!inputsize || inputsize > BUFSIZ*1024) { // stdin or big file, use decbuf & fread
		inputsize = BUFSIZ*1024/8*8;
		#ifdef _WIN32
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			perror("Fopen input file error");
			return 3;
		}
		int outputsize = decode_len(inputsize, 0)+16;
		int cnt = 0;
		int end = 0;
		rm_head(fp);
		while((cnt = fread(decbuf, sizeof(char), inputsize, fp))) {
			if((end = is_next_end(fp))) {
				decbuf[cnt++] = '=';
				decbuf[cnt++] = end;
			}
			if(fwrite(encbuf, decode(decbuf, cnt, encbuf, outputsize), 1, fpo) <= 0) {
				perror("Write file error");
				return 4;
			}
		}
		/* 由操作系统负责释放资源
			fclose(fpo);
			fclose(fp);
		以缩短程序运行时间 */
	#ifndef _WIN32
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd <= 0) {
			perror("Open input file error");
			return 5;
		}
		char *input_file = mmap(NULL, (size_t)inputsize, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			perror("Map input file error");
			return 6;
		}
		int outputsize = decode_len(inputsize, 0)+16;
		int off = skip_offset(input_file);
		if(fwrite(encbuf, decode(input_file+off, inputsize-off, encbuf, outputsize), 1, fpo) <= 0) {
			perror("Write file error");
			return 7;
		}
		munmap(input_file, (size_t)inputsize);
		/* 由操作系统负责释放资源
			fclose(fpo);
			close(fd);
		以缩短程序运行时间 */	
	}
	#endif
	return 0;
}

#ifndef _WIN32
unsigned long get_start_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

int main(int argc, char** argv) {
	if(argc != 4) {
        puts("Usage: -[e|d] <inputfile> <outputfile>");
        puts("\t-e encode");
        puts("\t-d decode");
		puts("\t<inputfile>  pass - to read from stdin");
		puts("\t<outputfile> pass - to write to stdout");
        exit(EXIT_SUCCESS);
    }
	#ifdef _WIN32
		clock_t t = clock();
	#else
		unsigned long t = get_start_ms();
	#endif
	int exitstat = 0;
	switch(argv[1][1]) {
		case 'e': exitstat = encode_file(argv[2], argv[3]); break;
		case 'd': exitstat = decode_file(argv[2], argv[3]); break;
		default: break;
	}
	if(!exitstat && *(uint16_t*)(argv[3]) != *(uint16_t*)"-") {
		#ifdef _WIN32
			printf("spend time: %lums\n", clock() - t);
		#else
			printf("spend time: %lums\n", get_start_ms() - t);
		#endif
	}
    return exitstat;
}
