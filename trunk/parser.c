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

/* Free arrays allocated by get_array(). */
void free_arrays(struct param *params)
{
	int i;

	i = 0;
	while (params[i].name) {
		if (params[i].type == ARRAY)
			free(params[i].value);
		i++;
	}
}

static void mark_as_not_found(struct param *params)
{
	int i;

	i = 0;
	while (params[i].name) {
		params[i].found = 0;
		i++;
	}
}

static int to_number(char *string, long double *ret)
{
	long double number;
	char *end;

	number = strtold(string, &end);
	if (end == string || *end)
		return -1;
	if (ret)
		*ret = number;
	return 0;
}

/* Allocate array of numbers and fill it from list of tokens. Proceed until
   first non-number. */
static void get_array(struct param *param, struct token *head)
{
	struct token *tmp;
	int len, i;

	/* Find out how many numbers there are. */
	tmp = head;
	len = 0;
	while (tmp && to_number(tmp->string, NULL) == 0) {
		len++;
		tmp = tmp->next;
	}
	if (len == 0)
		die("%s: missing value for \"%s\"", __func__, head->string);

	param->value = xmalloc(len * sizeof(*param->value));

	/* Fill the array. */
	tmp = head;
	i = 0;
	while (tmp && to_number(tmp->string, &param->value[i]) == 0) {
		i++;
		tmp = tmp->next;
	}
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
				switch (params[i].type) {
				case NUMBER:
					if (to_number(token->next->string,
							params[i].value) < 0)
						die("%s: expected number, "
							"got \"%s\"", __func__,
							token->next->string);
					token = token->next;
					break;
				case ARRAY: {
					struct token *prev;

					if (to_number(token->next->string,
								NULL) < 0)
						die("%s: missing value for "
							"\"%s\"", __func__,
							params[i].name);

					get_array(&params[i], token->next);

					/* Bump current token to the first not
					   used one. */
					prev = token;
					token = token->next;
					while (token &&
						to_number(token->string,
								NULL) == 0) {
						token = token->next;
						prev = prev->next;
					}
					/* Compensate for "token = token->next"
					   after "found:". */
					token = prev;
				}
					break;
				default:
					die("%s: unknown parameter type %d",
						__func__, params[i].type);
				}
				params[i].found = 1;
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

long double * find_array(char *name, struct param *params)
{
	int i;

	i = 0;
	while (params[i].name && strcmp(params[i].name, name) != 0) {
		if (params[i].type == ARRAY)
			break;
		i++;
	}
	return params[i].name ? params[i].value : NULL;
}
