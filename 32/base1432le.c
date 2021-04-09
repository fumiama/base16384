//base1432le.c
//fumiama 20210408
#include <stdio.h>
#include <stdlib.h>
#include "base1432le.h"

//#define DEBUG

LENDAT* encode(const uint8_t* data, const u_int32_t len) {
	LENDAT* encd = (LENDAT*)malloc(sizeof(LENDAT));
	uint32_t outlen = len / 7 * 8;
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
	encd->data = (uint8_t*)malloc(outlen + 8);	//冗余的8B用于可能的结尾的覆盖
	encd->len = outlen;
	uint32_t* vals = (uint32_t*)(encd->data);
	uint32_t n = 0;
	uint32_t i = 0;
	for(; i < len; i += 7) {
		register uint32_t sum = 0x0000003f & ((uint32_t)data[i] >> 2);
		sum |= (((uint32_t)data[i + 1] << 6) | (data[i] << 14)) & 0x0000ff00;
		sum |= (((uint32_t)data[i + 1] << 20) | ((uint32_t)data[i + 2] << 12)) & 0x003f0000;
		sum |= (((uint32_t)data[i + 2] << 28) | ((uint32_t)data[i + 3] << 20)) & 0xff000000;
		sum += 0x004e004e;
		vals[n++] = sum;
		#ifdef DEBUG
			printf("n: %u, add sum: %08x\n", n, sum);
		#endif
		sum = ((((uint32_t)data[i + 3] << 2) | ((uint32_t)data[i + 4] >> 6))) & 0x0000003f;
		sum |= (((uint32_t)data[i + 4] << 10) | ((uint32_t)data[i + 5] << 2)) & 0x0000ff00;
		sum |= ((uint32_t)data[i + 5] << 16) & 0x003f0000;
		sum |= ((uint32_t)data[i + 6] << 24) & 0xff000000;
		sum += 0x004e004e;
		vals[n++] = sum;
		#ifdef DEBUG
			printf("n: %u, add sum: %08x\n", n, sum);
		#endif
	}
	if(offset > 0) {
		register uint32_t sum = 0x0000003f & (data[i] >> 2);
		sum |= ((uint32_t)data[i] << 14) & 0x0000c000;
		if(offset > 1) {
			sum |= ((uint32_t)data[i + 1] << 6) & 0x00003f00;
			sum |= ((uint32_t)data[i + 1] << 20) & 0x00300000;
			if(offset > 2) {
				sum |= ((uint32_t)data[i + 2] << 12) & 0x000f0000;
				sum |= ((uint32_t)data[i + 2] << 28) & 0xf0000000;
				if(offset == 2) {
					sum += 0x004e004e;
					vals[n++] = sum;
				}
				if(offset > 3) {
					sum |= ((uint32_t)data[i + 3] << 20) & 0x0f000000;
					sum += 0x004e004e;
					vals[n++] = sum;
					sum = (((uint32_t)data[i + 3] << 2)) & 0x0000003c;
					if(offset > 4) {
						sum |= (((uint32_t)data[i + 4] >> 6)) & 0x00000003;
						sum |= ((uint32_t)data[i + 4] << 10) & 0x0000fc00;
						if(offset > 5) {
							sum |= ((uint32_t)data[i + 5] << 2) & 0x00000300;
							sum |= ((uint32_t)data[i + 5] << 16) & 0x003f0000;
						}
					}
					sum += 0x004e004e;
					vals[n] = sum;
				}
			}
		}
		encd->data[outlen - 2] = '=';
		encd->data[outlen - 1] = offset;
	}
	return encd;
}

LENDAT* decode(const uint8_t* data, const u_int32_t len) {
	LENDAT* decd = (LENDAT*)malloc(sizeof(LENDAT));
	uint32_t outlen = len;
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
	decd->data = (uint8_t*)malloc(outlen);
	decd->len = outlen;
	uint32_t* vals = (uint32_t*)data;
	uint32_t n = 0;
	uint32_t i = 0;
	for(; n < len / 4; n++) {	//n实际每次自增2
		register uint32_t sum = vals[n++];
		sum -= 0x004e004e;
		decd->data[i++] = ((sum & 0x0000003f) << 2) | ((sum & 0x0000c000) >> 14);
		decd->data[i++] = ((sum & 0x00003f00) >> 6) | ((sum & 0x00300000) >> 20);
		decd->data[i++] = ((sum & 0x000f0000) >> 12) | ((sum & 0xf0000000) >> 28);
		decd->data[i] = ((sum & 0x0f000000) >> 20);
		sum = vals[n];
		sum -= 0x004e004e;
		decd->data[i++] |= ((sum & 0x0000003c) >> 2);
		decd->data[i++] = ((sum & 0x00000003) << 6) | ((sum & 0x0000fc00) >> 10);
		decd->data[i++] = ((sum & 0x00000300) >> 2) | ((sum & 0x003f0000) >> 16);
		decd->data[i++] = ((sum & 0xff000000) >> 24);
	}
	if(offset > 0) {
		register uint32_t sum = vals[n++];
		sum -= 0x0000004e;
		decd->data[i++] = ((sum & 0x0000003f) << 2) | ((sum & 0x0000c000) >> 14);
		if(offset > 1) {
			sum -= 0x004e0000;
			decd->data[i++] = ((sum & 0x00003f00) >> 6) | ((sum & 0x00300000) >> 20);
			if(offset > 2) {
				decd->data[i++] = ((sum & 0x000f0000) >> 12) | ((sum & 0xf0000000) >> 28);
				if(offset > 3) {
					decd->data[i++] = (sum & 0x0f000000) >> 20;
					sum = vals[n];
					sum -= 0x0000004e;
					decd->data[i++] |= (sum & 0x0000003c) >> 2;
					if(offset > 4) {
						decd->data[i++] = ((sum & 0x00000003) << 6) | ((sum & 0x0000fc00) >> 10);
						if(offset > 5) {
							sum -= 0x004e0000;
							decd->data[i++] = ((sum & 0x00000300) >> 2) | ((sum & 0x003f0000) >> 16);
						}
					}
				}
			}
		}
	}
	return decd;
}
