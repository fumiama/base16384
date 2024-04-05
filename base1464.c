/* base1464.c
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
#include <string.h>
#endif

#include "binary.h"

typedef union {
	uint8_t buf[8];
	uint64_t val;
} base16384_union_remainder;

int base16384_encode_safe(const char* data, int dlen, char* buf) {
	int outlen = dlen / 7 * 8;
	int offset = dlen % 7;
	switch(offset) {	// 算上偏移标志字符占用的2字节
		case 0: break;
		case 1: outlen += 4; break;
		case 2:
		case 3: outlen += 6; break;
		case 4:
		case 5: outlen += 8; break;
		case 6: outlen += 10; break;
		default: break;
	}
	uint64_t* vals = (uint64_t*)buf;
	uint64_t n = 0;
	int64_t i = 0;
	for(; i < dlen - 7; i += 7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(*(uint64_t*)(data+i))>>2;
		sum |= shift & 0x3fff000000000000;
		shift >>= 2;
		sum |= shift & 0x00003fff00000000;
		shift >>= 2;
		sum |= shift & 0x000000003fff0000;
		shift >>= 2;
		sum |= shift & 0x0000000000003fff;
		sum += 0x4e004e004e004e00;
		vals[n++] = be64toh(sum);
	}
	base16384_union_remainder valbuf;
	if(dlen - i == 7) {
		memcpy(valbuf.buf, data+i, 7);
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(valbuf.val)>>2;
		sum |= shift & 0x3fff000000000000;
		shift >>= 2;
		sum |= shift & 0x00003fff00000000;
		shift >>= 2;
		sum |= shift & 0x000000003fff0000;
		shift >>= 2;
		sum |= shift & 0x0000000000003fff;
		sum += 0x4e004e004e004e00;
		vals[n++] = be64toh(sum);
		return outlen;
	}
	int o = offset;
	if(o--) {
		register uint64_t sum = 0x000000000000003f & (data[i] >> 2);
		sum |= ((uint64_t)data[i] << 14) & 0x000000000000c000;
		if(o--) {
			sum |= ((uint64_t)data[i + 1] << 6) & 0x0000000000003f00;
			sum |= ((uint64_t)data[i + 1] << 20) & 0x0000000000300000;
			if(o--) {
				sum |= ((uint64_t)data[i + 2] << 12) & 0x00000000000f0000;
				sum |= ((uint64_t)data[i + 2] << 28) & 0x00000000f0000000;
				if(o--) {
					sum |= ((uint64_t)data[i + 3] << 20) & 0x000000000f000000;
					sum |= ((uint64_t)data[i + 3] << 34) & 0x0000003c00000000;
					if(o--) {
						sum |= ((uint64_t)data[i + 4] << 26) & 0x0000000300000000;
						sum |= ((uint64_t)data[i + 4] << 42) & 0x0000fc0000000000;
						if(o--) {
							sum |= ((uint64_t)data[i + 5] << 34) & 0x0000030000000000;
							sum |= ((uint64_t)data[i + 5] << 48) & 0x003f000000000000;
						}
					}
				}
			}
		}
		sum += 0x004e004e004e004e;
		#ifdef WORDS_BIGENDIAN
			valbuf.val = __builtin_bswap64(sum);
		#else
			valbuf.val = sum;
		#endif
		memcpy(&vals[n], valbuf.buf, outlen-2-(int)n*(int)sizeof(uint64_t));
		buf[outlen - 2] = '=';
		buf[outlen - 1] = offset;
	}
	return outlen;
}

int base16384_encode(const char* data, int dlen, char* buf) {
	int outlen = dlen / 7 * 8;
	int offset = dlen % 7;
	switch(offset) {	// 算上偏移标志字符占用的2字节
		case 0: break;
		case 1: outlen += 4; break;
		case 2:
		case 3: outlen += 6; break;
		case 4:
		case 5: outlen += 8; break;
		case 6: outlen += 10; break;
		default: break;
	}
	uint64_t* vals = (uint64_t*)buf;
	uint64_t n = 0;
	int64_t i = 0;
	for(; i <= dlen - 7; i += 7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(*(uint64_t*)(data+i))>>2; // 这里有读取越界
		sum |= shift & 0x3fff000000000000;
		shift >>= 2;
		sum |= shift & 0x00003fff00000000;
		shift >>= 2;
		sum |= shift & 0x000000003fff0000;
		shift >>= 2;
		sum |= shift & 0x0000000000003fff;
		sum += 0x4e004e004e004e00;
		vals[n++] = be64toh(sum);
	}
	int o = offset;
	if(o--) {
		register uint64_t sum = 0x000000000000003f & (data[i] >> 2);
		sum |= ((uint64_t)data[i] << 14) & 0x000000000000c000;
		if(o--) {
			sum |= ((uint64_t)data[i + 1] << 6) & 0x0000000000003f00;
			sum |= ((uint64_t)data[i + 1] << 20) & 0x0000000000300000;
			if(o--) {
				sum |= ((uint64_t)data[i + 2] << 12) & 0x00000000000f0000;
				sum |= ((uint64_t)data[i + 2] << 28) & 0x00000000f0000000;
				if(o--) {
					sum |= ((uint64_t)data[i + 3] << 20) & 0x000000000f000000;
					sum |= ((uint64_t)data[i + 3] << 34) & 0x0000003c00000000;
					if(o--) {
						sum |= ((uint64_t)data[i + 4] << 26) & 0x0000000300000000;
						sum |= ((uint64_t)data[i + 4] << 42) & 0x0000fc0000000000;
						if(o--) {
							sum |= ((uint64_t)data[i + 5] << 34) & 0x0000030000000000;
							sum |= ((uint64_t)data[i + 5] << 48) & 0x003f000000000000;
						}
					}
				}
			}
		}
		sum += 0x004e004e004e004e;
		#ifdef WORDS_BIGENDIAN
			vals[n] = __builtin_bswap64(sum);
		#else
			vals[n] = sum;
		#endif
		buf[outlen - 2] = '=';
		buf[outlen - 1] = offset;
	}
	return outlen;
}

int base16384_encode_unsafe(const char* data, int dlen, char* buf) {
	int outlen = dlen / 7 * 8;
	int offset = dlen % 7;
	switch(offset) {	// 算上偏移标志字符占用的2字节
		case 0: break;
		case 1: outlen += 4; break;
		case 2:
		case 3: outlen += 6; break;
		case 4:
		case 5: outlen += 8; break;
		case 6: outlen += 10; break;
		default: break;
	}
	uint64_t* vals = (uint64_t*)buf;
	uint64_t n = 0;
	int64_t i = 0;
	for(; i < dlen; i += 7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(*(uint64_t*)(data+i))>>2; // 这里有读取越界
		sum |= shift & 0x3fff000000000000;
		shift >>= 2;
		sum |= shift & 0x00003fff00000000;
		shift >>= 2;
		sum |= shift & 0x000000003fff0000;
		shift >>= 2;
		sum |= shift & 0x0000000000003fff;
		sum += 0x4e004e004e004e00;
		vals[n++] = be64toh(sum);
	}
	if(offset) {
		buf[outlen - 2] = '=';
		buf[outlen - 1] = offset;
	}
	return outlen;
}

int base16384_decode_safe(const char* data, int dlen, char* buf) {
	int outlen = dlen;
	int offset = 0;
	if(data[dlen-2] == '=') {
		offset = data[dlen-1];
		switch(offset) {	// 算上偏移标志字符占用的2字节
			case 0: break;
			case 1: outlen -= 4; break;
			case 2:
			case 3: outlen -= 6; break;
			case 4:
			case 5: outlen -= 8; break;
			case 6: outlen -= 10; break;
			default: break;
		}
	}
	outlen = outlen / 8 * 7 + offset;
	const uint64_t* vals = (const uint64_t*)data;
	uint64_t n = 0;
	int64_t i = 0;
	for(; i < outlen - 7; n++, i+=7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(vals[n]) - 0x4e004e004e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc000000000000;
		shift <<= 2;
		sum |= shift & 0x0003fff000000000;
		shift <<= 2;
		sum |= shift & 0x0000000fffc00000;
		shift <<= 2;
		sum |= shift & 0x00000000003fff00;
		*(uint64_t*)(buf+i) = be64toh(sum);
	}
	base16384_union_remainder valbuf;
	if(outlen - i == 7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(vals[n]) - 0x4e004e004e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc000000000000;
		shift <<= 2;
		sum |= shift & 0x0003fff000000000;
		shift <<= 2;
		sum |= shift & 0x0000000fffc00000;
		shift <<= 2;
		sum |= shift & 0x00000000003fff00;
		valbuf.val = be64toh(sum);
		memcpy(buf+i, valbuf.buf, 7);
	} else if((*(uint8_t*)(&vals[n]) != '=') && offset--) {
		memcpy(valbuf.buf, &vals[n], dlen-2-(int)n*(int)sizeof(uint64_t));
		#ifdef WORDS_BIGENDIAN
			register uint64_t sum = __builtin_bswap64(valbuf.val) - 0x000000000000004e;
		#else
			register uint64_t sum = valbuf.val - 0x000000000000004e;
		#endif
		buf[i++] = ((sum & 0x000000000000003f) << 2) | ((sum & 0x000000000000c000) >> 14);
		if(offset--) {
			sum -= 0x00000000004e0000;
			buf[i++] = ((sum & 0x0000000000003f00) >> 6) | ((sum & 0x0000000000300000) >> 20);
			if(offset--) {
				buf[i++] = ((sum & 0x00000000000f0000) >> 12) | ((sum & 0x00000000f0000000) >> 28);
				if(offset--) {
					sum -= 0x0000004e00000000;
					buf[i++] = ((sum & 0x000000000f000000) >> 20) | ((sum & 0x0000003c00000000) >> 34);
					if(offset--) {
						buf[i++] = ((sum & 0x0000000300000000) >> 26) | ((sum & 0x0000fc0000000000) >> 42);
						if(offset--) {
							sum -= 0x004e000000000000;
							buf[i] = ((sum & 0x0000030000000000) >> 34) | ((sum & 0x003f000000000000) >> 48);
						}
					}
				}
			}
		}
	}
	return outlen;
}

int base16384_decode(const char* data, int dlen, char* buf) {
	int outlen = dlen;
	int offset = 0;
	if(data[dlen-2] == '=') {
		offset = data[dlen-1];
		switch(offset) {	// 算上偏移标志字符占用的2字节
			case 0: break;
			case 1: outlen -= 4; break;
			case 2:
			case 3: outlen -= 6; break;
			case 4:
			case 5: outlen -= 8; break;
			case 6: outlen -= 10; break;
			default: break;
		}
	}
	outlen = outlen / 8 * 7 + offset;
	const uint64_t* vals = (const uint64_t*)data;
	uint64_t n = 0;
	int64_t i = 0;
	for(; i <= outlen - 7; n++, i+=7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(vals[n]) - 0x4e004e004e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc000000000000;
		shift <<= 2;
		sum |= shift & 0x0003fff000000000;
		shift <<= 2;
		sum |= shift & 0x0000000fffc00000;
		shift <<= 2;
		sum |= shift & 0x00000000003fff00;
		*(uint64_t*)(buf+i) = be64toh(sum);
	}
	if(*(uint8_t*)(&vals[n]) == '=') return outlen;
	if(offset--) {
		// 这里有读取越界
		#ifdef WORDS_BIGENDIAN
			register uint64_t sum = __builtin_bswap64(vals[n]) - 0x000000000000004e;
		#else
			register uint64_t sum = vals[n] - 0x000000000000004e;
		#endif
		buf[i++] = ((sum & 0x000000000000003f) << 2) | ((sum & 0x000000000000c000) >> 14);
		if(offset--) {
			sum -= 0x00000000004e0000;
			buf[i++] = ((sum & 0x0000000000003f00) >> 6) | ((sum & 0x0000000000300000) >> 20);
			if(offset--) {
				buf[i++] = ((sum & 0x00000000000f0000) >> 12) | ((sum & 0x00000000f0000000) >> 28);
				if(offset--) {
					sum -= 0x0000004e00000000;
					buf[i++] = ((sum & 0x000000000f000000) >> 20) | ((sum & 0x0000003c00000000) >> 34);
					if(offset--) {
						buf[i++] = ((sum & 0x0000000300000000) >> 26) | ((sum & 0x0000fc0000000000) >> 42);
						if(offset--) {
							sum -= 0x004e000000000000;
							buf[i] = ((sum & 0x0000030000000000) >> 34) | ((sum & 0x003f000000000000) >> 48);
						}
					}
				}
			}
		}
	}
	return outlen;
}

int base16384_decode_unsafe(const char* data, int dlen, char* buf) {
	int outlen = dlen;
	int offset = 0;
	if(data[dlen-2] == '=') {
		offset = data[dlen-1];
		switch(offset) {	// 算上偏移标志字符占用的2字节
			case 0: break;
			case 1: outlen -= 4; break;
			case 2:
			case 3: outlen -= 6; break;
			case 4:
			case 5: outlen -= 8; break;
			case 6: outlen -= 10; break;
			default: break;
		}
	}
	outlen = outlen / 8 * 7 + offset;
	const uint64_t* vals = (const uint64_t*)data;
	uint64_t n = 0;
	int64_t i = 0;
	for(; i < outlen-7; n++, i+=7) {
		register uint64_t sum = 0;
		register uint64_t shift = htobe64(vals[n]) - 0x4e004e004e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc000000000000;
		shift <<= 2;
		sum |= shift & 0x0003fff000000000;
		shift <<= 2;
		sum |= shift & 0x0000000fffc00000;
		shift <<= 2;
		sum |= shift & 0x00000000003fff00;
		*(uint64_t*)(buf+i) = be64toh(sum);
	}
	register uint64_t sum = 0;
	register uint64_t shift = htobe64(vals[n]);
	if(((shift>>56)&0xff) == 0x3d) return outlen;
	if(((shift>>56)&0xff) < 0x4e) shift |= 0xff00000000000000;
	if(((shift>>40)&0xff) < 0x4e) shift |= 0x0000ff0000000000;
	if(((shift>>24)&0xff) < 0x4e) shift |= 0x00000000ff000000;
	if(((shift>> 8)&0xff) < 0x4e) shift |= 0x000000000000ff00;
	shift -= 0x4e004e004e004e00;
	shift <<= 2;
	sum |= shift & 0xfffc000000000000;
	shift <<= 2;
	sum |= shift & 0x0003fff000000000;
	shift <<= 2;
	sum |= shift & 0x0000000fffc00000;
	shift <<= 2;
	sum |= shift & 0x00000000003fff00;
	*(uint64_t*)(buf+i) = be64toh(sum);
	return outlen;
}
