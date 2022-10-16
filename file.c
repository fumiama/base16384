/* file.c
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
#include <unistd.h>
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

base16384_err_t base16384_encode_file(const char* input, const char* output, char* encbuf, char* decbuf) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	if(*(uint16_t*)input == *(uint16_t*)"-") { // read from stdin
		inputsize = 0;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize < 0) {
		return base16384_err_get_file_size;
	}
	fpo = (*(uint16_t*)output == *(uint16_t*)"-")?stdout:fopen(output, "wb");
	if(!fpo) {
		return base16384_err_fopen_output_file;
	}
	if(!inputsize || inputsize > BUFSIZ*1024) { // stdin or big file, use encbuf & fread
		inputsize = BUFSIZ*1024/7*7;
		#ifdef _WIN32
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			return base16384_err_fopen_input_file;
		}

		int outputsize = base16384_encode_len(inputsize)+16;
		size_t cnt = 0;
		fputc(0xFE, fpo);
		fputc(0xFF, fpo);
		while((cnt = fread(encbuf, sizeof(char), inputsize, fp))) {
			int n = base16384_encode(encbuf, cnt, decbuf, outputsize);
			if(fwrite(decbuf, n, 1, fpo) <= 0) {
				return base16384_err_write_file;
			}
		}
		fclose(fpo);
		fclose(fp);
	#ifndef _WIN32
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd <= 0) {
			return base16384_err_open_input_file;
		}
		char *input_file = mmap(NULL, (size_t)inputsize, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			return base16384_err_map_input_file;
		}
		int outputsize = base16384_encode_len(inputsize)+16;
		fputc(0xFE, fpo);
		fputc(0xFF, fpo);
		int n = base16384_encode(input_file, (int)inputsize, decbuf, outputsize);
		if(fwrite(decbuf, n, 1, fpo) <= 0) {
			return base16384_err_write_file;
		}
		munmap(input_file, (size_t)inputsize);
		fclose(fpo);
		close(fd);
	}
	#endif
	return base16384_err_ok;
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

base16384_err_t base16384_decode_file(const char* input, const char* output, char* encbuf, char* decbuf) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	if(*(uint16_t*)input == *(uint16_t*)"-") { // read from stdin
		inputsize = 0;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize < 0) {
		return base16384_err_get_file_size;
	}
	fpo = (*(uint16_t*)output == *(uint16_t*)"-")?stdout:fopen(output, "wb");
	if(!fpo) {
		return base16384_err_fopen_output_file;
	}
	if(!inputsize || inputsize > BUFSIZ*1024) { // stdin or big file, use decbuf & fread
		inputsize = BUFSIZ*1024/8*8;
		#ifdef _WIN32
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			return base16384_err_fopen_input_file;
		}
		int outputsize = base16384_decode_len(inputsize, 0)+16;
		int cnt = 0;
		int end = 0;
		rm_head(fp);
		while((cnt = fread(decbuf, sizeof(char), inputsize, fp))) {
			if((end = is_next_end(fp))) {
				decbuf[cnt++] = '=';
				decbuf[cnt++] = end;
			}
			if(fwrite(encbuf, base16384_decode(decbuf, cnt, encbuf, outputsize), 1, fpo) <= 0) {
				return base16384_err_write_file;
			}
		}
		fclose(fpo);
		fclose(fp);
	#ifndef _WIN32
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd <= 0) {
			return base16384_err_open_input_file;
		}
		char *input_file = mmap(NULL, (size_t)inputsize, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			return base16384_err_map_input_file;
		}
		int outputsize = base16384_decode_len(inputsize, 0)+16;
		int off = skip_offset(input_file);
		if(fwrite(encbuf, base16384_decode(input_file+off, inputsize-off, encbuf, outputsize), 1, fpo) <= 0) {
			return base16384_err_write_file;
		}
		munmap(input_file, (size_t)inputsize);
		fclose(fpo);
		close(fd);
	}
	#endif
	return base16384_err_ok;
}
