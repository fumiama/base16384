/* base1432.c
 * This file is part of the base16384 distribution (https://github.com/fumiama/base16384).
 * Copyright (c) 2022-2025 Fumiama Minamoto.
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
	uint8_t buf[4];
	uint32_t val;
} base16384_union_remainder;

int base16384_encode_safe(const char* data, int dlen, char* buf) {
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
	uint32_t* vals = (uint32_t*)buf;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i < dlen - 7; i += 7) {
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(*(uint32_t*)(data+i));
		sum |= (shift>>2) & 0x3fff0000;
		sum |= (shift>>4) & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
		shift <<= 26;
		shift &= 0x3c000000;
		sum = 0;
		shift |= (htobe32(*(uint32_t*)(data+i+4))>>6)&0x03fffffc;
		sum |= shift & 0x3fff0000;
		shift >>= 2;
		sum |= shift & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
	}
	base16384_union_remainder valbuf;
	if(dlen - i == 7) {
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(*(uint32_t*)(data+i));
		sum |= (shift>>2) & 0x3fff0000;
		sum |= (shift>>4) & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
		shift <<= 26;
		shift &= 0x3c000000;
		sum = 0;
		memcpy(valbuf.buf, data+i+4, 3);
		shift |= (htobe32(valbuf.val)>>6)&0x03fffffc;
		sum |= shift & 0x3fff0000;
		shift >>= 2;
		sum |= shift & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
		return outlen;
	}
	uint8_t o = offset;
	if(o--) {
		register uint32_t sum = 0x0000003f & (data[i] >> 2);
		sum |= ((uint32_t)data[i] << 14) & 0x0000c000;
		if(o--) {
			sum |= ((uint32_t)data[i + 1] << 6) & 0x00003f00;
			sum |= ((uint32_t)data[i + 1] << 20) & 0x00300000;
			if(o--) {
				sum |= ((uint32_t)data[i + 2] << 12) & 0x000f0000;
				sum |= ((uint32_t)data[i + 2] << 28) & 0xf0000000;
				if(o--) {
					sum |= ((uint32_t)data[i + 3] << 20) & 0x0f000000;
					sum += 0x004e004e;
					// safe, because it will never go over 0x3dxx
					#ifdef WORDS_BIGENDIAN
						vals[n++] = __builtin_bswap32(sum);
					#else
						vals[n++] = sum;
					#endif
					sum = (((uint32_t)data[i + 3] << 2)) & 0x0000003c;
					if(o--) {
						sum |= (((uint32_t)data[i + 4] >> 6)) & 0x00000003;
						sum |= ((uint32_t)data[i + 4] << 10) & 0x0000fc00;
						if(o--) {
							sum |= ((uint32_t)data[i + 5] << 2) & 0x00000300;
							sum |= ((uint32_t)data[i + 5] << 16) & 0x003f0000;
						}
					}
				}
			}
		}
		sum += 0x004e004e;
		// safe, because it will never go over 0x3dxx
		#ifdef WORDS_BIGENDIAN
			vals[n] = __builtin_bswap32(sum);
		#else
			vals[n] = sum;
		#endif
		buf[outlen - 2] = '=';
		buf[outlen - 1] = offset;
	}
	return outlen;
}

int base16384_encode(const char* data, int dlen, char* buf) {
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
	uint32_t* vals = (uint32_t*)buf;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i <= dlen - 7; i += 7) {
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(*(uint32_t*)(data+i));
		sum |= (shift>>2) & 0x3fff0000;
		sum |= (shift>>4) & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
		shift <<= 26;
		shift &= 0x3c000000;
		sum = 0;
		shift |= (htobe32(*(uint32_t*)(data+i+4))>>6)&0x03fffffc;
		sum |= shift & 0x3fff0000;
		shift >>= 2;
		sum |= shift & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
	}
	uint8_t o = offset;
	if(o--) {
		register uint32_t sum = 0x0000003f & (data[i] >> 2);
		sum |= ((uint32_t)data[i] << 14) & 0x0000c000;
		if(o--) {
			sum |= ((uint32_t)data[i + 1] << 6) & 0x00003f00;
			sum |= ((uint32_t)data[i + 1] << 20) & 0x00300000;
			if(o--) {
				sum |= ((uint32_t)data[i + 2] << 12) & 0x000f0000;
				sum |= ((uint32_t)data[i + 2] << 28) & 0xf0000000;
				if(o--) {
					sum |= ((uint32_t)data[i + 3] << 20) & 0x0f000000;
					sum += 0x004e004e;
					#ifdef WORDS_BIGENDIAN
						vals[n++] = __builtin_bswap32(sum);
					#else
						vals[n++] = sum;
					#endif
					sum = (((uint32_t)data[i + 3] << 2)) & 0x0000003c;
					if(o--) {
						sum |= (((uint32_t)data[i + 4] >> 6)) & 0x00000003;
						sum |= ((uint32_t)data[i + 4] << 10) & 0x0000fc00;
						if(o--) {
							sum |= ((uint32_t)data[i + 5] << 2) & 0x00000300;
							sum |= ((uint32_t)data[i + 5] << 16) & 0x003f0000;
						}
					}
				}
			}
		}
		sum += 0x004e004e;
		#ifdef WORDS_BIGENDIAN
			vals[n] = __builtin_bswap32(sum);
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
	uint32_t* vals = (uint32_t*)buf;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i < dlen; i += 7) {
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(*(uint32_t*)(data+i));
		sum |= (shift>>2) & 0x3fff0000;
		sum |= (shift>>4) & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
		shift <<= 26;
		shift &= 0x3c000000;
		sum = 0;
		shift |= (htobe32(*(uint32_t*)(data+i+4))>>6)&0x03fffffc;
		sum |= shift & 0x3fff0000;
		shift >>= 2;
		sum |= shift & 0x00003fff;
		sum += 0x4e004e00;
		vals[n++] = be32toh(sum);
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
	}
	outlen = outlen / 8 * 7 + offset;
	const uint32_t* vals = (const uint32_t*)data;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i < outlen - 7; i+=7) {	// n += 2 in one loop
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(vals[n++]) - 0x4e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc0000;
		shift <<= 2;
		sum |= shift & 0x0003fff0;
		shift = htobe32(vals[n++]) - 0x4e004e00;
		sum |= shift >> 26;
		*(uint32_t*)(buf+i) = be32toh(sum);
		sum = 0;
		shift <<= 6;
		sum |= shift & 0xffc00000;
		shift <<= 2;
		sum |= shift & 0x003fff00;
		*(uint32_t*)(buf+i+4) = be32toh(sum);
	}
	base16384_union_remainder valbuf;
	if(outlen - i == 7) {
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(vals[n++]) - 0x4e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc0000;
		shift <<= 2;
		sum |= shift & 0x0003fff0;
		shift = htobe32(vals[n]) - 0x4e004e00;
		sum |= shift >> 26;
		*(uint32_t*)(buf+i) = be32toh(sum);
		sum = 0;
		shift <<= 6;
		sum |= shift & 0xffc00000;
		shift <<= 2;
		sum |= shift & 0x003fff00;
		valbuf.val = be32toh(sum);
		memcpy(buf+i+4, valbuf.buf, 3);
	} else if((*(uint8_t*)(&vals[n]) != '=') && offset--) {
		int cnt = dlen-2-(int)n*(int)sizeof(uint32_t);
		if (cnt > 4) cnt = 4;
		memcpy(valbuf.buf, &vals[n], cnt);
		n++;
		#ifdef WORDS_BIGENDIAN
			register uint32_t sum = __builtin_bswap32(valbuf.val);
		#else
			register uint32_t sum = valbuf.val;
		#endif
		sum -= 0x0000004e;
		buf[i++] = ((sum & 0x0000003f) << 2) | ((sum & 0x0000c000) >> 14);
		if(offset--) {
			sum -= 0x004e0000;
			buf[i++] = ((sum & 0x00003f00) >> 6) | ((sum & 0x00300000) >> 20);
			if(offset--) {
				buf[i++] = ((sum & 0x000f0000) >> 12) | ((sum & 0xf0000000) >> 28);
				if(offset--) {
					buf[i] = (sum & 0x0f000000) >> 20;
					if(*(uint8_t*)(&vals[n]) == '=') return outlen;
					memcpy(valbuf.buf, &vals[n], dlen-2-(int)n*(int)sizeof(uint32_t));
					#ifdef WORDS_BIGENDIAN
						sum = __builtin_bswap32(valbuf.val);
					#else
						sum = valbuf.val;
					#endif
					sum -= 0x0000004e;
					buf[i++] |= (sum & 0x0000003c) >> 2;
					if(offset--) {
						buf[i++] = ((sum & 0x00000003) << 6) | ((sum & 0x0000fc00) >> 10);
						if(offset--) {
							sum -= 0x004e0000;
							buf[i] = ((sum & 0x00000300) >> 2) | ((sum & 0x003f0000) >> 16);
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
	}
	outlen = outlen / 8 * 7 + offset;
	const uint32_t* vals = (const uint32_t*)data;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i <= outlen - 7; i+=7) {	// n += 2 in one loop
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(vals[n++]) - 0x4e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc0000;
		shift <<= 2;
		sum |= shift & 0x0003fff0;
		shift = htobe32(vals[n++]) - 0x4e004e00;
		sum |= shift >> 26;
		*(uint32_t*)(buf+i) = be32toh(sum);
		sum = 0;
		shift <<= 6;
		sum |= shift & 0xffc00000;
		shift <<= 2;
		sum |= shift & 0x003fff00;
		*(uint32_t*)(buf+i+4) = be32toh(sum);
	}
	if(*(uint8_t*)(&vals[n]) == '=') return outlen;
	if(offset--) {
		// here comes a read overlap
		#ifdef WORDS_BIGENDIAN
			register uint32_t sum = __builtin_bswap32(vals[n++]);
		#else
			register uint32_t sum = vals[n++];
		#endif
		sum -= 0x0000004e;
		buf[i++] = ((sum & 0x0000003f) << 2) | ((sum & 0x0000c000) >> 14);
		if(offset--) {
			sum -= 0x004e0000;
			buf[i++] = ((sum & 0x00003f00) >> 6) | ((sum & 0x00300000) >> 20);
			if(offset--) {
				buf[i++] = ((sum & 0x000f0000) >> 12) | ((sum & 0xf0000000) >> 28);
				if(offset--) {
					buf[i] = (sum & 0x0f000000) >> 20;
					if(*(uint8_t*)(&vals[n]) == '=') return outlen;
					// here comes a read overlap
					#ifdef WORDS_BIGENDIAN
						sum = __builtin_bswap32(vals[n]);
					#else
						sum = vals[n];
					#endif
					sum -= 0x0000004e;
					buf[i++] |= (sum & 0x0000003c) >> 2;
					if(offset--) {
						buf[i++] = ((sum & 0x00000003) << 6) | ((sum & 0x0000fc00) >> 10);
						if(offset--) {
							sum -= 0x004e0000;
							buf[i] = ((sum & 0x00000300) >> 2) | ((sum & 0x003f0000) >> 16);
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
	}
	outlen = outlen / 8 * 7 + offset;
	const uint32_t* vals = (const uint32_t*)data;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i < outlen-7; i+=7) {	// n += 2 in one loop
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(vals[n++]) - 0x4e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc0000;
		shift <<= 2;
		sum |= shift & 0x0003fff0;
		shift = htobe32(vals[n++]) - 0x4e004e00;
		sum |= shift >> 26;
		*(uint32_t*)(buf+i) = be32toh(sum);
		sum = 0;
		shift <<= 6;
		sum |= shift & 0xffc00000;
		shift <<= 2;
		sum |= shift & 0x003fff00;
		*(uint32_t*)(buf+i+4) = be32toh(sum);
	}
	register uint32_t sum = 0;
	register uint32_t shift = htobe32(vals[n++]);
	if(((shift>>24)&0xff) == 0x3d) return outlen;
	if(((shift>>24)&0xff) < 0x4e) shift |= 0xff000000;
	if(((shift>> 8)&0xff) < 0x4e) shift |= 0x0000ff00;
	shift -= 0x4e004e00;
	shift <<= 2;
	sum |= shift & 0xfffc0000;
	shift <<= 2;
	sum |= shift & 0x0003fff0;
	shift = htobe32(vals[n]);
	if(((shift>>24)&0xff) == 0x3d) {
		*(uint32_t*)(buf+i) = be32toh(sum);
		return outlen;
	}
	if(((shift>>24)&0xff) < 0x4e) shift |= 0xff000000;
	if(((shift>> 8)&0xff) < 0x4e) shift |= 0x0000ff00;
	shift -= 0x4e004e00;
	sum |= shift >> 26;
	*(uint32_t*)(buf+i) = be32toh(sum);
	sum = 0;
	shift <<= 6;
	sum |= shift & 0xffc00000;
	shift <<= 2;
	sum |= shift & 0x003fff00;
	*(uint32_t*)(buf+i+4) = be32toh(sum);
	return outlen;
}
