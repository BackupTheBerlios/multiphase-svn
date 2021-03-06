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

#ifndef __MISC_H__
#define __MISC_H__

#include <stddef.h>
#include <stdio.h>

void __die(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
#define die(fmt, args...)				\
	do {						\
		fprintf(stderr, "%s: ", __func__);	\
		__die(fmt, ##args);			\
	} while (0);

void * xmalloc(size_t size);

#endif /* ndef __MISC_H__ */
