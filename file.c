/* file.c
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

#ifndef __cosmopolitan
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
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
#include "binary.h"

#ifdef __cosmopolitan
#define get_file_size(filepath) ((off_t)GetFileSize(filepath))
#else
static inline off_t get_file_size(const char* filepath) {
    struct stat statbuf;
    return stat(filepath, &statbuf)?-1:statbuf.st_size;
}
#endif

#define is_standard_io(filename) (*(uint16_t*)(filename) == *(uint16_t*)"-")

static inline uint32_t calc_sum(uint32_t sum, size_t cnt, char* encbuf) {
	uint32_t i;
	#ifdef DEBUG
		fprintf(stderr, "cnt: %zu, roundin: %08x, ", cnt, sum);
	#endif
	for(i = 0; i < cnt/sizeof(sum); i++) {
		#ifdef DEBUG
			if (!i) {
				fprintf(stderr, "firstval: %08x, ", htobe32(((uint32_t*)encbuf)[i]));
			}
		#endif
		sum += ~LEFTROTATE(htobe32(((uint32_t*)encbuf)[i]), encbuf[i*sizeof(sum)]%(8*sizeof(sum)));
	}
	#ifdef DEBUG
		fprintf(stderr, "roundmid: %08x", sum);
	#endif
	size_t rem = cnt % sizeof(sum);
	if(rem) {
		uint32_t x = htobe32(((uint32_t*)encbuf)[i]) & (0xffffffff << (8*(sizeof(sum)-rem)));
		sum += ~LEFTROTATE(x, encbuf[i*sizeof(sum)]%(8*sizeof(sum)));
		#ifdef DEBUG
			fprintf(stderr, ", roundrem:%08x\n", sum);
		#endif
	}
	#ifdef DEBUG
		else fprintf(stderr, "\n");
	#endif
	return sum;
}

static inline uint32_t calc_and_embed_sum(uint32_t sum, size_t cnt, char* encbuf) {
	sum = calc_sum(sum, cnt, encbuf);
	if(cnt%7) { // last encode
		*(uint32_t*)(&encbuf[cnt]) = htobe32(sum);
	}
	return sum;
}

static inline int calc_and_check_sum(uint32_t* s, size_t cnt, char* encbuf) {
	uint32_t sum = calc_sum(*s, cnt, encbuf);
	if(cnt%7) { // is last decode block
		int shift = (int[]){0, 26, 20, 28, 22, 30, 24}[cnt%7];
		uint32_t sum_read = be32toh((*(uint32_t*)(&encbuf[cnt]))) >> shift;
		sum >>= shift;
		#ifdef DEBUG
			fprintf(stderr, "cntrm: %lu, mysum: %08x, sumrd: %08x\n", cnt%7, sum, sum_read);
		#endif
		return sum != sum_read;
	}
	*s = sum;
	return 0;
}

#define goto_base16384_file_detailed_cleanup(method, reason, dobeforereturn) { \
	errnobak = errno; \
	retval = reason; \
	dobeforereturn; \
	goto base16384_##method##_file_detailed_cleanup; \
}

base16384_err_t base16384_encode_file_detailed(const char* input, const char* output, char* encbuf, char* decbuf, int flag) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	int errnobak = 0;
	base16384_err_t retval = base16384_err_ok;
	if(!input || !output || strlen(input) <= 0 || strlen(output) <= 0) {
		errno = EINVAL;
		return base16384_err_invalid_file_name;
	}
	if(is_standard_io(input)) { // read from stdin
		inputsize = _BASE16384_ENCBUFSZ;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize <= 0) {
		if(!inputsize) errno = EINVAL;
		return base16384_err_get_file_size;
	}
	fpo = is_standard_io(output)?stdout:fopen(output, "wb");
	if(!fpo) {
		return base16384_err_fopen_output_file;
	}
	if(inputsize >= _BASE16384_ENCBUFSZ) { // stdin or big file, use encbuf & fread
		inputsize = _BASE16384_ENCBUFSZ;
		#if defined _WIN32 || defined __cosmopolitan
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			goto_base16384_file_detailed_cleanup(encode, base16384_err_fopen_input_file, {});
		}

		if(!(flag&BASE16384_FLAG_NOHEADER)) {
			fputc(0xFE, fpo);
			fputc(0xFF, fpo);
		}
		#ifdef DEBUG
			inputsize = 917504;
			fprintf(stderr, "inputsize: %lld\n", inputsize);
		#endif
		size_t cnt;
		while((cnt = fread(encbuf, sizeof(char), inputsize, fp)) > 0) {
			if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_and_embed_sum(sum, cnt, encbuf);
			int n = base16384_encode_unsafe(encbuf, cnt, decbuf);
			if(n && fwrite(decbuf, n, 1, fpo) <= 0) {
				goto_base16384_file_detailed_cleanup(encode, base16384_err_write_file, {});
			}
		}
	#if !defined _WIN32 && !defined __cosmopolitan
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd < 0) {
			goto_base16384_file_detailed_cleanup(encode, base16384_err_open_input_file, {});
		}
		char *input_file = mmap(NULL, (size_t)inputsize+16, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			goto_base16384_file_detailed_cleanup(encode, base16384_err_map_input_file, close(fd));
		}
		if(!(flag&BASE16384_FLAG_NOHEADER)) {
			fputc(0xFE, fpo);
			fputc(0xFF, fpo);
		}
		int n = base16384_encode(input_file, (int)inputsize, decbuf);
		if(n && fwrite(decbuf, n, 1, fpo) <= 0) {
			goto_base16384_file_detailed_cleanup(encode, base16384_err_write_file, {
				munmap(input_file, (size_t)inputsize+16);
				close(fd);
			});
		}
		munmap(input_file, (size_t)inputsize+16);
		close(fd);
	}
	#endif
base16384_encode_file_detailed_cleanup:
	if(fpo && !is_standard_io(output)) fclose(fpo);
	if(fp && !is_standard_io(input)) fclose(fp);
	if(errnobak) errno = errnobak;
	return retval;
}

base16384_err_t base16384_encode_fp_detailed(FILE* input, FILE* output, char* encbuf, char* decbuf, int flag) {
	if(!input) {
		return base16384_err_fopen_input_file;
	}
	if(!output) {
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = _BASE16384_ENCBUFSZ;
	size_t cnt = 0;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	if(!(flag&BASE16384_FLAG_NOHEADER)) {
		fputc(0xFE, output);
		fputc(0xFF, output);
	}
	while((cnt = fread(encbuf, sizeof(char), inputsize, input)) > 0) {
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_and_embed_sum(sum, cnt, encbuf);
		int n = base16384_encode_unsafe(encbuf, cnt, decbuf);
		if(n && fwrite(decbuf, n, 1, output) <= 0) {
			return base16384_err_write_file;
		}
	}
	return base16384_err_ok;
}

base16384_err_t base16384_encode_fd_detailed(int input, int output, char* encbuf, char* decbuf, int flag) {
	if(input < 0) {
		return base16384_err_fopen_input_file;
	}
	if(output < 0) {
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = _BASE16384_ENCBUFSZ;
	size_t cnt = 0;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	if(!(flag&BASE16384_FLAG_NOHEADER)) write(output, "\xfe\xff", 2);
	while((cnt = read(input, encbuf, inputsize)) > 0) {
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_and_embed_sum(sum, cnt, encbuf);
		int n = base16384_encode_unsafe(encbuf, cnt, decbuf);
		if(n && write(output, decbuf, n) < n) {
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

base16384_err_t base16384_decode_file_detailed(const char* input, const char* output, char* encbuf, char* decbuf, int flag) {
	off_t inputsize;
	FILE* fp = NULL;
	FILE* fpo;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	base16384_err_t retval = base16384_err_ok;
	int errnobak = 0;
	if(!input || !output || strlen(input) <= 0 || strlen(output) <= 0) {
		errno = EINVAL;
		return base16384_err_invalid_file_name;
	}
	if(is_standard_io(input)) { // read from stdin
		inputsize = _BASE16384_DECBUFSZ;
		fp = stdin;
	} else inputsize = get_file_size(input);
	if(inputsize <= 0) {
		if(!inputsize) errno = EINVAL;
		return base16384_err_get_file_size;
	}
	fpo = is_standard_io(output)?stdout:fopen(output, "wb");
	if(!fpo) {
		return base16384_err_fopen_output_file;
	}
	if(inputsize >= _BASE16384_DECBUFSZ) { // stdin or big file, use decbuf & fread
		inputsize = _BASE16384_DECBUFSZ;
		#if defined _WIN32 || defined __cosmopolitan
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_fopen_input_file, {});
		}
		int cnt = 0;
		int end = 0;
		rm_head(fp);
		if(errno) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_read_file, {});
		}
		#ifdef DEBUG
			fprintf(stderr, "inputsize: %lld\n", inputsize);
		#endif
		while((cnt = fread(decbuf, sizeof(char), inputsize, fp)) > 0) {
			if((end = is_next_end(fp))) {
				decbuf[cnt++] = '=';
				decbuf[cnt++] = end;
			}
			if(errno) goto_base16384_file_detailed_cleanup(decode, base16384_err_read_file, {});
			cnt = base16384_decode_unsafe(decbuf, cnt, encbuf);
			if(cnt && fwrite(encbuf, cnt, 1, fpo) <= 0) {
				goto_base16384_file_detailed_cleanup(decode, base16384_err_write_file, {});
			}
			if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) {
				if(calc_and_check_sum(&sum, cnt, encbuf)) {
					errno = EINVAL;
					goto_base16384_file_detailed_cleanup(decode, base16384_err_invalid_decoding_checksum, {});
				}
			}
		}
	#if !defined _WIN32 && !defined __cosmopolitan
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd < 0) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_open_input_file, {});
		}
		char *input_file = mmap(NULL, (size_t)inputsize+16, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_map_input_file, close(fd));
		}
		int off = skip_offset(input_file);
		int n = base16384_decode(input_file+off, inputsize-off, encbuf);
		if(n && fwrite(encbuf, n, 1, fpo) <= 0) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_write_file, {
				munmap(input_file, (size_t)inputsize+16);
				close(fd);
			});
		}
		munmap(input_file, (size_t)inputsize+16);
		close(fd);
	}
	#endif
base16384_decode_file_detailed_cleanup:
	if(fpo && !is_standard_io(output)) fclose(fpo);
	if(fp && !is_standard_io(input)) fclose(fp);
	if(errnobak) errno = errnobak;
	return retval;
}

base16384_err_t base16384_decode_fp_detailed(FILE* input, FILE* output, char* encbuf, char* decbuf, int flag) {
	if(!input) {
		errno = EINVAL;
		return base16384_err_fopen_input_file;
	}
	if(!output) {
		errno = EINVAL;
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = _BASE16384_DECBUFSZ;
	int cnt = 0;
	int end = 0;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	rm_head(input);
	if(errno) {
		return base16384_err_read_file;
	}
	while((cnt = fread(decbuf, sizeof(char), inputsize, input)) > 0) {
		if((end = is_next_end(input))) {
			decbuf[cnt++] = '=';
			decbuf[cnt++] = end;
		}
		cnt = base16384_decode_unsafe(decbuf, cnt, encbuf);
		if(cnt && fwrite(encbuf, cnt, 1, output) <= 0) {
			return base16384_err_write_file;
		}
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) {
			if (calc_and_check_sum(&sum, cnt, encbuf)) {
				errno = EINVAL;
				return base16384_err_invalid_decoding_checksum;
			}
		}
	}
	return base16384_err_ok;
}

static inline uint16_t is_next_end_fd(int fd) {
	uint8_t ch = 0;
	if(read(fd, &ch, 1) != 1) return (uint16_t)EOF;
	uint16_t ret = (uint16_t)ch & 0x00ff;
	if(ch == '=') {
		if(read(fd, &ch, 1) != 1) return (uint16_t)EOF;
		ret <<= 8;
		ret |= (uint16_t)ch & 0x00ff;
	}
	return ret;
}

base16384_err_t base16384_decode_fd_detailed(int input, int output, char* encbuf, char* decbuf, int flag) {
	if(input < 0) {
		errno = EINVAL;
		return base16384_err_fopen_input_file;
	}
	if(output < 0) {
		errno = EINVAL;
		return base16384_err_fopen_output_file;
	}
	off_t inputsize = _BASE16384_DECBUFSZ-1;
	int cnt = 0;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	decbuf[0] = 0;
	if(read(input, decbuf, 2) != 2) {
		return base16384_err_read_file;
	}
	if(decbuf[0] != (char)(0xfe)) {
		cnt = read(input, decbuf+2, inputsize-2)+2;
	} else {
		cnt = read(input, decbuf, inputsize);
	}
	if(cnt > 0) do {
		uint16_t next = is_next_end_fd(input);
		if(errno) {
			return base16384_err_read_file;
		}
		if((uint16_t)(~next)) {
			if(next&0xff00) decbuf[cnt++] = '=';
			decbuf[cnt++] = (char)(next&0x00ff);
		}
		#ifdef DEBUG
			fprintf(stderr, "decode chunk: %d, last2: %c %02x\n", cnt, decbuf[cnt-2], (uint8_t)decbuf[cnt-1]);
		#endif
		cnt = base16384_decode_unsafe(decbuf, cnt, encbuf);
		if(cnt && write(output, encbuf, cnt) != cnt) {
			return base16384_err_write_file;
		}
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) {
			if (calc_and_check_sum(&sum, cnt, encbuf)) {
				errno = EINVAL;
				return base16384_err_invalid_decoding_checksum;
			}
		}
	} while((cnt = read(input, decbuf, inputsize)) > 0);
	return base16384_err_ok;
}
