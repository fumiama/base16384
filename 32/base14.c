//base1432le.c
//fumiama 20210408
#include <stdio.h>
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
#include "base14.h"

//#define DEBUG

#ifndef new
	#define new malloc
#endif

LENDAT* encode(const uint8_t* data, const int32_t len) {
	LENDAT* encd = (LENDAT*)new(sizeof(LENDAT));
	int32_t outlen = len / 7 * 8;
	uint8_t offset = len % 7;
	switch(offset) {	//算上偏移标志字符占用的2字节
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
	encd->data = (uint8_t*)new(outlen + 8);	//冗余的8B用于可能的结尾的覆盖
	encd->len = outlen;
	uint32_t* vals = (uint32_t*)(encd->data);
	uint32_t n = 0;
	int32_t i = 0;
	for(; i <= len - 7; i += 7) {
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
		encd->data[outlen - 2] = '=';
		encd->data[outlen - 1] = offset;
	}
	return encd;
}

LENDAT* decode(const uint8_t* data, const int32_t len) {
	LENDAT* decd = (LENDAT*)new(sizeof(LENDAT));
	int32_t outlen = len;
	uint8_t offset = 0;
	if(data[len-2] == '=') {
		offset = data[len-1];
		switch(offset) {	//算上偏移标志字符占用的2字节
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
	decd->data = (uint8_t*)new(outlen+1); //多出1字节用于循环覆盖
	decd->len = outlen;
	uint32_t* vals = (uint32_t*)data;
	uint32_t n = 0;
	int32_t i = 0;
	for(; i <= outlen - 7; i+=7) {	//n实际每次自增2
		register uint32_t sum = 0;
		register uint32_t shift = htobe32(vals[n++]) - 0x4e004e00;
		shift <<= 2;
		sum |= shift & 0xfffc0000;
		shift <<= 2;
		sum |= shift & 0x0003fff0;
		shift = htobe32(vals[n++]) - 0x4e004e00;
		sum |= shift >> 26;
		*(uint32_t*)(decd->data+i) = be32toh(sum);
		sum = 0;
		shift <<= 6;
		sum |= shift & 0xffc00000;
		shift <<= 2;
		sum |= shift & 0x003fff00;
		*(uint32_t*)(decd->data+i+4) = be32toh(sum);
	}
	if(offset--) {
		//这里有读取越界
		#ifdef WORDS_BIGENDIAN
			register uint32_t sum = __builtin_bswap32(vals[n++]);
		#else
			register uint32_t sum = vals[n++];
		#endif
		sum -= 0x0000004e;
		decd->data[i++] = ((sum & 0x0000003f) << 2) | ((sum & 0x0000c000) >> 14);
		if(offset--) {
			sum -= 0x004e0000;
			decd->data[i++] = ((sum & 0x00003f00) >> 6) | ((sum & 0x00300000) >> 20);
			if(offset--) {
				decd->data[i++] = ((sum & 0x000f0000) >> 12) | ((sum & 0xf0000000) >> 28);
				if(offset--) {
					decd->data[i++] = (sum & 0x0f000000) >> 20;
					//这里有读取越界
					sum = vals[n];
					sum -= 0x0000004e;
					decd->data[i++] |= (sum & 0x0000003c) >> 2;
					if(offset--) {
						decd->data[i++] = ((sum & 0x00000003) << 6) | ((sum & 0x0000fc00) >> 10);
						if(offset--) {
							sum -= 0x004e0000;
							decd->data[i] = ((sum & 0x00000300) >> 2) | ((sum & 0x003f0000) >> 16);
						}
					}
				}
			}
		}
	}
	return decd;
}
