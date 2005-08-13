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

	for (i = 0; params[i].name; i++)
		if (params[i].type == ARRAY)
			free(params[i].value);
}

static void mark_as_not_found(struct param *params)
{
	int i;

	for (i = 0; params[i].name; i++)
		params[i].found = 0;
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

static void get_number(struct param *param, struct token *token)
{
	if (to_number(token->string, param->value) < 0)
		die("%s: expected number, got \"%s\"", __func__, token->string);
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
	param->nr_elements = len;
}

/* Get either one number or an array. Return last used token. */
static struct token * get_param(struct param *param, struct token *token)
{
	if (!token->next)
		die("%s: missing value for \"%s\"", __func__, param->name);
	switch (param->type) {
	case NUMBER:
		token = token->next;
		get_number(param, token);
		break;
	case ARRAY: {
		struct token *prev;

		prev = token;
		token = token->next;
		if (to_number(token->string, NULL) < 0)
			die("%s: missing value for \"%s\"", __func__,
				param->name);

		get_array(param, token);

		/* Bump current token to the first not used one. */
		while (token &&	to_number(token->string, NULL) == 0) {
			token = token->next;
			prev = prev->next;
		}
		/* Compensate for "token = token->next" in get_values(). */
		token = prev;
	}
		break;
	default:
		die("%s: unknown parameter type %d", __func__, param->type);
	}
	return token;
}

void get_values(struct param *params, struct token *token)
{
	int i;

	mark_as_not_found(params);

	while (token) {
		for (i = 0; params[i].name; i++)
			if (strcmp(token->string, params[i].name) == 0) {
				if (params[i].found)
					die("%s: \"%s\" already found", __func__, params[i].name);
				token = get_param(&params[i], token);
				params[i].found = 1;
				goto found;
			}
		die("%s: unknown token \"%s\"", __func__, token->string);
found:
		token = token->next;
	}

	for (i = 0; params[i].name; i++)
		if (!params[i].found)
			die("%s: no value for %s", __func__, params[i].name);
}

struct param * find_param(char *name, struct param *params)
{
	int i;

	for (i = 0; params[i].name; i++)
		if (strcmp(params[i].name, name) == 0)
			break;
	if (!params[i].name)
		die("%s: can't find \"%s\" param", __func__, name);
	return &params[i];
}
