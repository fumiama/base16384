/* base1432.c
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

#ifdef __cosmopolitan // always le
#  define be16toh(x) bswap_16(x)
#  define be32toh(x) bswap_32(x)
#  define htobe16(x) bswap_16(x)
#  define htobe32(x) bswap_32(x)
#else
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef __linux__
#  include <endian.h>
#endif
#ifdef __FreeBSD__
#  include <sys/endian.h>
#endif
#ifdef __NetBSD__
#  include <sys/endian.h>
#endif
#ifdef __OpenBSD__
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#endif
#ifdef __MAC_10_0
#  define be16toh(x) ntohs(x)
#  define be32toh(x) ntohl(x)
#  define htobe16(x) ntohs(x)
#  define htobe32(x) htonl(x)
#endif
#ifdef _WIN32
	#ifdef WORDS_BIGENDIAN
		#  define be16toh(x) (x)
		#  define be32toh(x) (x)
		#  define htobe16(x) (x)
		#  define htobe32(x) (x)
	#else
		#  define be16toh(x) _byteswap_ushort(x)
		#  define be32toh(x) _byteswap_ulong(x)
		#  define htobe16(x) _byteswap_ushort(x)
		#  define htobe32(x) _byteswap_ulong(x)
	#endif
#endif
#endif

// #define DEBUG

int encode(const char* data, int dlen, char* buf, int blen) {
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
	#ifdef DEBUG
		printf("outlen: %llu, offset: %u, malloc: %llu\n", outlen, offset, outlen + 8);
	#endif
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

int decode(const char* data, int dlen, char* buf, int blen) {
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
	uint32_t* vals = (uint32_t*)data;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i <= outlen - 7; i+=7) {	// n实际每次自增2
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
	if(offset--) {
		// 这里有读取越界
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
					buf[i++] = (sum & 0x0f000000) >> 20;
					// 这里有读取越界
					sum = vals[n];
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
