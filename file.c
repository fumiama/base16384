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

#define goto_base16384_file_detailed_cleanup(method, reason, dobeforereturn) { \
	errnobak = errno; \
	retval = reason; \
	dobeforereturn; \
	goto base16384_##method##_file_detailed_cleanup; \
}

base16384_err_t base16384_encode_file_detailed(const char* input, const char* output, char* encbuf, char* decbuf, int flag) {
	off_t inputsize;
	FILE *fp = NULL, *fpo;
	int errnobak = 0, is_stdin = is_standard_io(input);
	base16384_err_t retval = base16384_err_ok;
	if(!input || !output || strlen(input) <= 0 || strlen(output) <= 0) {
		errno = EINVAL;
		return base16384_err_invalid_file_name;
	}
	if(is_stdin) { // read from stdin
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
	if(flag&BASE16384_FLAG_DO_SUM_CHECK_FORCELY || inputsize >= _BASE16384_ENCBUFSZ) { // stdin or big file, use encbuf & fread
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
		size_t cnt;
		uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
		while((cnt = fread(encbuf, sizeof(char), inputsize, fp)) > 0) {
			int n;
			while(cnt%7) {
				n = fread(encbuf+cnt, sizeof(char), 1, fp);
				if(n > 0) cnt++;
				else break;
			}
			if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_and_embed_sum(sum, cnt, encbuf);
			n = base16384_encode_unsafe(encbuf, cnt, decbuf);
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
		char *input_file = mmap(NULL, (size_t)inputsize, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			goto_base16384_file_detailed_cleanup(encode, base16384_err_map_input_file, close(fd));
		}
		if(!(flag&BASE16384_FLAG_NOHEADER)) {
			fputc(0xFE, fpo);
			fputc(0xFF, fpo);
		}
		int n = base16384_encode_safe(input_file, (int)inputsize, decbuf);
		if(n && fwrite(decbuf, n, 1, fpo) <= 0) {
			goto_base16384_file_detailed_cleanup(encode, base16384_err_write_file, {
				munmap(input_file, (size_t)inputsize);
				close(fd);
			});
		}
		munmap(input_file, (size_t)inputsize);
		close(fd);
	}
	#endif
base16384_encode_file_detailed_cleanup:
	if(fpo && !is_standard_io(output)) fclose(fpo);
	if(fp && !is_stdin) fclose(fp);
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
	if(!(flag&BASE16384_FLAG_NOHEADER)) {
		fputc(0xFE, output);
		fputc(0xFF, output);
	}
	off_t inputsize = _BASE16384_ENCBUFSZ;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	size_t cnt;
	while((cnt = fread(encbuf, sizeof(char), inputsize, input)) > 0) {
		int n;
		while(cnt%7) {
			n = fread(encbuf+cnt, sizeof(char), 1, input);
			if(n > 0) cnt++;
			else break;
		}
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_and_embed_sum(sum, cnt, encbuf);
		n = base16384_encode_unsafe(encbuf, cnt, decbuf);
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
		int n;
		while(cnt%7) {
			n = read(input, encbuf+cnt, sizeof(char));
			if(n > 0) cnt++;
			else break;
		}
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_and_embed_sum(sum, cnt, encbuf);
		n = base16384_encode_unsafe(encbuf, cnt, decbuf);
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
	int errnobak = 0, is_stdin = is_standard_io(input);
	if(!input || !output || strlen(input) <= 0 || strlen(output) <= 0) {
		errno = EINVAL;
		return base16384_err_invalid_file_name;
	}
	if(is_stdin) { // read from stdin
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
	int loop_count = 0;
	if(inputsize >= _BASE16384_DECBUFSZ) { // stdin or big file, use decbuf & fread
		if(!is_stdin) loop_count = inputsize/_BASE16384_DECBUFSZ;
		inputsize = _BASE16384_DECBUFSZ;
		#if defined _WIN32 || defined __cosmopolitan
	}
		#endif
		if(!fp) fp = fopen(input, "rb");
		if(!fp) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_fopen_input_file, {});
		}
		rm_head(fp);
		if(errno) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_read_file, {});
		}
		int cnt, last_encbuf_cnt = 0, last_decbuf_cnt = 0, offset = 0;
		size_t total_decoded_len = 0;
		while((cnt = fread(decbuf, sizeof(char), inputsize, fp)) > 0) {
			int n;
			while(cnt%8) {
				n = fread(decbuf+cnt, sizeof(char), 1, fp);
				if(n > 0) cnt++;
				else break;
			}
			int end;
			if((end = is_next_end(fp))) {
				decbuf[cnt++] = '=';
				decbuf[cnt++] = end;
			}
			if(errno) goto_base16384_file_detailed_cleanup(decode, base16384_err_read_file, {});
			offset = decbuf[cnt-1];
			last_decbuf_cnt = cnt;
			cnt = base16384_decode_unsafe(decbuf, cnt, encbuf);
			if(cnt && fwrite(encbuf, cnt, 1, fpo) <= 0) {
				goto_base16384_file_detailed_cleanup(decode, base16384_err_write_file, {});
			}
			total_decoded_len += cnt;
			if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_sum(sum, cnt, encbuf);
			last_encbuf_cnt = cnt;
		}
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN
			&& (flag&BASE16384_FLAG_DO_SUM_CHECK_FORCELY || total_decoded_len >= _BASE16384_ENCBUFSZ)
			&& last_decbuf_cnt > 2
			&& decbuf[last_decbuf_cnt-2] == '='
			&& check_sum(sum, *(uint32_t*)(&encbuf[last_encbuf_cnt]), offset)) {
			errno = EINVAL;
			goto_base16384_file_detailed_cleanup(decode, base16384_err_invalid_decoding_checksum, {});
		}
	#if !defined _WIN32 && !defined __cosmopolitan
	} else { // small file, use mmap & fwrite
		int fd = open(input, O_RDONLY);
		if(fd < 0) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_open_input_file, {});
		}
		char *input_file = mmap(NULL, (size_t)inputsize, PROT_READ, MAP_PRIVATE, fd, 0);
		if(input_file == MAP_FAILED) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_map_input_file, close(fd));
		}
		int n = skip_offset(input_file);
		n = base16384_decode_safe(input_file+n, inputsize-n, encbuf);
		if(n && fwrite(encbuf, n, 1, fpo) <= 0) {
			goto_base16384_file_detailed_cleanup(decode, base16384_err_write_file, {
				munmap(input_file, (size_t)inputsize);
				close(fd);
			});
		}
		munmap(input_file, (size_t)inputsize);
		close(fd);
	}
	#endif
base16384_decode_file_detailed_cleanup:
	if(fpo && !is_standard_io(output)) fclose(fpo);
	if(fp && !is_stdin) fclose(fp);
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
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	rm_head(input);
	if(errno) {
		return base16384_err_read_file;
	}
	int cnt, last_encbuf_cnt = 0, last_decbuf_cnt = 0, offset = 0;
	size_t total_decoded_len = 0;
	while((cnt = fread(decbuf, sizeof(char), inputsize, input)) > 0) {
		int n;
		while(cnt%8) {
			n = fread(decbuf+cnt, sizeof(char), 1, input);
			if(n > 0) cnt++;
			else break;
		}
		int end;
		if((end = is_next_end(input))) {
			decbuf[cnt++] = '=';
			decbuf[cnt++] = end;
		}
		if(errno) return base16384_err_read_file;
		offset = decbuf[cnt-1];
		last_decbuf_cnt = cnt;
		cnt = base16384_decode_unsafe(decbuf, cnt, encbuf);
		if(cnt && fwrite(encbuf, cnt, 1, output) <= 0) {
			return base16384_err_write_file;
		}
		total_decoded_len += cnt;
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_sum(sum, cnt, encbuf);
		last_encbuf_cnt = cnt;
	}
	if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN
		&& (flag&BASE16384_FLAG_DO_SUM_CHECK_FORCELY || total_decoded_len >= _BASE16384_ENCBUFSZ)
		&& last_decbuf_cnt > 2
		&& decbuf[last_decbuf_cnt-2] == '='
		&& check_sum(sum, *(uint32_t*)(&encbuf[last_encbuf_cnt]), offset)) {
		errno = EINVAL;
		return base16384_err_invalid_decoding_checksum;
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

	off_t inputsize = _BASE16384_DECBUFSZ;
	uint32_t sum = BASE16384_SIMPLE_SUM_INIT_VALUE;
	uint8_t remains[8];

	decbuf[0] = 0;
	if(read(input, remains, 2) != 2) {
		return base16384_err_read_file;
	}

	int p = 0;
	if(remains[0] != (uint8_t)(0xfe)) p = 2;

	int n, last_encbuf_cnt = 0, last_decbuf_cnt = 0, offset = 0;
	size_t total_decoded_len = 0;
	while((n = read(input, decbuf+p, inputsize-p)) > 0) {
		if(p) {
			memcpy(decbuf, remains, p);
			n += p;
			p = 0;
		}
		int x;
		while(n%8) {
			x = read(input, decbuf+n, sizeof(char));
			if(x > 0) n++;
			else break;
		}
		uint16_t next = is_next_end_fd(input);
		if(errno) {
			return base16384_err_read_file;
		}
		if((uint16_t)(~next)) {
			if(next&0xff00) {
				decbuf[n++] = '=';
				decbuf[n++] = (char)(next&0x00ff);
			} else remains[p++] = (char)(next&0x00ff);
		}
		offset = decbuf[n-1];
		last_decbuf_cnt = n;
		n = base16384_decode_unsafe(decbuf, n, encbuf);
		if(n && write(output, encbuf, n) != n) {
			return base16384_err_write_file;
		}
		total_decoded_len += n;
		if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN) sum = calc_sum(sum, n, encbuf);
		last_encbuf_cnt = n;
	}
	if(flag&BASE16384_FLAG_SUM_CHECK_ON_REMAIN
		&& (flag&BASE16384_FLAG_DO_SUM_CHECK_FORCELY || total_decoded_len >= _BASE16384_ENCBUFSZ)
		&& last_decbuf_cnt > 2
		&& decbuf[last_decbuf_cnt-2] == '='
		&& check_sum(sum, *(uint32_t*)(&encbuf[last_encbuf_cnt]), offset)) {
		errno = EINVAL;
		return base16384_err_invalid_decoding_checksum;
	}
	return base16384_err_ok;
}
