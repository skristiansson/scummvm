/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WP81_PORTDEFS_H
#define WP81_PORTDEFS_H

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#include <string.h>
#undef strdup
#define strdup _strdup

// FIXME: The placement of the workaround functions for MSVC below
// require us to include stdio.h and stdarg.h for MSVC here. This
// is not exactly nice...
// We should think of a better way of doing this.
#include <stdio.h>
#include <stdarg.h>

// MSVC's vsnprintf is either non-existent (2003) or bugged since it
// does not always include a terminating NULL (2005+). To work around
// that we fix up the _vsnprintf included. Note that the return value
// will still not match C99's specs!
inline int vsnprintf_msvc(char *str, size_t size, const char *format, va_list args) {
	// We do not pass size - 1 here, to ensure we would get the same
	// return value as when we would use _vsnprintf directly, since
	// for example Common::String::format relies on this.
	int retValue = _vsnprintf(str, size, format, args);
	str[size - 1] = 0;
	return retValue;
}

#define vsnprintf vsnprintf_msvc

// Visual Studio does not include snprintf in its standard C library.
// Instead it includes a function called _snprintf with somewhat
// similar semantics. The minor difference is that the return value in
// case the formatted string exceeds the buffer size is different.
// A much more dangerous one is that _snprintf does not always include
// a terminating null (Whoops!). Instead we map to our fixed vsnprintf.
inline int snprintf(char *str, size_t size, const char *format, ...) {
	va_list args;
	va_start(args, format);
	int len = vsnprintf(str, size, format, args);
	va_end(args);
	return len;
}

#if defined(ARRAYSIZE)
#undef ARRAYSIZE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
// MSVC does not define M_PI, M_SQRT2 and other math defines by default.
// _USE_MATH_DEFINES must be defined in order to have these defined, thus
// we enable it here. For more information, check:
// http://msdn.microsoft.com/en-us/library/4hwaceh6(v=VS.100).aspx
#define _USE_MATH_DEFINES
#include <math.h>
#include <new>

#endif
