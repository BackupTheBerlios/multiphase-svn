/* Miscellaneous routines.
   Copyright (C) 2005 Alexey Dobriyan <adobriyan@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "misc.h"

void __die(const char *fmt, ...)
{
	va_list args;
	static char excuse[512];

	va_start(args, fmt);
	vsnprintf(excuse, sizeof(excuse), fmt, args);
	va_end(args);

	fprintf(stderr, "%s\n", excuse);
	exit(EXIT_FAILURE);
}

void * xmalloc(size_t size)
{
	void *ret;

	ret = malloc(size);
	if (!ret)
		die("not enough memory for %zu bytes", size);
	return ret;
}
