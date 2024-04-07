/* wrap.c
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

#include "base16384.h"

#define base16384_typed_params(type) type input, type output, char* encbuf, char* decbuf

#define BASE16384_WRAP_DECL(method, name, type) \
	base16384_err_t base16384_##method##_##name(base16384_typed_params(type)) { \
		return base16384_##method##_##name##_detailed(input, output, encbuf, decbuf, 0); \
	}

	BASE16384_WRAP_DECL(encode, file, const char*);
	BASE16384_WRAP_DECL(encode, fp, FILE*);
	BASE16384_WRAP_DECL(encode, fd, int);
	BASE16384_WRAP_DECL(encode, stream, base16384_stream_t*);

	BASE16384_WRAP_DECL(decode, file, const char*);
	BASE16384_WRAP_DECL(decode, fp, FILE*);
	BASE16384_WRAP_DECL(decode, fd, int);
	BASE16384_WRAP_DECL(decode, stream, base16384_stream_t*);

#undef BASE16384_WRAP_DECL

#undef base16384_typed_params
