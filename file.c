/* file.c
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2022 Fumiama Minamoto.
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

#ifndef __cosmopolitan
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	#include <process.h>
#else
	#include <unistd.h>
	#include <sys/mman.h>
#endif
#endif
#include "base16384.h"

#ifdef __cosmopolitan
#define get_file_size(filepath) ((off_t)GetFileSize(filepath))
#else
static inline off_t get_file_size(const char* filepath) {
    struct stat statbuf;
    return stat(filepath, &statbuf)?-1:statbuf.st_size;
}
#endif

#define is_standard_io(filename) (*(uint16_t*)(filename) == *(uint16_t*)"-")

base16384_err_t base16384_encode_file(const char* input, const char* output, char* encbuf, char* decbuf) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	if(is_standard_io(input)) { // read from stdin
		inputsize = 0;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize < 0) {
		return base16384_err_get_file_size;
	}
	fpo = is_standard_io(output)?stdout:fopen(output, "wb");
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
		while((cnt = fread(encbuf, sizeof(char), inputsize, fp)) > 0) {
			int n = base16384_encode(encbuf, cnt, decbuf, outputsize);
			if(fwrite(decbuf, n, 1, fpo) <= 0) {
				return base16384_err_write_file;
			}
		}
		if(!is_standard_io(output)) fclose(fpo);
		if(!is_standard_io(input)) fclose(fp);
	#ifndef _WIN32
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd < 0) {
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
		if(!is_standard_io(output)) fclose(fpo);
		close(fd);
	}
	#endif
	return base16384_err_ok;
}

base16384_err_t base16384_encode_fp(FILE* input, FILE* output, char* encbuf, char* decbuf) {
	if(!input) {
		return base16384_err_fopen_input_file;
	}
	if(!output) {
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = BUFSIZ*1024/7*7;
	int outputsize = base16384_encode_len(inputsize)+16;
	size_t cnt = 0;
	fputc(0xFE, output);
	fputc(0xFF, output);
	while((cnt = fread(encbuf, sizeof(char), inputsize, input)) > 0) {
		int n = base16384_encode(encbuf, cnt, decbuf, outputsize);
		if(fwrite(decbuf, n, 1, output) <= 0) {
			return base16384_err_write_file;
		}
	}
	return base16384_err_ok;
}

base16384_err_t base16384_encode_fd(int input, int output, char* encbuf, char* decbuf) {
	if(input < 0) {
		return base16384_err_fopen_input_file;
	}
	if(output < 0) {
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = BUFSIZ*1024/7*7;
	int outputsize = base16384_encode_len(inputsize)+16;
	size_t cnt = 0;
	write(output, "\xfe\xff", 2);
	while((cnt = read(input, encbuf, inputsize)) > 0) {
		int n = base16384_encode(encbuf, cnt, decbuf, outputsize);
		if(write(output, decbuf, n) < n) {
			return base16384_err_write_file;
		}
	}
	return base16384_err_ok;
}

#define rm_head(fp) {\
	int ch = fgetc(fp);\
	if(ch == 0xFE) fgetc(fp);\
	else ungetc(ch, fp);\
}

#define skip_offset(input_file) ((input_file[0]==(char)0xFE)?2:0)

static inline int is_next_end(FILE* fp) {
	int ch = fgetc(fp);
	if(ch == EOF) return 0;
	if(ch == '=') return fgetc(fp);
	ungetc(ch, fp);
	return 0;
}

base16384_err_t base16384_decode_file(const char* input, const char* output, char* encbuf, char* decbuf) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	if(is_standard_io(input)) { // read from stdin
		inputsize = 0;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize < 0) {
		return base16384_err_get_file_size;
	}
	fpo = is_standard_io(output)?stdout:fopen(output, "wb");
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
		while((cnt = fread(decbuf, sizeof(char), inputsize, fp)) > 0) {
			if((end = is_next_end(fp))) {
				decbuf[cnt++] = '=';
				decbuf[cnt++] = end;
			}
			if(fwrite(encbuf, base16384_decode(decbuf, cnt, encbuf, outputsize), 1, fpo) <= 0) {
				return base16384_err_write_file;
			}
		}
		if(!is_standard_io(output)) fclose(fpo);
		if(!is_standard_io(input)) fclose(fp);
	#ifndef _WIN32
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd < 0) {
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
		if(!is_standard_io(output)) fclose(fpo);
		close(fd);
	}
	#endif
	return base16384_err_ok;
}

base16384_err_t base16384_decode_fp(FILE* input, FILE* output, char* encbuf, char* decbuf) {
	if(!input) {
		return base16384_err_fopen_input_file;
	}
	if(!output) {
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = BUFSIZ*1024/8*8;
	int outputsize = base16384_decode_len(inputsize, 0)+16;
	int cnt = 0;
	int end = 0;
	rm_head(input);
	while((cnt = fread(decbuf, sizeof(char), inputsize, input)) > 0) {
		if((end = is_next_end(input))) {
			decbuf[cnt++] = '=';
			decbuf[cnt++] = end;
		}
		if(fwrite(encbuf, base16384_decode(decbuf, cnt, encbuf, outputsize), 1, output) <= 0) {
			return base16384_err_write_file;
		}
	}
	return base16384_err_ok;
}

static inline int is_next_end_fd(int fd) {
	char ch = 0;
	read(fd, &ch, 1);
	if(ch == '=') {
		read(fd, &ch, 1);
	}
	return (int)ch;
}

base16384_err_t base16384_decode_fd(int input, int output, char* encbuf, char* decbuf) {
	if(input < 0) {
		return base16384_err_fopen_input_file;
	}
	if(output < 0) {
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = BUFSIZ*1024/8*8;
	int outputsize = base16384_decode_len(inputsize, 0)+16;
	int cnt = 0;
	int end = 0;
	decbuf[0] = 0;
	if(read(input, decbuf, 2) < 2) {
		return base16384_err_read_file;
	}
	if(decbuf[0] != (char)(0xfe)) cnt = 2;
	while((end = read(input, decbuf+cnt, inputsize-cnt)) > 0 || cnt > 0) {
		if(end > 0) {
			cnt += end;
			if((end = is_next_end_fd(input))) {
				decbuf[cnt++] = '=';
				decbuf[cnt++] = end;
				end = 0;
			} else end = 1;
		} else end = 0;
		cnt = base16384_decode(decbuf, cnt, encbuf, outputsize);
		if(write(output, encbuf, cnt) < cnt) {
			return base16384_err_write_file;
		}
		cnt = end;
	}
	return base16384_err_ok;
}
