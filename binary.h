#ifndef _BINARY_H_
#define _BINARY_H_

/* binary.h
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

#ifdef __cosmopolitan // always le
	#define be16toh(x) bswap_16(x)
	#define be32toh(x) bswap_32(x)
	#define htobe16(x) bswap_16(x)
	#define htobe32(x) bswap_32(x)
#else
	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>
	#ifdef __linux__
		#include <endian.h>
	#endif
	#ifdef __FreeBSD__
		#include <sys/endian.h>
	#endif
	#ifdef __NetBSD__
		#include <sys/endian.h>
	#endif
	#ifdef __OpenBSD__
		#include <sys/types.h>
		#define be16toh(x) betoh16(x)
		#define be32toh(x) betoh32(x)
		#ifdef IS_64BIT_PROCESSOR
			#define be64toh(x) betoh64(x)
		#endif
	#endif
	#ifdef __APPLE__
		#define be16toh(x) ntohs(x)
		#define be32toh(x) ntohl(x)
		#ifdef IS_64BIT_PROCESSOR
			#define be64toh(x) ntohll(x)
		#endif
		#define htobe16(x) htons(x)
		#define htobe32(x) htonl(x)
	#ifdef IS_64BIT_PROCESSOR
		#define htobe64(x) htonll(x)
	#endif
	#endif
	#ifdef _MSC_VER
		#ifdef WORDS_BIGENDIAN
			#define be16toh(x) (x)
			#define be32toh(x) (x)
			#ifdef IS_64BIT_PROCESSOR
				#define be64toh(x) (x)
			#endif
			#define htobe16(x) (x)
			#define htobe32(x) (x)
			#ifdef IS_64BIT_PROCESSOR
				#define htobe64(x) (x)
			#endif
		#else
			#define be16toh(x) _byteswap_ushort(x)
			#define be32toh(x) _byteswap_ulong(x)
			#ifdef IS_64BIT_PROCESSOR
				#define be64toh(x) _byteswap_uint64(x)
			#endif
			#define htobe16(x) _byteswap_ushort(x)
			#define htobe32(x) _byteswap_ulong(x)
			#ifdef IS_64BIT_PROCESSOR
				#define htobe64(x) _byteswap_uint64(x)
			#endif
		#endif
	#endif
#endif

// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (sizeof(x)*8 - (c))))

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

#endif
