/* Input parser.
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

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "parser.h"

static int get_string(int c, char **string)
{
	int pos;
	char buf[32];

	buf[0] = c;
	pos = 1;
	c = getchar();
	while (pos < sizeof(buf) && c != EOF && !isspace(c)) {
		buf[pos] = c;
		pos++;
		c = getchar();
	}
	if (pos == sizeof(buf))
		die("%s: token \"%s\" too long", __func__, buf);
	buf[pos] = '\0';
	*string = xmalloc(strlen(buf) + 1);
	strcpy(*string, buf);
	return c;
}

static struct token head, *last_token = &head;

/* Tokenize input stream. Return single-linked list of tokens. */
struct token * tokenize(void)
{
	int c;

	c = getchar();
	while (c != EOF) {
		if (!isspace(c)) {
			struct token *token;

			token = xmalloc(sizeof(*token));
			c = get_string(c, &token->string);
			last_token->next = token;
			last_token = last_token->next;
		} else
			c = getchar();
	}
	last_token->next = NULL;
	return head.next;
}

void free_tokens(struct token *token)
{
	while (token) {
		struct token *next;

		next = token->next;
		free(token->string);
		free(token);
		token = next;
	}
}
