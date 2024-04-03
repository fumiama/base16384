#ifndef _BASE16384_H_
#define _BASE16384_H_

/* base16384.h
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
#include <stdint.h>
#include <stdio.h>
#endif

#define define_base16384_err_t(n) base16384_err_##n

    enum base16384_err_t {
        define_base16384_err_t(ok),
        define_base16384_err_t(get_file_size),
        define_base16384_err_t(fopen_output_file),
        define_base16384_err_t(fopen_input_file),
        define_base16384_err_t(write_file),
        define_base16384_err_t(open_input_file),
        define_base16384_err_t(map_input_file),
        define_base16384_err_t(read_file),
        define_base16384_err_t(invalid_file_name),
        define_base16384_err_t(invalid_commandline_parameter),
        define_base16384_err_t(invalid_decoding_checksum),
    };

    /**
     * @brief return value of base16384_en/decode_file
    */
    typedef enum base16384_err_t base16384_err_t;

#undef define_base16384_err_t

#define _BASE16384_ENCBUFSZ (BUFSIZ*1024/7*7)
#define _BASE16384_DECBUFSZ (BUFSIZ*1024/8*8)

#define BASE16384_ENCBUFSZ (_BASE16384_ENCBUFSZ+16)
#define BASE16384_DECBUFSZ (_BASE16384_DECBUFSZ+16)

// disable 0xFEFF file header in encode
#define BASE16384_FLAG_NOHEADER				(1<<0)
// enable sum check when using stdin or inputsize > _BASE16384_ENCBUFSZ
#define BASE16384_FLAG_SUM_CHECK_ON_REMAIN	(1<<1)
// initial sum value used in BASE16384_FLAG_SUM_CHECK_ON_REMAIN
#define BASE16384_SIMPLE_SUM_INIT_VALUE		(0x8e29c213)

/**
 * @brief calculate the exact encoded size
 * @param dlen the data length to encode
 * @return the size
*/
static inline int _base16384_encode_len(int dlen) {
	int outlen = dlen / 7 * 8;
	int offset = dlen % 7;
	switch(offset) {	// 算上偏移标志字符占用的 2 字节
		case 0: break;
		case 1: outlen += 4; break;
		case 2:
		case 3: outlen += 6; break;
		case 4:
		case 5: outlen += 8; break;
		case 6: outlen += 10; break;
		default: break;
	}
	return outlen;
}

/**
 * @brief calculate minimum encoding buffer size (16 bits larger than the real encoded size)
 * @param dlen the data length to encode
 * @return the minimum encoding buffer size
*/
static inline int base16384_encode_len(int dlen) {
	return _base16384_encode_len(dlen) + 16;	// 冗余的 16 字节用于可能的结尾的 unsafe 覆盖
}

/**
 * @brief calculate the exact decoded size
 * @param dlen the data length to decode
 * @param offset the last char `xx` of the underfilled coding (0x3Dxx) or 0 for the full coding
 * @return the size
*/
static inline int _base16384_decode_len(int dlen, int offset) {
	int outlen = dlen;
	switch(offset) {	// 算上偏移标志字符占用的 2 字节
		case 0: break;
		case 1: outlen -= 4; break;
		case 2:
		case 3: outlen -= 6; break;
		case 4:
		case 5: outlen -= 8; break;
		case 6: outlen -= 10; break;
		default: break;
	}
	return outlen / 8 * 7 + offset;
}

/**
 * @brief calculate minimum decoding buffer size (16 bits larger than the real decoded size)
 * @param dlen the data length to decode
 * @param offset the last char `xx` of the underfilled coding (0x3Dxx) or 0 for the full coding
 * @return the minimum decoding buffer size
*/
static inline int base16384_decode_len(int dlen, int offset) {
	return _base16384_decode_len(dlen, offset) + 16; // 多出 16 字节用于 unsafe 循环覆盖
}

/**
 * @brief encode data and write result into buf
 * @param data data to encode
 * @param dlen the data length
 * @param buf the output buffer, whose size must greater than `base16384_encode_len`
 * @return the total length written
*/
int base16384_encode(const char* data, int dlen, char* buf);

/**
 * @brief encode data and write result into buf without considering border condition
 * @param data data to encode
 * @param dlen the data length
 * @param buf the output buffer, whose size must greater than `base16384_encode_len`
 * @return the total length written
*/
int base16384_encode_unsafe(const char* data, int dlen, char* buf);

/**
 * @brief decode data and write result into buf
 * @param data data to decode
 * @param dlen the data length
 * @param buf the output buffer, whose size must greater than `base16384_decode_len`
 * @return the total length written
*/
int base16384_decode(const char* data, int dlen, char* buf);

/**
 * @brief decode data and write result into buf without considering border condition
 * @param data data to decode
 * @param dlen the data length
 * @param buf the output buffer, whose size must greater than `base16384_decode_len`
 * @return the total length written
*/
int base16384_decode_unsafe(const char* data, int dlen, char* buf);

// base16384_encode_file_detailed encodes input file to output file.
//    use `-` to specify stdin/stdout
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
base16384_err_t base16384_encode_file_detailed(const char* input, const char* output, char* encbuf, char* decbuf, int flag);

// base16384_encode_fp_detailed encodes input file to output file.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
base16384_err_t base16384_encode_fp_detailed(FILE* input, FILE* output, char* encbuf, char* decbuf, int flag);

// base16384_encode_fd_detailed encodes input fd to output fd.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
base16384_err_t base16384_encode_fd_detailed(int input, int output, char* encbuf, char* decbuf, int flag);

// base16384_decode_file_detailed decodes input file to output file.
//    use `-` to specify stdin/stdout
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
base16384_err_t base16384_decode_file_detailed(const char* input, const char* output, char* encbuf, char* decbuf, int flag);

// base16384_decode_fp_detailed decodes input file to output file.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
base16384_err_t base16384_decode_fp_detailed(FILE* input, FILE* output, char* encbuf, char* decbuf, int flag);

// base16384_decode_fd_detailed decodes input fd to output fd.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
base16384_err_t base16384_decode_fd_detailed(int input, int output, char* encbuf, char* decbuf, int flag);

// base16384_encode_file encodes input file to output file.
//    use `-` to specify stdin/stdout
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
static inline base16384_err_t base16384_encode_file(const char* input, const char* output, char* encbuf, char* decbuf) {
	return base16384_encode_file_detailed(input, output, encbuf, decbuf, 0);
}

// base16384_encode_fp encodes input file to output file.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
static inline base16384_err_t base16384_encode_fp(FILE* input, FILE* output, char* encbuf, char* decbuf) {
	return base16384_encode_fp_detailed(input, output, encbuf, decbuf, 0);
}

// base16384_encode_fd encodes input fd to output fd.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
static inline base16384_err_t base16384_encode_fd(int input, int output, char* encbuf, char* decbuf) {
	return base16384_encode_fd_detailed(input, output, encbuf, decbuf, 0);
}

// base16384_decode_file decodes input file to output file.
//    use `-` to specify stdin/stdout
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
static inline base16384_err_t base16384_decode_file(const char* input, const char* output, char* encbuf, char* decbuf) {
	return base16384_decode_file_detailed(input, output, encbuf, decbuf, 0);
}

// base16384_decode_fp decodes input file to output file.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
static inline base16384_err_t base16384_decode_fp(FILE* input, FILE* output, char* encbuf, char* decbuf) {
	return base16384_decode_fp_detailed(input, output, encbuf, decbuf, 0);
}

// base16384_decode_fd decodes input fd to output fd.
//    encbuf & decbuf must be no less than BASE16384_ENCBUFSZ & BASE16384_DECBUFSZ
static inline base16384_err_t base16384_decode_fd(int input, int output, char* encbuf, char* decbuf) {
	return base16384_decode_fd_detailed(input, output, encbuf, decbuf, 0);
}

#endif
