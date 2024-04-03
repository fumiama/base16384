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

char encbuf[BASE16384_ENCBUFSZ];
char decbuf[BASE16384_DECBUFSZ];

#ifndef _WIN32
unsigned long get_start_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

static base16384_err_t print_usage() {
	fputs("Copyright (c) 2022-2024 Fumiama Minamoto.\nBase16384 2.3.0 (April 4th 2024). Usage:\n", stderr);
	fputs("base16384 [-edtn] [inputfile] [outputfile]\n", stderr);
	fputs("  -e\t\tencode (default)\n", stderr);
	fputs("  -d\t\tdecode\n", stderr);
	fputs("  -t\t\tshow spend time\n", stderr);
	fputs("  -n\t\tdon't write utf16be file header (0xFEFF)\n", stderr);
	fputs("  -c\t\tembed or validate checksum in remainder\n", stderr);
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
		(no_header?BASE16384_FLAG_NOHEADER:0) | (use_checksum?BASE16384_FLAG_SUM_CHECK_ON_REMAIN:0) \
	)
	exitstat = is_encode?do_coding(encode):do_coding(decode);

	if(t) {
		#ifdef _WIN32
			fprintf(stderr, "spend time: %lums\n", clock() - t);
		#else
			fprintf(stderr, "spend time: %lums\n", get_start_ms() - t);
		#endif
	}

	#define print_base16384_err(n) case base16384_err_##n: perror("base16384_err_"#n)
	if(exitstat) switch(exitstat) {
		print_base16384_err(get_file_size); break;
		print_base16384_err(fopen_output_file); break;
		print_base16384_err(fopen_input_file); break;
		print_base16384_err(write_file); break;
		print_base16384_err(open_input_file); break;
		print_base16384_err(map_input_file); break;
		print_base16384_err(read_file); break;
		print_base16384_err(invalid_file_name); break;
		print_base16384_err(invalid_commandline_parameter); break;
		print_base16384_err(invalid_decoding_checksum); break;
		default: perror("base16384"); break;
	}
    return exitstat;

}
