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

enum base16384_err_t {
	base16384_err_ok,
	base16384_err_get_file_size,
	base16384_err_fopen_output_file,
	base16384_err_fopen_input_file,
	base16384_err_write_file,
	base16384_err_open_input_file,
	base16384_err_map_input_file,
	base16384_err_read_file,
	base16384_err_invalid_file_name,
	base16384_err_invalid_commandline_parameter,
	base16384_err_invalid_decoding_checksum,
};
/**
 * @brief return value of base16384_en/decode_file
*/
typedef enum base16384_err_t base16384_err_t;

#ifndef BASE16384_BUFSZ_FACTOR
	#define BASE16384_BUFSZ_FACTOR (8)
#endif

#define _BASE16384_ENCBUFSZ ((BUFSIZ*BASE16384_BUFSZ_FACTOR)/7*7)
#define _BASE16384_DECBUFSZ ((BUFSIZ*BASE16384_BUFSZ_FACTOR)/8*8)

#define BASE16384_ENCBUFSZ (_BASE16384_ENCBUFSZ+16)
#define BASE16384_DECBUFSZ (_BASE16384_DECBUFSZ+16)

// disable 0xFEFF file header in encode
#define BASE16384_FLAG_NOHEADER				(1<<0)
// enable sum check when using stdin or stdout or inputsize > _BASE16384_ENCBUFSZ
#define BASE16384_FLAG_SUM_CHECK_ON_REMAIN	(1<<1)
// forcely do sumcheck without checking data length
#define BASE16384_FLAG_DO_SUM_CHECK_FORCELY	(1<<2)

/**
 * @brief custom reader function interface
 * @param client_data the data pointer defined by the client
 * @param buffer to where put data
 * @param count read bytes count
 * @return the size read
*/
typedef ssize_t (*base16384_reader_t)(const void *client_data, void *buffer, size_t count);

/**
 * @brief custom writer function interface
 * @param client_data the data pointer defined by the client
 * @param buffer from where read data
 * @param count write bytes count
 * @return the size written
*/
typedef ssize_t (*base16384_writer_t)(const void *client_data, const void *buffer, size_t count);

union base16384_io_function_t {
	base16384_reader_t reader;
	base16384_writer_t writer;
};
typedef union base16384_io_function_t base16384_io_function_t;

struct base16384_stream_t {
	const base16384_io_function_t f;
	const void *client_data;
};
/**
 * @brief for stream encode/decode
*/
typedef struct base16384_stream_t base16384_stream_t;

/**
 * @brief calculate the exact encoded size
 * @param dlen the data length to encode
 * @return the size
*/
static inline int _base16384_encode_len(int dlen) {
	int outlen = dlen / 7 * 8;
	int offset = dlen % 7;
	switch(offset) {	// also count 0x3dxx
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
	switch(offset) {	// also count 0x3dxx
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
 * @brief safely encode data and write result into buf
 * @param data data to encode, no data overread
 * @param dlen the data length
 * @param buf the output buffer, whose size can be exactly `_base16384_encode_len`
 * @return the total length written
*/
int base16384_encode_safe(const char* data, int dlen, char* buf);

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
 * @brief safely decode data and write result into buf
 * @param data data to decode, no data overread
 * @param dlen the data length
 * @param buf the output buffer, whose size can be exactly `_base16384_decode_len`
 * @return the total length written
*/
int base16384_decode_safe(const char* data, int dlen, char* buf);

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

#define base16384_typed_params(type) type input, type output, char* encbuf, char* decbuf
#define base16384_typed_flag_params(type) base16384_typed_params(type), int flag

/**
 * @brief encode input file to output file
 * @param input filename or `-` to specify stdin
 * @param output filename or `-` to specify stdout
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_encode_file_detailed(base16384_typed_flag_params(const char*));

/**
 * @brief encode input `FILE*` to output `FILE*`
 * @param input `FILE*` pointer
 * @param output `FILE*` pointer
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_encode_fp_detailed(base16384_typed_flag_params(FILE*));

/**
 * @brief encode input stream to output stream
 * @param input file descripter
 * @param output file descripter
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_encode_fd_detailed(base16384_typed_flag_params(int));

/**
 * @brief encode custom input reader to custom output writer
 * @param input custom input reader
 * @param output custom output writer
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_encode_stream_detailed(base16384_typed_flag_params(base16384_stream_t*));

/**
 * @brief decode input file to output file
 * @param input filename or `-` to specify stdin
 * @param output filename or `-` to specify stdout
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_decode_file_detailed(base16384_typed_flag_params(const char*));

/**
 * @brief decode input `FILE*` to output `FILE*`
 * @param input `FILE*` pointer
 * @param output `FILE*` pointer
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_decode_fp_detailed(base16384_typed_flag_params(FILE*));

/**
 * @brief decode input stream to output stream
 * @param input file descripter
 * @param output file descripter
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_decode_fd_detailed(base16384_typed_flag_params(int));

/**
 * @brief decode custom input reader to custom output writer
 * @param input custom input reader
 * @param output custom output writer
 * @param encbuf must be no less than BASE16384_ENCBUFSZ
 * @param decbuf must be no less than BASE16384_DECBUFSZ
 * @param flag BASE16384_FLAG_xxx value, add multiple flags by `|`
 * @return the error code
*/
base16384_err_t base16384_decode_stream_detailed(base16384_typed_flag_params(base16384_stream_t*));

#define BASE16384_WRAP_DECL(method, name, type) \
	base16384_err_t base16384_##method##_##name(base16384_typed_params(type));

	BASE16384_WRAP_DECL(encode, file, const char*);
	BASE16384_WRAP_DECL(encode, fp, FILE*);
	BASE16384_WRAP_DECL(encode, fd, int);
	BASE16384_WRAP_DECL(encode, stream, base16384_stream_t*);

	BASE16384_WRAP_DECL(decode, file, const char*);
	BASE16384_WRAP_DECL(decode, fp, FILE*);
	BASE16384_WRAP_DECL(decode, fd, int);
	BASE16384_WRAP_DECL(decode, stream, base16384_stream_t*);

#undef BASE16384_WRAP_DECL

#undef base16384_typed_flag_params
#undef base16384_typed_params

/**
 * @brief call perror on error
 * @param err the error
 * @return the input parameter `err`
*/
static inline base16384_err_t base16384_perror(base16384_err_t err) {
	#define base16384_perror_case(n) case base16384_err_##n: perror("base16384_err_"#n)
		if(err) switch(err) {
			base16384_perror_case(get_file_size); break;
			base16384_perror_case(fopen_output_file); break;
			base16384_perror_case(fopen_input_file); break;
			base16384_perror_case(write_file); break;
			base16384_perror_case(open_input_file); break;
			base16384_perror_case(map_input_file); break;
			base16384_perror_case(read_file); break;
			base16384_perror_case(invalid_file_name); break;
			base16384_perror_case(invalid_commandline_parameter); break;
			base16384_perror_case(invalid_decoding_checksum); break;
			default: perror("base16384"); break;
		}
	#undef base16384_perror_case
	return err;
}

#endif
