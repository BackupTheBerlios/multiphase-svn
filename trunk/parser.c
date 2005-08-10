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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "parser.h"

static int get_token(int c, char **string)
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
			c = get_token(c, &token->string);
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

static void mark_as_not_found(struct param *params)
{
	int i;

	while (params[i].name) {
		params[i].found = 0;
		i++;
	}
}

static long double to_number(char *string)
{
	long double number;
	char *end;

	number = strtold(string, &end);
	if (end == string || *end)
		die("%s: expected number, got \"%s\"", __func__, string);
	return number;
}

void get_values(struct token *token, struct param *params)
{
	int i;

	mark_as_not_found(params);

	while (token) {
		i = 0;
		while (params[i].name) {
			if (strcmp(token->string, params[i].name) == 0) {
				if (!token->next)
					die("%s: missing value for \"%s\"",
						__func__, params[i].name);
				*params[i].value =
					to_number(token->next->string);
				params[i].found = 1;
				token = token->next;
				goto found;
			}
			i++;
		}
		die("%s: unknown token \"%s\"", __func__, token->string);
found:
		token = token->next;
	}

	i = 0;
	while (params[i].name) {
		if (!params[i].found)
			die("%s: no value for %s", __func__, params[i].name);
		i++;
	}
}
