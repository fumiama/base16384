/* base16384.c
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
#include <string.h>
#include <time.h>
#ifdef _WIN32
	#include <windows.h>
#endif
#endif
#include "base16384.h"

static char encbuf[BASE16384_ENCBUFSZ];
static char decbuf[BASE16384_DECBUFSZ];

#ifndef _WIN32
static unsigned long get_start_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

static base16384_err_t print_usage() {
	#ifndef BASE16384_VERSION
		#define BASE16384_VERSION "dev"
	#endif
	#ifndef BASE16384_VERSION_DATE
		#define BASE16384_VERSION_DATE "unknown date"
	#endif
	fputs(
		"Copyright (c) 2022-2024 Fumiama Minamoto.\nBase16384 "
		BASE16384_VERSION
		" ("
			BASE16384_VERSION_DATE
		"). Usage:\n", stderr
	);
	fputs("base16384 -[ed][t][n][cC] [inputfile] [outputfile]\n", stderr);
	fputs("  -e\t\tencode (default)\n", stderr);
	fputs("  -d\t\tdecode\n", stderr);
	fputs("  -t\t\tshow spend time\n", stderr);
	fputs("  -n\t\tdonot write utf16be file header (0xFEFF)\n", stderr);
	fputs("  -c\t\tembed or validate checksum in remainder\n", stderr);
	fputs("  -C\t\tdo -c forcely\n", stderr);
	fputs("  inputfile\tpass - to read from stdin\n", stderr);
	fputs("  outputfile\tpass - to write to stdout\n", stderr);
	return base16384_err_invalid_commandline_parameter;
}

int main(int argc, char** argv) {

	const char* cmd = argv[1];
	if(argc != 4 || cmd[0] != '-') return print_usage();

	int flaglen = strlen(cmd);
	if(flaglen <= 1 || flaglen > 5) return print_usage();

	#ifdef _WIN32
		clock_t t = 0;
	#else
		unsigned long t = 0;
	#endif

	uint16_t is_encode = 1, use_timer = 0, no_header = 0, use_checksum = 0;
	#define set_flag(f, v) ((f) = (((((f)>>8)+1) << 8)&0xff00) | (v&0x00ff))
	#define flag_has_been_set(f) ((f)>>8)
	#define set_or_test_flag(f, v) (flag_has_been_set(f)?1:(set_flag(f, v), 0))
	while(--flaglen) switch(cmd[flaglen]) { // skip cmd[0] = '-'
		case 'e':
			if(set_or_test_flag(is_encode, 1)) return print_usage();
		break;
		case 'd':
			if(set_or_test_flag(is_encode, 0)) return print_usage();
		break;
		case 't':
			if(set_or_test_flag(use_timer, 1)) return print_usage();
		break;
		case 'n':
			if(set_or_test_flag(no_header, 1)) return print_usage();
		break;
		case 'c':
			if(set_or_test_flag(use_checksum, 1)) return print_usage();
		break;
		case 'C':
			if(set_or_test_flag(use_checksum, 2)) return print_usage();
		break;
		default:
			return print_usage();
		break;
	}
	#define clear_high_byte(x) ((x) &= 0x00ff)
	clear_high_byte(is_encode); clear_high_byte(use_timer);
	clear_high_byte(no_header); clear_high_byte(use_checksum);

	if(use_timer) {
		#ifdef _WIN32
			t = clock();
		#else
			t = get_start_ms();
		#endif
	}

	base16384_err_t exitstat = base16384_err_ok;

	#define do_coding(method) base16384_##method##_file_detailed( \
		argv[2], argv[3], encbuf, decbuf, \
		(no_header?BASE16384_FLAG_NOHEADER:0) \
		| ((use_checksum&1)?BASE16384_FLAG_SUM_CHECK_ON_REMAIN:0) \
		| ((use_checksum&2)?BASE16384_FLAG_DO_SUM_CHECK_FORCELY:0) \
	)
		exitstat = is_encode?do_coding(encode):do_coding(decode);
	#undef do_coding
	if(t) {
		#ifdef _WIN32
			fprintf(stderr, "spend time: %lums\n", clock() - t);
		#else
			fprintf(stderr, "spend time: %lums\n", get_start_ms() - t);
		#endif
	}

    return base16384_perror(exitstat);

}
